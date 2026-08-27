#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "r2/archive/r2_archive.h"
#include "r2/core/byte_view.h"
#include "r2/routing/block_features.h"

namespace hz::r2 {

constexpr std::size_t kFixedPointRankerModeCount = 43U;
constexpr std::size_t kFixedPointRankerModelV1CanonicalByteCount = 2644U;

struct FixedPointRankerModelV1 {
    std::array<std::array<std::int16_t, kBlockFeatureCount>,
               kFixedPointRankerModeCount> weights{};
    std::array<std::int32_t, kFixedPointRankerModeCount> biases{};
    std::array<std::int16_t, kBlockFeatureCount> feature_shifts{};
    std::uint32_t version = 0U;
    std::uint32_t crc32 = 0U;
};

static_assert(sizeof(FixedPointRankerModelV1) ==
                  kFixedPointRankerModelV1CanonicalByteCount,
              "R2 V1 fixed-point ranker model layout changed");

const FixedPointRankerModelV1& fixed_point_ranker_model_v1() noexcept;
bool fixed_point_ranker_model_v1_valid() noexcept;
// SHA-256 of the canonical little-endian 2,644-byte model image. This is
// encoder telemetry, not HZ02 archive metadata.
const std::string& fixed_point_ranker_model_v1_sha256_hex() noexcept;
std::int64_t fixed_point_ranker_score(const BlockFeaturesV1& features,
                                      BlockMode mode) noexcept;

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
