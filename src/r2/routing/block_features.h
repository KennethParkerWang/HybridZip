#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "r2/core/byte_view.h"

namespace hz::r2 {

enum class BlockClass : std::uint8_t {
    Text,
    X86,
    Numeric,
    Generic
};

enum class BlockFeatureId : std::uint8_t {
    BlockSizeBucket = 0,
    ByteEntropyQ12,
    CoarseEntropyQ12,
    MaximumByteFrequencyQ12,
    ZeroFractionQ12,
    FfFractionQ12,
    PrintableFractionQ12,
    HighBitFractionQ12,
    WhitespaceFractionQ12,
    NewlineFractionQ12,
    DigitFractionQ12,
    MarkupFractionQ12,
    SourcePunctuationFractionQ12,
    EqualAdjacentFractionQ12,
    LongRunCoverageQ12,
    LongestRunLog2Q12,
    SmallDeltaWidth1Q12,
    SmallDeltaWidth2Q12,
    SmallDeltaWidth4Q12,
    SmallDeltaWidth8Q12,
    LowByteConcentrationQ12,
    BestPeriodicityQ12,
    BestPeriodWidth,
    SampledLzCoverageQ12,
    SampledLzMeanLengthQ12,
    SampledLzMaximumLength,
    X86RelativeTargetDensityQ12,
    PackedFlags
};

constexpr std::size_t kBlockFeatureCount = 28U;
constexpr std::uint32_t kBlockFeatureQ12One = 1U << 12U;

enum BlockFeatureFlags : std::uint32_t {
    kBlockFeatureUtf8 = 1U << 0U,
    kBlockFeatureKnownMagic = 1U << 1U,
    kBlockFeatureCompressedMagic = 1U << 2U,
    kBlockFeatureSaturated = 1U << 3U,
    kBlockFeatureOutOfDistribution = 1U << 4U
};

// Encoder-only, integer-only router features. Every fractional and entropy
// feature uses Q12; F0, F22, F25, and F27 are discrete integer values.
struct BlockFeaturesV1 {
    std::uint32_t byte_count = 0;
    std::array<std::int32_t, kBlockFeatureCount> values{};

    std::int32_t operator[](BlockFeatureId id) const noexcept {
        return values[static_cast<std::size_t>(id)];
    }

    bool has_flag(BlockFeatureFlags flag) const noexcept {
        return (static_cast<std::uint32_t>(
                    (*this)[BlockFeatureId::PackedFlags]) & flag) != 0U;
    }

    BlockClass classify() const noexcept;
};

BlockFeaturesV1 extract_block_features(ByteView input) noexcept;

}  // namespace hz::r2
