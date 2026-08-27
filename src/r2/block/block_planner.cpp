#include "r2/block/block_planner.h"

#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

#include "r2/entropy/donor_match_predictive_backend.h"
#include "r2/entropy/paq8px_apm_backend.h"
#include "r2/entropy/paq8px_record_model_backend.h"
#include "r2/entropy/paq8px_linear_prediction_backend.h"
#include "r2/entropy/paq8px_similarity_backend.h"
#include "r2/entropy/paq8px_similarity_sse_backend.h"
#include "r2/entropy/paq8px_generic_sse_backend.h"
#include "r2/entropy/paq8px_detected_sse_backend.h"
#include "r2/entropy/wavpack_backend.h"
#include "r2/entropy/neural_lstm_backend.h"
#include "r2/entropy/lstm_compress_backend.h"
#include "r2/entropy/bgpt_shared_prior_backend.h"
#include "r2/entropy/jax_compress_portable_backend.h"
#include "r2/entropy/lmic_arithmetic_backend.h"
#include "r2/entropy/predictive_v1_backend.h"
#include "r2/entropy/fse_backend.h"
#include "r2/entropy/fastpfor_backend.h"
#include "r2/entropy/rans_backend.h"
#include "r2/entropy/lzma_backend.h"
#include "r2/entropy/lz4_backend.h"
#include "r2/entropy/kanzi_ans_backend.h"
#include "r2/entropy/ppmd7_backend.h"
#include "r2/entropy/ppmd8_backend.h"
#include "r2/entropy/zpaq_backend.h"
#include "r2/entropy/ctw_backend.h"
#include "r2/entropy/stored_backend.h"
#include "r2/entropy/zstd_backend.h"
#include "r2/representation/bwt_transform.h"
#include "r2/representation/kanzi_mtf_transform.h"
#include "r2/representation/kanzi_rlt_transform.h"
#include "r2/representation/xz_x86_bcj_transform.h"
#include "r2/representation/bcj2_transform.h"
#include "r2/representation/jpegls_transform.h"
#include "r2/representation/flac_residual_transform.h"
#include "r2/representation/brotli_text_transform.h"
#include "r2/representation/cmix_word_dictionary_transform.h"
#include "r2/representation/record_transpose_transform.h"
#include "r2/representation/blosc_shuffle_transform.h"
#include "r2/representation/structure_analyzer.h"
#include "r2/routing/activation_router.h"
#include "r2/representation/blosc_bitshuffle_transform.h"
#include "r2/representation/blosc_delta_transform.h"
#include "r2/representation/delta_of_delta_transform.h"
#include "r2/representation/delta_binary_packed_transform.h"
#include "r2/routing/mode_ranker.h"

namespace hz::r2 {
namespace {

void consider(BlockDecision& decision,
              const BlockMode mode,
              const TransformKind transform,
              const EntropyKind entropy,
              std::vector<std::uint8_t> payload,
              std::vector<std::uint8_t> transform_metadata = {}) {
    ++decision.candidate_blocks_by_mode[static_cast<std::size_t>(mode)];
    if (payload.size() + transform_metadata.size() <
        decision.payload.size() + decision.transform_metadata.size()) {
        decision.mode = mode;
        decision.transform = transform;
        decision.entropy = entropy;
        decision.payload = std::move(payload);
        decision.transform_metadata = std::move(transform_metadata);
    }
}

void append_u32_le(std::vector<std::uint8_t>& bytes,
                   const std::uint32_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value));
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
    bytes.push_back(static_cast<std::uint8_t>(value >> 16U));
    bytes.push_back(static_cast<std::uint8_t>(value >> 24U));
}

}  // namespace

BlockPlanner::BlockPlanner(BlockPlannerOptions options)
    : options_(options),
      family_telemetry_(HierarchicalActivationRouter::family_slot_count()) {}

void BlockPlanner::update_family_telemetry(
    const BlockDecision& decision,
    const std::size_t input_size) {
    if (options_.policy != CandidatePolicy::Auto || input_size == 0) {
        return;
    }

    const std::array<std::vector<std::optional<std::size_t>>, 8> family_candidates{{
        {decision.predictive_candidate_bytes,
         decision.zstd_candidate_bytes,
         decision.fse_candidate_bytes},
        {decision.donor_match_predictive_candidate_bytes,
         decision.paq8px_apm_predictive_candidate_bytes,
         decision.paq8px_record_model_candidate_bytes,
         decision.paq8px_linear_prediction_candidate_bytes,
         decision.paq8px_similarity_candidate_bytes,
         decision.paq8px_similarity_sse_candidate_bytes,
         decision.paq8px_generic_sse_candidate_bytes,
         decision.paq8px_detected_sse_candidate_bytes},
        {decision.neural_lstm_candidate_bytes,
         decision.shared_neural_lstm_candidate_bytes,
         decision.lstm_compress_candidate_bytes,
         decision.bgpt_shared_prior_candidate_bytes,
         decision.jax_compress_portable_candidate_bytes,
         decision.lmic_arithmetic_candidate_bytes},
        {decision.bwt_zstd_candidate_bytes,
         decision.bwt_mtf_zstd_candidate_bytes,
         decision.bwt_rlt_zstd_candidate_bytes,
         decision.x86_bcj_zstd_candidate_bytes,
         decision.bcj2_zstd_candidate_bytes},
        {decision.shuffle_zstd_candidate_bytes,
         decision.bitshuffle_zstd_candidate_bytes,
         decision.delta_zstd_candidate_bytes,
         decision.delta_of_delta_zstd_candidate_bytes,
         decision.delta_binary_packed_zstd_candidate_bytes,
         decision.fastpfor_candidate_bytes,
         decision.record_transpose_zstd_candidate_bytes},
        {decision.brotli_text_candidate_bytes,
         decision.cmix_word_dictionary_zstd_candidate_bytes},
        {decision.jpeg_ls_candidate_bytes,
          decision.flac_residual_candidate_bytes,
          decision.wavpack_candidate_bytes},
        {decision.lzma_candidate_bytes,
         decision.lz4_candidate_bytes,
         decision.kanzi_ans_candidate_bytes,
         decision.rans_candidate_bytes,
         decision.ppmd7_candidate_bytes,
         decision.ppmd8_candidate_bytes,
         decision.zpaq_candidate_bytes,
         decision.ctw_candidate_bytes}}};

    for (std::size_t family = 0; family < family_candidates.size(); ++family) {
        auto& telemetry = family_telemetry_[family];
        ++telemetry.age;
        std::size_t best = std::numeric_limits<std::size_t>::max();
        for (const auto& candidate : family_candidates[family]) {
            if (candidate.has_value()) {
                best = std::min(best, *candidate);
            }
        }
        if (best == std::numeric_limits<std::size_t>::max()) {
            continue;
        }

        const double loss = std::log2(
            static_cast<double>(std::max<std::size_t>(1, best))) /
            std::log2(static_cast<double>(std::max<std::size_t>(2, input_size)));
        if (telemetry.age > 1U) {
            telemetry.recent_log_loss_16 = loss;
            telemetry.recent_log_loss_256 =
                telemetry.recent_log_loss_256 * 0.8 + loss * 0.2;
            telemetry.recent_log_loss_4096 =
                telemetry.recent_log_loss_4096 * 0.95 + loss * 0.05;
        } else {
            telemetry.recent_log_loss_16 = loss;
            telemetry.recent_log_loss_256 = loss;
            telemetry.recent_log_loss_4096 = loss;
        }
        telemetry.age = 0;
    }
}

