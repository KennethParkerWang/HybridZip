#include "r2/block/block_planner.h"

#include <stdexcept>
#include <utility>

#include "r2/entropy/donor_match_predictive_backend.h"
#include "r2/entropy/predictive_v1_backend.h"
#include "r2/entropy/fse_backend.h"
#include "r2/entropy/lzma_backend.h"
#include "r2/entropy/stored_backend.h"
#include "r2/entropy/zstd_backend.h"

namespace hz::r2 {
namespace {

void consider(BlockDecision& decision,
              const BlockMode mode,
              const EntropyKind entropy,
              std::vector<std::uint8_t> payload) {
    if (payload.size() < decision.payload.size()) {
        decision.mode = mode;
        decision.entropy = entropy;
        decision.payload = std::move(payload);
    }
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
        consider(decision, BlockMode::PredictiveV1,
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
        consider(decision, BlockMode::DonorMatchPredictive,
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
        consider(decision, BlockMode::Zstd, EntropyKind::ZstdFse,
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
        consider(decision, BlockMode::Fse, EntropyKind::Fse,
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
        consider(decision, BlockMode::Lzma, EntropyKind::Lzma,
                 std::move(payload));
    }

    return decision;
}

}  // namespace hz::r2
