#include "r2/routing/mode_ranker.h"

#include <algorithm>
#include <stdexcept>

namespace hz::r2 {
namespace {

void append_unique(std::vector<BlockMode>& modes, const BlockMode mode) {
    if (!shortlist_contains(modes, mode)) {
        modes.push_back(mode);
    }
}

}  // namespace

std::vector<BlockMode> rank_modes(const BlockFeaturesV1& features,
                                  const std::uint8_t candidate_count) {
    if (candidate_count != 2U && candidate_count != 4U &&
        candidate_count != 8U) {
        throw std::invalid_argument("R2 shortlist size must be 2, 4, or 8");
    }

    std::vector<BlockMode> modes;
    modes.reserve(candidate_count);
    append_unique(modes, BlockMode::Stored);
    // K=2 is an ablation-only lower bound. Generic SSE had the most wins in
    // the current frozen 32 KiB ledger; this does not establish generality.
    append_unique(modes, BlockMode::Paq8pxGenericSse);
    if (candidate_count == 2U) {
        return modes;
    }

    append_unique(modes, BlockMode::Zstd);
    append_unique(modes, BlockMode::Paq8pxDetectedSse);
    if (candidate_count == 4U) {
        return modes;
    }

    switch (features.classify()) {
    case BlockClass::Text:
        append_unique(modes, BlockMode::Ppmd7);
        append_unique(modes, BlockMode::Ppmd8);
        append_unique(modes, BlockMode::BrotliText);
        append_unique(modes, BlockMode::BwtZstd);
        break;
    case BlockClass::X86:
        append_unique(modes, BlockMode::Fse);
        append_unique(modes, BlockMode::Lzma);
        append_unique(modes, BlockMode::X86BcjZstd);
        append_unique(modes, BlockMode::Bcj2Zstd);
        break;
    case BlockClass::Numeric:
        append_unique(modes, BlockMode::Fse);
        append_unique(modes, BlockMode::Lzma);
        append_unique(modes, BlockMode::DeltaZstd);
        append_unique(modes, BlockMode::ShuffleZstd);
        break;
    case BlockClass::Generic:
        append_unique(modes, BlockMode::Fse);
        append_unique(modes, BlockMode::Lzma);
        append_unique(modes, BlockMode::Ppmd7);
        append_unique(modes, BlockMode::Ppmd8);
        break;
    }
    return modes;
}

std::vector<BlockMode> rank_modes(const ByteView input,
                                  const std::uint8_t candidate_count) {
    return rank_modes(extract_block_features(input), candidate_count);
}

std::vector<BlockMode> rank_modes_k2(const BlockFeaturesV1& features) {
    return rank_modes(features, 2U);
}

std::vector<BlockMode> rank_modes_k4(const BlockFeaturesV1& features) {
    return rank_modes(features, 4U);
}

std::vector<BlockMode> rank_modes_k8(const BlockFeaturesV1& features) {
    return rank_modes(features, 8U);
}

std::vector<BlockMode> rank_modes_k8(const ByteView input) {
    return rank_modes(input, 8U);
}

bool shortlist_contains(const std::vector<BlockMode>& modes,
                        const BlockMode mode) noexcept {
    return std::find(modes.begin(), modes.end(), mode) != modes.end();
}

}  // namespace hz::r2