BlockDecision BlockPlanner::plan(const ByteView input) {
    if (input.empty()) {
        throw std::invalid_argument("R2 block planner received an empty block");
    }

    const StoredBackend stored;
    BlockDecision decision{};
    decision.payload = stored.encode(input);
    decision.stored_candidate_bytes = decision.payload.size();

    if (options_.policy == CandidatePolicy::StoredOnly) {
        return decision;
    }

    const bool automatic = options_.policy == CandidatePolicy::Auto;
    const bool shortlisted = options_.policy == CandidatePolicy::AutoK2 ||
        options_.policy == CandidatePolicy::AutoK4 ||
        options_.policy == CandidatePolicy::AutoK8;
    const std::uint8_t shortlist_size =
        options_.policy == CandidatePolicy::AutoK2 ? 2U :
        options_.policy == CandidatePolicy::AutoK4 ? 4U : 8U;
    if (automatic || shortlisted) {
        decision.candidate_blocks_by_mode[
            static_cast<std::size_t>(BlockMode::Stored)] = 1U;
    }
    StructureFeatures structure{};
    RepresentationActivation activation{};
    std::vector<bool> family_active;
    if (automatic) {
        structure = StructureAnalyzer().analyze(input);
        activation = StructureActivationRouter().activate(structure);
        family_active = HierarchicalActivationRouter().active_experts(
            structure, family_telemetry_);
        // Layer B is a family gate; Layer A's cheaper representation gates
        // remain the final candidate-specific filter below.
        activation.bwt_zstd = activation.bwt_zstd && family_active[3];
        activation.bwt_mtf_zstd = activation.bwt_mtf_zstd && family_active[3];
        activation.bwt_rlt_zstd = activation.bwt_rlt_zstd && family_active[3];
        activation.x86_bcj_zstd = activation.x86_bcj_zstd && family_active[3];
        activation.shuffle_zstd = activation.shuffle_zstd && family_active[4];
        activation.bitshuffle_zstd = activation.bitshuffle_zstd && family_active[4];
        activation.delta_zstd = activation.delta_zstd && family_active[4];
        activation.delta_of_delta_zstd =
            activation.delta_of_delta_zstd && family_active[4];
        activation.fastpfor = activation.fastpfor && family_active[4];
        activation.record_transpose_zstd =
            activation.record_transpose_zstd && family_active[4];
        activation.jpeg_ls = activation.jpeg_ls && family_active[6];
        activation.flac_residual = activation.flac_residual && family_active[6];
        activation.wavpack = activation.wavpack && family_active[6];
        activation.brotli_text = activation.brotli_text && family_active[5];
        activation.cmix_word_dictionary_zstd =
            activation.cmix_word_dictionary_zstd && family_active[5];
        activation.neural_lstm = activation.neural_lstm && family_active[2];
        activation.shared_neural_lstm =
            activation.shared_neural_lstm && family_active[2];
        activation.lstm_compress = activation.lstm_compress && family_active[2];
        activation.bgpt_shared_prior =
            activation.bgpt_shared_prior && family_active[2];
        activation.jax_compress_portable =
            activation.jax_compress_portable && family_active[2];
        activation.lmic_arithmetic =
            activation.lmic_arithmetic && family_active[2];
        activation.rans = activation.rans && family_active[7];
        activation.delta_binary_packed_zstd =
            activation.delta_binary_packed_zstd && family_active[4];
    }

    const std::vector<BlockMode> shortlist_modes = shortlisted
        ? rank_modes(input, shortlist_size) : std::vector<BlockMode>{};
    const auto shortlist_has = [&](const BlockMode mode) noexcept {
        return shortlisted && shortlist_contains(shortlist_modes, mode);
    };

    const bool auto_statistical = automatic && family_active[0];
    const bool auto_match = automatic && family_active[1];
    const bool auto_lz = automatic && family_active[7];

    if (options_.policy == CandidatePolicy::PredictiveV1Only ||
        auto_statistical) {
        const PredictiveV1Backend predictive(options_.model_seed);
        std::vector<std::uint8_t> payload = predictive.encode(input);
        decision.predictive_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::PredictiveV1Only) {
            decision.mode = BlockMode::PredictiveV1;
            decision.entropy = EntropyKind::SymbolArithmetic;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::PredictiveV1, TransformKind::Raw,
                 EntropyKind::SymbolArithmetic, std::move(payload));
    }

    if (options_.policy == CandidatePolicy::DonorMatchPredictiveOnly ||
        auto_match) {
        const DonorMatchPredictiveBackend predictive(options_.model_seed);
        std::vector<std::uint8_t> payload = predictive.encode(input);
        decision.donor_match_predictive_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::DonorMatchPredictiveOnly) {
            decision.mode = BlockMode::DonorMatchPredictive;
            decision.entropy = EntropyKind::SymbolArithmetic;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::DonorMatchPredictive, TransformKind::Raw,
                 EntropyKind::SymbolArithmetic, std::move(payload));
    }

    if (options_.policy == CandidatePolicy::Paq8pxApmPredictiveOnly ||
        auto_match) {
        const Paq8pxApmBackend predictive(options_.model_seed);
        std::vector<std::uint8_t> payload = predictive.encode(input);
        decision.paq8px_apm_predictive_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::Paq8pxApmPredictiveOnly) {
            decision.mode = BlockMode::Paq8pxApmPredictive;
            decision.entropy = EntropyKind::Paq8pxApm;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Paq8pxApmPredictive,
                 TransformKind::Raw, EntropyKind::Paq8pxApm,
                 std::move(payload));
    }

    if (options_.policy == CandidatePolicy::Paq8pxRecordModelOnly ||
        auto_match) {
        const Paq8pxRecordModelBackend record_model;
        std::vector<std::uint8_t> payload = record_model.encode(input);
        decision.paq8px_record_model_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::Paq8pxRecordModelOnly) {
            decision.mode = BlockMode::Paq8pxRecordModel;
            decision.entropy = EntropyKind::Paq8pxRecordModel;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Paq8pxRecordModel,
                 TransformKind::Raw, EntropyKind::Paq8pxRecordModel,
                 std::move(payload));
    }

    if (options_.policy == CandidatePolicy::Paq8pxLinearPredictionOnly ||
        auto_match) {
        const Paq8pxLinearPredictionBackend linear_prediction;
        std::vector<std::uint8_t> payload = linear_prediction.encode(input);
        decision.paq8px_linear_prediction_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::Paq8pxLinearPredictionOnly) {
            decision.mode = BlockMode::Paq8pxLinearPrediction;
            decision.entropy = EntropyKind::Paq8pxLinearPrediction;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Paq8pxLinearPrediction,
                 TransformKind::Raw, EntropyKind::Paq8pxLinearPrediction,
                 std::move(payload));
    }

    if (options_.policy == CandidatePolicy::Paq8pxSimilarityOnly ||
        auto_match) {
        const Paq8pxSimilarityBackend similarity;
        std::vector<std::uint8_t> payload = similarity.encode(input);
        decision.paq8px_similarity_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::Paq8pxSimilarityOnly) {
            decision.mode = BlockMode::Paq8pxSimilarity;
            decision.entropy = EntropyKind::Paq8pxSimilarity;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Paq8pxSimilarity,
                 TransformKind::Raw, EntropyKind::Paq8pxSimilarity,
                 std::move(payload));
    }

    if (options_.policy == CandidatePolicy::Paq8pxSimilaritySseOnly ||
        auto_match || shortlist_has(BlockMode::Paq8pxSimilaritySse)) {
        const Paq8pxSimilaritySseBackend similarity_sse;
        std::vector<std::uint8_t> payload = similarity_sse.encode(input);
        decision.paq8px_similarity_sse_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::Paq8pxSimilaritySseOnly) {
            decision.mode = BlockMode::Paq8pxSimilaritySse;
            decision.entropy = EntropyKind::Paq8pxSimilaritySse;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Paq8pxSimilaritySse,
                 TransformKind::Raw, EntropyKind::Paq8pxSimilaritySse,
                 std::move(payload));
    }

    if (options_.policy == CandidatePolicy::Paq8pxGenericSseOnly ||
        auto_match || shortlist_has(BlockMode::Paq8pxGenericSse)) {
        const Paq8pxGenericSseBackend generic_sse;
        std::vector<std::uint8_t> payload = generic_sse.encode(input);
        decision.paq8px_generic_sse_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::Paq8pxGenericSseOnly) {
            decision.mode = BlockMode::Paq8pxGenericSse;
            decision.entropy = EntropyKind::Paq8pxGenericSse;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Paq8pxGenericSse,
                 TransformKind::Raw, EntropyKind::Paq8pxGenericSse,
                 std::move(payload));
    }

    if (options_.policy == CandidatePolicy::Paq8pxDetectedSseOnly ||
        auto_match || shortlist_has(BlockMode::Paq8pxDetectedSse)) {
        const Paq8pxDetectedSseBackend detected_sse;
        std::vector<std::uint8_t> payload = detected_sse.encode(input);
        decision.paq8px_detected_sse_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::Paq8pxDetectedSseOnly) {
            decision.mode = BlockMode::Paq8pxDetectedSse;
            decision.entropy = EntropyKind::Paq8pxDetectedSse;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Paq8pxDetectedSse,
                 TransformKind::Raw, EntropyKind::Paq8pxDetectedSse,
                 std::move(payload));
    }

    if (options_.policy == CandidatePolicy::ZstdOnly ||
        options_.policy == CandidatePolicy::Fast || auto_lz ||
        shortlist_has(BlockMode::Zstd)) {
        const int zstd_level = options_.policy == CandidatePolicy::Fast
            ? std::min(options_.zstd_level, 3) : options_.zstd_level;
        const ZstdBackend zstd(zstd_level);
        std::vector<std::uint8_t> payload = zstd.encode(input);
        decision.zstd_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::ZstdOnly ||
            options_.policy == CandidatePolicy::Fast) {
            decision.mode = BlockMode::Zstd;
            decision.entropy = EntropyKind::ZstdFse;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Zstd, TransformKind::Raw,
                 EntropyKind::ZstdFse,
                 std::move(payload));
    }

    if (options_.policy == CandidatePolicy::FseOnly || auto_lz ||
        shortlist_has(BlockMode::Fse)) {
        const FseBackend fse;
        std::vector<std::uint8_t> payload = fse.encode(input);
        decision.fse_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::FseOnly) {
            decision.mode = BlockMode::Fse;
            decision.entropy = EntropyKind::Fse;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Fse, TransformKind::Raw, EntropyKind::Fse,
                 std::move(payload));
    }

    if (options_.policy == CandidatePolicy::LzmaOnly || auto_lz ||
        shortlist_has(BlockMode::Lzma)) {
        const LzmaBackend lzma(options_.lzma_level,
                               options_.lzma_dictionary_size);
        std::vector<std::uint8_t> payload = lzma.encode(input);
        decision.lzma_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::LzmaOnly) {
            decision.mode = BlockMode::Lzma;
            decision.entropy = EntropyKind::Lzma;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Lzma, TransformKind::Raw, EntropyKind::Lzma,
                 std::move(payload));
    }

    if (options_.policy == CandidatePolicy::Lz4Only || auto_lz) {
        const Lz4Backend lz4;
        std::vector<std::uint8_t> payload = lz4.encode(input);
        decision.lz4_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::Lz4Only) {
            decision.mode = BlockMode::Lz4;
            decision.entropy = EntropyKind::Lz4;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Lz4, TransformKind::Raw,
                 EntropyKind::Lz4, std::move(payload));
    }

    if (options_.policy == CandidatePolicy::KanziAnsOnly || auto_lz) {
        std::vector<std::uint8_t> payload = KanziAnsBackend().encode(input);
        decision.kanzi_ans_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::KanziAnsOnly) {
            decision.mode = BlockMode::KanziAns;
            decision.entropy = EntropyKind::KanziAns;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::KanziAns, TransformKind::Raw,
                 EntropyKind::KanziAns, std::move(payload));
    }

    if (options_.policy == CandidatePolicy::Ppmd7Only || auto_lz ||
        shortlist_has(BlockMode::Ppmd7)) {
        const Ppmd7Backend ppmd7;
        std::vector<std::uint8_t> payload = ppmd7.encode(input);
        decision.ppmd7_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::Ppmd7Only) {
            decision.mode = BlockMode::Ppmd7;
            decision.entropy = EntropyKind::Ppmd7;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Ppmd7, TransformKind::Raw,
                 EntropyKind::Ppmd7, std::move(payload));
    }

    if (options_.policy == CandidatePolicy::Ppmd8Only || auto_lz ||
        shortlist_has(BlockMode::Ppmd8)) {
        const Ppmd8Backend ppmd8;
        std::vector<std::uint8_t> payload = ppmd8.encode(input);
        decision.ppmd8_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::Ppmd8Only) {
            decision.mode = BlockMode::Ppmd8;
            decision.entropy = EntropyKind::Ppmd8;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Ppmd8, TransformKind::Raw,
                 EntropyKind::Ppmd8, std::move(payload));
    }

    if (options_.policy == CandidatePolicy::ZpaqOnly || auto_lz) {
        const ZpaqBackend zpaq;
        std::vector<std::uint8_t> payload = zpaq.encode(input);
        decision.zpaq_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::ZpaqOnly) {
            decision.mode = BlockMode::Zpaq;
            decision.entropy = EntropyKind::Zpaq;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Zpaq, TransformKind::Raw,
                 EntropyKind::Zpaq, std::move(payload));
    }

    if (options_.policy == CandidatePolicy::CtwOnly ||
        (auto_lz && input.size() <= CtwBackend::kMaximumOutputSize)) {
        const CtwBackend ctw;
        std::vector<std::uint8_t> payload = ctw.encode(input);
        decision.ctw_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::CtwOnly) {
            decision.mode = BlockMode::Ctw;
            decision.entropy = EntropyKind::Ctw;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Ctw, TransformKind::Raw,
                 EntropyKind::Ctw, std::move(payload));
    }

    if (options_.policy == CandidatePolicy::BwtZstdOnly ||
        (automatic && activation.bwt_zstd) || shortlist_has(BlockMode::BwtZstd)) {
        const BwtTransform bwt;
        const TransformResult transformed = bwt.forward(input);
        const ZstdBackend zstd(options_.zstd_level);
        std::vector<std::uint8_t> payload = zstd.encode(
            ByteView(transformed.bytes));
        decision.bwt_zstd_candidate_bytes =
            payload.size() + transformed.side_information.size();
        if (options_.policy == CandidatePolicy::BwtZstdOnly) {
            decision.mode = BlockMode::BwtZstd;
            decision.transform = TransformKind::Bwt;
            decision.entropy = EntropyKind::ZstdFse;
            decision.payload = std::move(payload);
            decision.transform_metadata = transformed.side_information;
            return decision;
        }
        consider(decision, BlockMode::BwtZstd, TransformKind::Bwt,
                 EntropyKind::ZstdFse, std::move(payload),
                 transformed.side_information);
    }

    if (options_.policy == CandidatePolicy::BwtMtfZstdOnly ||
        (automatic && activation.bwt_mtf_zstd)) {
        const TransformResult bwt = BwtTransform().forward(input);
        const TransformResult mtf = KanziMtfTransform().forward(ByteView(bwt.bytes));
        std::vector<std::uint8_t> payload = ZstdBackend(options_.zstd_level).encode(ByteView(mtf.bytes));
        decision.bwt_mtf_zstd_candidate_bytes = payload.size() + bwt.side_information.size();
        if (options_.policy == CandidatePolicy::BwtMtfZstdOnly) {
            decision.mode = BlockMode::BwtMtfZstd;
            decision.transform = TransformKind::BwtMtf;
            decision.entropy = EntropyKind::ZstdFse;
            decision.payload = std::move(payload);
            decision.transform_metadata = std::move(bwt.side_information);
            return decision;
        }
        consider(decision, BlockMode::BwtMtfZstd, TransformKind::BwtMtf,
                 EntropyKind::ZstdFse, std::move(payload),
                 std::move(bwt.side_information));
    }

    if (options_.policy == CandidatePolicy::BwtRltZstdOnly ||
        (automatic && activation.bwt_rlt_zstd)) {
        const TransformResult bwt = BwtTransform().forward(input);
        const std::optional<std::vector<std::uint8_t>> rlt =
            KanziRltTransform().forward_if_smaller(ByteView(bwt.bytes));
        if (!rlt.has_value()) {
            if (options_.policy == CandidatePolicy::BwtRltZstdOnly) {
                throw std::runtime_error(
                    "Kanzi RLT did not reduce the BWT block");
            }
        } else {
            std::vector<std::uint8_t> payload =
                ZstdBackend(options_.zstd_level).encode(ByteView(*rlt));
            std::vector<std::uint8_t> metadata = bwt.side_information;
            append_u32_le(metadata, static_cast<std::uint32_t>(rlt->size()));
            decision.bwt_rlt_zstd_candidate_bytes =
                payload.size() + metadata.size();
            if (options_.policy == CandidatePolicy::BwtRltZstdOnly) {
                decision.mode = BlockMode::BwtRltZstd;
                decision.transform = TransformKind::BwtRlt;
                decision.entropy = EntropyKind::ZstdFse;
                decision.payload = std::move(payload);
                decision.transform_metadata = std::move(metadata);
                return decision;
            }
            consider(decision, BlockMode::BwtRltZstd, TransformKind::BwtRlt,
                     EntropyKind::ZstdFse, std::move(payload),
                     std::move(metadata));
        }
    }

    if (options_.policy == CandidatePolicy::X86BcjZstdOnly ||
        (automatic && activation.x86_bcj_zstd) ||
        shortlist_has(BlockMode::X86BcjZstd)) {
        const TransformResult bcj = XzX86BcjTransform().forward(input);
        std::vector<std::uint8_t> payload = ZstdBackend(options_.zstd_level).encode(ByteView(bcj.bytes));
        decision.x86_bcj_zstd_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::X86BcjZstdOnly) {
            decision.mode = BlockMode::X86BcjZstd;
            decision.transform = TransformKind::X86Bcj;
            decision.entropy = EntropyKind::ZstdFse;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::X86BcjZstd, TransformKind::X86Bcj,
                 EntropyKind::ZstdFse, std::move(payload));
    }

    if (options_.policy == CandidatePolicy::ShuffleZstdOnly ||
        (automatic && activation.shuffle_zstd) ||
        shortlist_has(BlockMode::ShuffleZstd)) {
        std::vector<std::uint8_t> best_payload;
        std::vector<std::uint8_t> best_transformed;
        std::uint8_t best_width = 0;
        const BloscShuffleTransform shuffle;
        for (const std::uint8_t width : {std::uint8_t{2}, std::uint8_t{4}, std::uint8_t{8}}) {
            const TransformResult transformed = shuffle.forward(input, width);
            std::vector<std::uint8_t> payload = ZstdBackend(options_.zstd_level).encode(ByteView(transformed.bytes));
            if (best_width == 0 || payload.size() < best_payload.size()) {
                best_payload = std::move(payload);
                best_transformed = transformed.bytes;
                best_width = width;
            }
        }
        decision.shuffle_zstd_candidate_bytes = best_payload.size() + 1;
        std::vector<std::uint8_t> metadata{best_width};
        if (options_.policy == CandidatePolicy::ShuffleZstdOnly) {
            decision.mode = BlockMode::ShuffleZstd;
            decision.transform = TransformKind::Shuffle;
            decision.entropy = EntropyKind::ZstdFse;
            decision.payload = std::move(best_payload);
            decision.transform_metadata = std::move(metadata);
            return decision;
        }
        consider(decision, BlockMode::ShuffleZstd, TransformKind::Shuffle, EntropyKind::ZstdFse,
                 std::move(best_payload), std::move(metadata));
    }

    if (options_.policy == CandidatePolicy::BitshuffleZstdOnly ||
        (automatic && activation.bitshuffle_zstd)) {
        std::vector<std::uint8_t> best_payload;
        std::uint8_t best_width = 0;
        const BloscBitshuffleTransform bitshuffle;
        for (const std::uint8_t width : {std::uint8_t{2}, std::uint8_t{4}, std::uint8_t{8}}) {
            if (!bitshuffle.applicable(input, width)) continue;
            const TransformResult transformed = bitshuffle.forward(input, width);
            std::vector<std::uint8_t> payload = ZstdBackend(options_.zstd_level).encode(ByteView(transformed.bytes));
            if (best_width == 0 || payload.size() < best_payload.size()) { best_payload = std::move(payload); best_width = width; }
        }
        if (best_width == 0) {
            if (options_.policy == CandidatePolicy::BitshuffleZstdOnly) throw std::runtime_error("C-Blosc2 bitshuffle is not applicable to this block");
        } else {
            decision.bitshuffle_zstd_candidate_bytes = best_payload.size() + 1;
            std::vector<std::uint8_t> metadata{best_width};
            if (options_.policy == CandidatePolicy::BitshuffleZstdOnly) {
                decision.mode = BlockMode::BitshuffleZstd; decision.transform = TransformKind::Bitshuffle;
                decision.entropy = EntropyKind::ZstdFse; decision.payload = std::move(best_payload);
                decision.transform_metadata = std::move(metadata); return decision;
            }
            consider(decision, BlockMode::BitshuffleZstd, TransformKind::Bitshuffle, EntropyKind::ZstdFse,
                     std::move(best_payload), std::move(metadata));
        }
    }

    if (options_.policy == CandidatePolicy::DeltaZstdOnly ||
        (automatic && activation.delta_zstd) || shortlist_has(BlockMode::DeltaZstd)) {
        std::vector<std::uint8_t> best_payload;
        std::uint8_t best_width = 0;
        const BloscDeltaTransform delta;
        for (const std::uint8_t width : {std::uint8_t{1}, std::uint8_t{2}, std::uint8_t{4}, std::uint8_t{8}}) {
            if (!delta.applicable(input, width)) continue;
            const TransformResult transformed = delta.forward(input, width);
            std::vector<std::uint8_t> payload = ZstdBackend(options_.zstd_level).encode(ByteView(transformed.bytes));
            if (best_width == 0 || payload.size() < best_payload.size()) { best_payload = std::move(payload); best_width = width; }
        }
        decision.delta_zstd_candidate_bytes = best_payload.size() + 1;
        std::vector<std::uint8_t> metadata{best_width};
        if (options_.policy == CandidatePolicy::DeltaZstdOnly) {
            decision.mode = BlockMode::DeltaZstd; decision.transform = TransformKind::Delta;
            decision.entropy = EntropyKind::ZstdFse; decision.payload = std::move(best_payload);
            decision.transform_metadata = std::move(metadata); return decision;
        }
        consider(decision, BlockMode::DeltaZstd, TransformKind::Delta, EntropyKind::ZstdFse,
                 std::move(best_payload), std::move(metadata));
    }

    if (options_.policy == CandidatePolicy::DeltaOfDeltaZstdOnly ||
        (automatic && activation.delta_of_delta_zstd)) {
        std::vector<std::uint8_t> best_payload;
        std::uint8_t best_width = 0;
        const DeltaOfDeltaTransform delta_of_delta;
        for (const std::uint8_t width : {std::uint8_t{4}, std::uint8_t{8}}) {
            if (!delta_of_delta.applicable(input, width)) {
                continue;
            }
            const TransformResult transformed =
                delta_of_delta.forward(input, width);
            std::vector<std::uint8_t> payload =
                ZstdBackend(options_.zstd_level).encode(
                    ByteView(transformed.bytes));
            if (best_width == 0 || payload.size() < best_payload.size()) {
                best_payload = std::move(payload);
                best_width = width;
            }
        }
        if (best_width == 0) {
            if (options_.policy == CandidatePolicy::DeltaOfDeltaZstdOnly) {
                throw std::runtime_error(
                    "Delta-of-delta requires at least three 32/64-bit words");
            }
        } else {
            decision.delta_of_delta_zstd_candidate_bytes =
                best_payload.size() + 1U;
            std::vector<std::uint8_t> metadata{best_width};
            if (options_.policy == CandidatePolicy::DeltaOfDeltaZstdOnly) {
                decision.mode = BlockMode::DeltaOfDeltaZstd;
                decision.transform = TransformKind::DeltaOfDelta;
                decision.entropy = EntropyKind::ZstdFse;
                decision.payload = std::move(best_payload);
                decision.transform_metadata = std::move(metadata);
                return decision;
            }
            consider(decision, BlockMode::DeltaOfDeltaZstd,
                     TransformKind::DeltaOfDelta, EntropyKind::ZstdFse,
                     std::move(best_payload), std::move(metadata));
        }
    }

    if (options_.policy == CandidatePolicy::DeltaBinaryPackedZstdOnly ||
        (automatic && activation.delta_binary_packed_zstd)) {
        std::vector<std::uint8_t> best_payload;
        std::vector<std::uint8_t> best_transformed;
        std::uint8_t best_width = 0;
        const DeltaBinaryPackedTransform delta_binary_packed;
        for (const std::uint8_t width : {std::uint8_t{4}, std::uint8_t{8}}) {
            if (!delta_binary_packed.applicable(input, width)) {
                continue;
            }
            const TransformResult transformed =
                delta_binary_packed.forward(input, width);
            std::vector<std::uint8_t> payload =
                ZstdBackend(options_.zstd_level).encode(
                    ByteView(transformed.bytes));
            if (best_width == 0 || payload.size() < best_payload.size()) {
                best_payload = std::move(payload);
                best_transformed = transformed.bytes;
                best_width = width;
            }
        }
        if (best_width == 0) {
            if (options_.policy == CandidatePolicy::DeltaBinaryPackedZstdOnly) {
                throw std::runtime_error(
                    "DeltaBinaryPacked requires a 32/64-bit integer block");
            }
        } else {
            decision.delta_binary_packed_zstd_candidate_bytes =
                best_payload.size() + 5U;
            std::vector<std::uint8_t> metadata{best_width};
            append_u32_le(metadata, static_cast<std::uint32_t>(
                best_transformed.size()));
            if (options_.policy == CandidatePolicy::DeltaBinaryPackedZstdOnly) {
                decision.mode = BlockMode::DeltaBinaryPackedZstd;
                decision.transform = TransformKind::DeltaBinaryPacked;
                decision.entropy = EntropyKind::ZstdFse;
                decision.payload = std::move(best_payload);
                decision.transform_metadata = std::move(metadata);
                return decision;
            }
            consider(decision, BlockMode::DeltaBinaryPackedZstd,
                     TransformKind::DeltaBinaryPacked, EntropyKind::ZstdFse,
                     std::move(best_payload), std::move(metadata));
        }
    }

    if (options_.policy == CandidatePolicy::FastPforOnly ||
        (automatic && activation.fastpfor)) {
        const FastPforBackend fastpfor;
        if (!fastpfor.applicable(input)) {
            if (options_.policy == CandidatePolicy::FastPforOnly) {
                throw std::runtime_error(
                    "FastPFOR requires at least one 1024-byte group");
            }
        } else {
            FastPforEncodedBlock encoded = fastpfor.encode(input);
            decision.fastpfor_candidate_bytes =
                encoded.payload.size() + encoded.metadata.size();
            if (options_.policy == CandidatePolicy::FastPforOnly) {
                decision.mode = BlockMode::FastPfor;
                decision.transform = TransformKind::FastPfor;
                decision.entropy = EntropyKind::FastPfor;
                decision.payload = std::move(encoded.payload);
                decision.transform_metadata = std::move(encoded.metadata);
                return decision;
            }
            consider(decision, BlockMode::FastPfor, TransformKind::FastPfor,
                     EntropyKind::FastPfor, std::move(encoded.payload),
                     std::move(encoded.metadata));
        }
    }

    if (options_.policy == CandidatePolicy::RansOnly ||
        (automatic && activation.rans)) {
        std::vector<std::uint8_t> payload = RansBackend().encode(input);
        decision.rans_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::RansOnly) {
            decision.mode = BlockMode::Rans;
            decision.transform = TransformKind::Raw;
            decision.entropy = EntropyKind::Rans;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Rans, TransformKind::Raw,
                 EntropyKind::Rans, std::move(payload));
    }

    if (options_.policy == CandidatePolicy::BrotliTextOnly ||
        (automatic && activation.brotli_text) ||
        shortlist_has(BlockMode::BrotliText)) {
        const BrotliTextTransform brotli;
        TransformResult encoded = brotli.forward(input);
        decision.brotli_text_candidate_bytes = encoded.bytes.size();
        if (options_.policy == CandidatePolicy::BrotliTextOnly) {
            decision.mode = BlockMode::BrotliText;
            decision.transform = TransformKind::BrotliText;
            decision.entropy = EntropyKind::BrotliText;
            decision.payload = std::move(encoded.bytes);
            return decision;
        }
        consider(decision, BlockMode::BrotliText, TransformKind::BrotliText,
                 EntropyKind::BrotliText, std::move(encoded.bytes));
    }

    if (options_.policy == CandidatePolicy::CmixWordDictionaryZstdOnly ||
        (automatic && activation.cmix_word_dictionary_zstd)) {
        const CmixWordDictionaryTransform dictionary;
        TransformResult transformed = dictionary.forward(input);
        std::vector<std::uint8_t> payload =
            ZstdBackend(options_.zstd_level).encode(ByteView(transformed.bytes));
        std::vector<std::uint8_t> metadata;
        append_u32_le(metadata,
                      static_cast<std::uint32_t>(transformed.bytes.size()));
        decision.cmix_word_dictionary_zstd_candidate_bytes =
            payload.size() + metadata.size();
        if (options_.policy == CandidatePolicy::CmixWordDictionaryZstdOnly) {
            decision.mode = BlockMode::CmixWordDictionaryZstd;
            decision.transform = TransformKind::CmixWordDictionary;
            decision.entropy = EntropyKind::ZstdFse;
            decision.payload = std::move(payload);
            decision.transform_metadata = std::move(metadata);
            return decision;
        }
        consider(decision, BlockMode::CmixWordDictionaryZstd,
                 TransformKind::CmixWordDictionary, EntropyKind::ZstdFse,
                 std::move(payload), std::move(metadata));
    }

    if (options_.policy == CandidatePolicy::NeuralLstmOnly ||
        (automatic && activation.neural_lstm)) {
        const NeuralLstmBackend neural(options_.model_seed);
        std::vector<std::uint8_t> payload = neural.encode(input);
        decision.neural_lstm_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::NeuralLstmOnly) {
            decision.mode = BlockMode::NeuralLstm;
            decision.transform = TransformKind::Raw;
            decision.entropy = EntropyKind::SymbolArithmetic;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::NeuralLstm, TransformKind::Raw,
                 EntropyKind::SymbolArithmetic, std::move(payload));
    }

    if (options_.policy == CandidatePolicy::SharedNeuralLstmOnly ||
        (automatic && activation.shared_neural_lstm)) {
        const NeuralLstmBackend neural(kSharedNeuralModelSeed);
        std::vector<std::uint8_t> payload = neural.encode(input);
        std::vector<std::uint8_t> metadata;
        append_u32_le(metadata, kSharedNeuralModelId);
        decision.shared_neural_lstm_candidate_bytes =
            payload.size() + metadata.size();
        if (options_.policy == CandidatePolicy::SharedNeuralLstmOnly) {
            decision.mode = BlockMode::SharedNeuralLstm;
            decision.transform = TransformKind::NeuralShared;
            decision.entropy = EntropyKind::SymbolArithmetic;
            decision.payload = std::move(payload);
            decision.transform_metadata = std::move(metadata);
            return decision;
        }
        consider(decision, BlockMode::SharedNeuralLstm,
                 TransformKind::NeuralShared, EntropyKind::SymbolArithmetic,
                 std::move(payload), std::move(metadata));
    }

    if (options_.policy == CandidatePolicy::LstmCompressOnly ||
        (automatic && activation.lstm_compress)) {
        const LstmCompressBackend lstm(options_.model_seed);
        std::vector<std::uint8_t> payload = lstm.encode(input);
        decision.lstm_compress_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::LstmCompressOnly) {
            decision.mode = BlockMode::LstmCompress;
            decision.transform = TransformKind::NeuralLstm;
            decision.entropy = EntropyKind::SymbolArithmetic;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::LstmCompress, TransformKind::NeuralLstm,
                 EntropyKind::SymbolArithmetic, std::move(payload));
    }

    if (options_.policy == CandidatePolicy::BgptSharedPriorOnly ||
        (automatic && activation.bgpt_shared_prior)) {
        std::vector<std::uint8_t> payload =
            BgptSharedPriorBackend().encode(input);
        std::vector<std::uint8_t> metadata;
        append_u32_le(metadata, kBgptSharedPriorModelId);
        metadata.insert(metadata.end(), kBgptTextCheckpointSha256.begin(),
                        kBgptTextCheckpointSha256.end());
        decision.bgpt_shared_prior_candidate_bytes =
            payload.size() + metadata.size();
        if (options_.policy == CandidatePolicy::BgptSharedPriorOnly) {
            decision.mode = BlockMode::BgptSharedPrior;
            decision.transform = TransformKind::NeuralSharedPrior;
            decision.entropy = EntropyKind::SymbolArithmetic;
            decision.payload = std::move(payload);
            decision.transform_metadata = std::move(metadata);
            return decision;
        }
        consider(decision, BlockMode::BgptSharedPrior,
                 TransformKind::NeuralSharedPrior,
                 EntropyKind::SymbolArithmetic, std::move(payload),
                 std::move(metadata));
    }

    if (options_.policy == CandidatePolicy::JaxCompressPortableOnly ||
        (automatic && activation.jax_compress_portable)) {
        std::vector<std::uint8_t> payload =
            JaxCompressPortableBackend().encode(input);
        std::vector<std::uint8_t> metadata;
        append_u32_le(metadata, kJaxCompressPortableModelId);
        metadata.insert(metadata.end(), kJaxCompressSourceRevision.begin(),
                        kJaxCompressSourceRevision.end());
        metadata.insert(metadata.end(),
                        kJaxCompressPortableProfileSha256.begin(),
                        kJaxCompressPortableProfileSha256.end());
        decision.jax_compress_portable_candidate_bytes =
            payload.size() + metadata.size();
        if (options_.policy == CandidatePolicy::JaxCompressPortableOnly) {
            decision.mode = BlockMode::JaxCompressPortable;
            decision.transform = TransformKind::NeuralOnlinePortable;
            decision.entropy = EntropyKind::SymbolArithmetic;
            decision.payload = std::move(payload);
            decision.transform_metadata = std::move(metadata);
            return decision;
        }
        consider(decision, BlockMode::JaxCompressPortable,
                 TransformKind::NeuralOnlinePortable,
                 EntropyKind::SymbolArithmetic, std::move(payload),
                 std::move(metadata));
    }

    if (options_.policy == CandidatePolicy::LmicArithmeticOnly ||
        (automatic && activation.lmic_arithmetic)) {
        std::vector<std::uint8_t> payload =
            LmicArithmeticBackend().encode(input);
        std::vector<std::uint8_t> metadata;
        append_u32_le(metadata, kLmicArithmeticModelId);
        metadata.insert(metadata.end(), kLmicArithmeticProfileSha256.begin(),
                        kLmicArithmeticProfileSha256.end());
        decision.lmic_arithmetic_candidate_bytes =
            payload.size() + metadata.size();
        if (options_.policy == CandidatePolicy::LmicArithmeticOnly) {
            decision.mode = BlockMode::LmicArithmetic;
            decision.transform = TransformKind::NeuralLmicArithmetic;
            decision.entropy = EntropyKind::LmicArithmetic;
            decision.payload = std::move(payload);
            decision.transform_metadata = std::move(metadata);
            return decision;
        }
        consider(decision, BlockMode::LmicArithmetic,
                 TransformKind::NeuralLmicArithmetic,
                 EntropyKind::LmicArithmetic, std::move(payload),
                 std::move(metadata));
    }

    if (options_.policy == CandidatePolicy::Bcj2ZstdOnly ||
        (automatic && activation.x86_bcj_zstd) ||
        shortlist_has(BlockMode::Bcj2Zstd)) {
        const TransformResult transformed = Bcj2Transform().forward(input);
        std::vector<std::uint8_t> payload = ZstdBackend(options_.zstd_level).encode(
            ByteView(transformed.bytes));
        decision.bcj2_zstd_candidate_bytes =
            payload.size() + transformed.side_information.size();
        if (options_.policy == CandidatePolicy::Bcj2ZstdOnly) {
            decision.mode = BlockMode::Bcj2Zstd;
            decision.transform = TransformKind::Bcj2;
            decision.entropy = EntropyKind::ZstdFse;
            decision.payload = std::move(payload);
            decision.transform_metadata = std::move(transformed.side_information);
            return decision;
        }
        consider(decision, BlockMode::Bcj2Zstd, TransformKind::Bcj2,
                 EntropyKind::ZstdFse, std::move(payload),
                 std::move(transformed.side_information));
    }

    if (options_.policy == CandidatePolicy::RecordTransposeZstdOnly ||
        (automatic && activation.record_transpose_zstd)) {
        std::vector<std::uint8_t> best_payload;
        std::uint8_t best_width = 0;
        const RecordTransposeTransform transpose;
        for (const std::uint8_t width : {std::uint8_t{16}, std::uint8_t{32}}) {
            if (!transpose.applicable(input, width)) {
                continue;
            }
            const TransformResult transformed = transpose.forward(input, width);
            std::vector<std::uint8_t> payload =
                ZstdBackend(options_.zstd_level).encode(ByteView(transformed.bytes));
            if (best_width == 0 || payload.size() < best_payload.size()) {
                best_payload = std::move(payload);
                best_width = width;
            }
        }
        if (best_width == 0) {
            if (options_.policy == CandidatePolicy::RecordTransposeZstdOnly) {
                throw std::runtime_error(
                    "record transpose requires a whole 16- or 32-byte record block");
            }
        } else {
            decision.record_transpose_zstd_candidate_bytes =
                best_payload.size() + 1U;
            std::vector<std::uint8_t> metadata{best_width};
            if (options_.policy == CandidatePolicy::RecordTransposeZstdOnly) {
                decision.mode = BlockMode::RecordTransposeZstd;
                decision.transform = TransformKind::RecordTranspose;
                decision.entropy = EntropyKind::ZstdFse;
                decision.payload = std::move(best_payload);
                decision.transform_metadata = std::move(metadata);
                return decision;
            }
            consider(decision, BlockMode::RecordTransposeZstd,
                     TransformKind::RecordTranspose, EntropyKind::ZstdFse,
                     std::move(best_payload), std::move(metadata));
        }
    }

    if (options_.policy == CandidatePolicy::JpegLsOnly ||
        (automatic && activation.jpeg_ls)) {
        std::vector<std::uint8_t> best_payload;
        std::vector<std::uint8_t> best_metadata;
        const JpegLsTransform jpeg_ls;
        for (const std::uint32_t width : {16U, 32U, 64U, 128U, 256U, 512U,
                                          1024U, 2048U, 4096U}) {
            if (!jpeg_ls.applicable(input, width) || input.size() / width < 2U) {
                continue;
            }
            TransformResult encoded = jpeg_ls.forward(input, width);
            if (best_metadata.empty() || encoded.bytes.size() < best_payload.size()) {
                best_payload = std::move(encoded.bytes);
                best_metadata = std::move(encoded.side_information);
            }
        }
        if (best_metadata.empty()) {
            if (options_.policy == CandidatePolicy::JpegLsOnly) {
                throw std::runtime_error(
                    "JPEG-LS requires a block divisible by a supported image width");
            }
        } else {
            decision.jpeg_ls_candidate_bytes =
                best_payload.size() + best_metadata.size();
            if (options_.policy == CandidatePolicy::JpegLsOnly) {
                decision.mode = BlockMode::JpegLs;
                decision.transform = TransformKind::JpegLs;
                decision.entropy = EntropyKind::JpegLs;
                decision.payload = std::move(best_payload);
                decision.transform_metadata = std::move(best_metadata);
                return decision;
            }
            consider(decision, BlockMode::JpegLs, TransformKind::JpegLs,
                     EntropyKind::JpegLs, std::move(best_payload),
                     std::move(best_metadata));
        }
    }

    if (options_.policy == CandidatePolicy::FlacResidualOnly ||
        (automatic && activation.flac_residual)) {
        const FlacResidualTransform flac;
        if (!flac.applicable(input)) {
            if (options_.policy == CandidatePolicy::FlacResidualOnly) {
                throw std::runtime_error(
                    "FLAC residual requires at least 32 signed 16-bit PCM samples");
            }
        } else {
            TransformResult encoded = flac.forward(input);
            decision.flac_residual_candidate_bytes =
                encoded.bytes.size() + encoded.side_information.size();
            if (options_.policy == CandidatePolicy::FlacResidualOnly) {
                decision.mode = BlockMode::FlacResidual;
                decision.transform = TransformKind::FlacResidual;
                decision.entropy = EntropyKind::FlacResidual;
                decision.payload = std::move(encoded.bytes);
                decision.transform_metadata = std::move(encoded.side_information);
                return decision;
            }
            consider(decision, BlockMode::FlacResidual,
                     TransformKind::FlacResidual, EntropyKind::FlacResidual,
                     std::move(encoded.bytes),
                     std::move(encoded.side_information));
        }
    }

    if (options_.policy == CandidatePolicy::WavpackOnly ||
        (automatic && activation.wavpack)) {
        std::vector<std::uint8_t> payload = WavpackBackend().encode(input);
        decision.wavpack_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::WavpackOnly) {
            decision.mode = BlockMode::Wavpack;
            decision.transform = TransformKind::Raw;
            decision.entropy = EntropyKind::Wavpack;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Wavpack, TransformKind::Raw,
                 EntropyKind::Wavpack, std::move(payload));
    }

    const std::array<std::optional<std::size_t>, 43> candidates{
        decision.stored_candidate_bytes,
        decision.predictive_candidate_bytes,
        decision.zstd_candidate_bytes,
        decision.fse_candidate_bytes,
        decision.lzma_candidate_bytes,
        decision.donor_match_predictive_candidate_bytes,
        decision.paq8px_apm_predictive_candidate_bytes,
        decision.paq8px_record_model_candidate_bytes,
        decision.paq8px_linear_prediction_candidate_bytes,
        decision.bwt_zstd_candidate_bytes,
        decision.bwt_mtf_zstd_candidate_bytes,
        decision.bwt_rlt_zstd_candidate_bytes,
        decision.x86_bcj_zstd_candidate_bytes,
        decision.shuffle_zstd_candidate_bytes,
        decision.bitshuffle_zstd_candidate_bytes,
        decision.delta_zstd_candidate_bytes,
        decision.delta_of_delta_zstd_candidate_bytes,
        decision.fastpfor_candidate_bytes,
        decision.rans_candidate_bytes,
        decision.bcj2_zstd_candidate_bytes,
        decision.record_transpose_zstd_candidate_bytes,
        decision.jpeg_ls_candidate_bytes,
        decision.flac_residual_candidate_bytes,
        decision.brotli_text_candidate_bytes,
        decision.cmix_word_dictionary_zstd_candidate_bytes,
        decision.neural_lstm_candidate_bytes,
        decision.shared_neural_lstm_candidate_bytes,
        decision.lstm_compress_candidate_bytes,
        decision.bgpt_shared_prior_candidate_bytes,
        decision.jax_compress_portable_candidate_bytes,
        decision.lmic_arithmetic_candidate_bytes,
        decision.ppmd7_candidate_bytes,
        decision.ppmd8_candidate_bytes,
        decision.zpaq_candidate_bytes,
        decision.ctw_candidate_bytes,
        decision.paq8px_similarity_candidate_bytes,
        decision.paq8px_similarity_sse_candidate_bytes,
        decision.paq8px_generic_sse_candidate_bytes,
        decision.paq8px_detected_sse_candidate_bytes,
        decision.wavpack_candidate_bytes,
        decision.lz4_candidate_bytes,
        decision.kanzi_ans_candidate_bytes,
        decision.delta_binary_packed_zstd_candidate_bytes};
    // Candidate byte fields are grouped by backend construction order, not by
    // decoder-visible BlockMode ID. Keep this mapping explicit so encoder
    // telemetry cannot silently relabel a materialized candidate.
    const std::array<BlockMode, 43> candidate_mode_ids{
        BlockMode::Stored,
        BlockMode::PredictiveV1,
        BlockMode::Zstd,
        BlockMode::Fse,
        BlockMode::Lzma,
        BlockMode::DonorMatchPredictive,
        BlockMode::Paq8pxApmPredictive,
        BlockMode::Paq8pxRecordModel,
        BlockMode::Paq8pxLinearPrediction,
        BlockMode::BwtZstd,
        BlockMode::BwtMtfZstd,
        BlockMode::BwtRltZstd,
        BlockMode::X86BcjZstd,
        BlockMode::ShuffleZstd,
        BlockMode::BitshuffleZstd,
        BlockMode::DeltaZstd,
        BlockMode::DeltaOfDeltaZstd,
        BlockMode::FastPfor,
        BlockMode::Rans,
        BlockMode::Bcj2Zstd,
        BlockMode::RecordTransposeZstd,
        BlockMode::JpegLs,
        BlockMode::FlacResidual,
        BlockMode::BrotliText,
        BlockMode::CmixWordDictionaryZstd,
        BlockMode::NeuralLstm,
        BlockMode::SharedNeuralLstm,
        BlockMode::LstmCompress,
        BlockMode::BgptSharedPrior,
        BlockMode::JaxCompressPortable,
        BlockMode::LmicArithmetic,
        BlockMode::Ppmd7,
        BlockMode::Ppmd8,
        BlockMode::Zpaq,
        BlockMode::Ctw,
        BlockMode::Paq8pxSimilarity,
        BlockMode::Paq8pxSimilaritySse,
        BlockMode::Paq8pxGenericSse,
        BlockMode::Paq8pxDetectedSse,
        BlockMode::Wavpack,
        BlockMode::Lz4,
        BlockMode::KanziAns,
        BlockMode::DeltaBinaryPackedZstd};
    std::size_t oracle = std::numeric_limits<std::size_t>::max();
    for (std::size_t mode_index = 0; mode_index < candidates.size();
         ++mode_index) {
        const auto& candidate = candidates[mode_index];
        if (candidate.has_value()) {
            ++decision.candidates_evaluated;
            oracle = std::min(oracle, *candidate);
            const std::size_t candidate_mode_index = static_cast<std::size_t>(
                candidate_mode_ids[mode_index]);
            if ((automatic || shortlisted) &&
                decision.candidate_blocks_by_mode[candidate_mode_index] != 1U) {
                throw std::logic_error(
                    "R2 shortlist candidate accounting mismatch");
            }
        }
    }
    constexpr std::size_t kBlockArchiveOverhead =
        kR2BlockHeaderSize + kR2BlockChecksumSize;
    decision.selected_candidate_bytes = decision.payload.size() +
        decision.transform_metadata.size() + kBlockArchiveOverhead;
    decision.oracle_candidate_bytes = oracle ==
            std::numeric_limits<std::size_t>::max()
        ? decision.selected_candidate_bytes
        : oracle + kBlockArchiveOverhead;
    decision.oracle_gap_bytes = decision.selected_candidate_bytes >
            decision.oracle_candidate_bytes
        ? decision.selected_candidate_bytes - decision.oracle_candidate_bytes
        : 0;
    update_family_telemetry(decision, input.size());
    return decision;
}

}  // namespace hz::r2
