#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "r2/archive/r2_archive.h"
#include "r2/core/byte_view.h"

namespace hz::r2 {

enum class CandidatePolicy {
    Auto,
    StoredOnly,
    PredictiveV1Only,
    ZstdOnly,
    FseOnly,
    LzmaOnly,
    DonorMatchPredictiveOnly,
    BwtZstdOnly,
    BwtMtfZstdOnly,
    BwtRltZstdOnly,
    X86BcjZstdOnly
};

struct BlockPlannerOptions {
    CandidatePolicy policy = CandidatePolicy::Auto;
    int zstd_level = 19;
    int lzma_level = 9;
    std::uint32_t lzma_dictionary_size = 0;
    std::uint64_t model_seed = kDefaultModelSeed;
};

struct BlockDecision {
    BlockMode mode = BlockMode::Stored;
    TransformKind transform = TransformKind::Raw;
    EntropyKind entropy = EntropyKind::Stored;
    std::vector<std::uint8_t> payload;
    std::vector<std::uint8_t> transform_metadata;
    std::size_t stored_candidate_bytes = 0;
    std::optional<std::size_t> predictive_candidate_bytes;
    std::optional<std::size_t> zstd_candidate_bytes;
    std::optional<std::size_t> fse_candidate_bytes;
    std::optional<std::size_t> lzma_candidate_bytes;
    std::optional<std::size_t> donor_match_predictive_candidate_bytes;
    std::optional<std::size_t> bwt_zstd_candidate_bytes;
    std::optional<std::size_t> bwt_mtf_zstd_candidate_bytes;
    std::optional<std::size_t> bwt_rlt_zstd_candidate_bytes;
    std::optional<std::size_t> x86_bcj_zstd_candidate_bytes;
};

class BlockPlanner {
public:
    explicit BlockPlanner(BlockPlannerOptions options)
        : options_(options) {}

    BlockDecision plan(ByteView input) const;

private:
    BlockPlannerOptions options_;
};

}  // namespace hz::r2
