#pragma once

#include <vector>

#include "r2/archive/r2_archive.h"
#include "r2/core/byte_view.h"
#include "r2/routing/block_features.h"

namespace hz::r2 {

std::vector<BlockMode> rank_modes(const BlockFeaturesV1& features,
                                  std::uint8_t candidate_count);
std::vector<BlockMode> rank_modes(ByteView input,
                                  std::uint8_t candidate_count);
std::vector<BlockMode> rank_modes_k2(const BlockFeaturesV1& features);
std::vector<BlockMode> rank_modes_k4(const BlockFeaturesV1& features);
std::vector<BlockMode> rank_modes_k8(const BlockFeaturesV1& features);
std::vector<BlockMode> rank_modes_k8(ByteView input);
bool shortlist_contains(const std::vector<BlockMode>& modes,
                        BlockMode mode) noexcept;

}  // namespace hz::r2
