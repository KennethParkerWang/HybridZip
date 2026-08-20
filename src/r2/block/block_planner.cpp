#include "r2/block/block_planner.h"

#include <stdexcept>
#include <utility>

#include "r2/entropy/donor_match_predictive_backend.h"
#include "r2/entropy/predictive_v1_backend.h"
#include "r2/entropy/fse_backend.h"
#include "r2/entropy/lzma_backend.h"
#include "r2/entropy/stored_backend.h"
#include "r2/entropy/zstd_backend.h"
#include "r2/representation/bwt_transform.h"
#include "r2/representation/kanzi_mtf_transform.h"
#include "r2/representation/kanzi_rlt_transform.h"
#include "r2/representation/xz_x86_bcj_transform.h"

namespace hz::r2 {
namespace {

void consider(BlockDecision& decision,
              const BlockMode mode,
              const TransformKind transform,
              const EntropyKind entropy,
              std::vector<std::uint8_t> payload,
              std::vector<std::uint8_t> transform_metadata = {}) {
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

BlockDecision BlockPlanner::plan(const ByteView input) const {
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

    if (options_.policy == CandidatePolicy::PredictiveV1Only ||
        options_.policy == CandidatePolicy::Auto) {
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
        options_.policy == CandidatePolicy::Auto) {
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

    if (options_.policy == CandidatePolicy::ZstdOnly ||
        options_.policy == CandidatePolicy::Auto) {
        const ZstdBackend zstd(options_.zstd_level);
        std::vector<std::uint8_t> payload = zstd.encode(input);
        decision.zstd_candidate_bytes = payload.size();
        if (options_.policy == CandidatePolicy::ZstdOnly) {
            decision.mode = BlockMode::Zstd;
            decision.entropy = EntropyKind::ZstdFse;
            decision.payload = std::move(payload);
            return decision;
        }
        consider(decision, BlockMode::Zstd, TransformKind::Raw,
                 EntropyKind::ZstdFse,
                 std::move(payload));
    }

    if (options_.policy == CandidatePolicy::FseOnly ||
        options_.policy == CandidatePolicy::Auto) {
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

    if (options_.policy == CandidatePolicy::LzmaOnly ||
        options_.policy == CandidatePolicy::Auto) {
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

    if (options_.policy == CandidatePolicy::BwtZstdOnly ||
        options_.policy == CandidatePolicy::Auto) {
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
        options_.policy == CandidatePolicy::Auto) {
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
        options_.policy == CandidatePolicy::Auto) {
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
        options_.policy == CandidatePolicy::Auto) {
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

    return decision;
}

}  // namespace hz::r2
