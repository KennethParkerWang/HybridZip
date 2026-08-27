#pragma once

#include <cstdint>

#include "r2/core/byte_view.h"

namespace hz::r2 {

enum class BlockClass : std::uint8_t {
    Text,
    X86,
    Numeric,
    Generic
};

// Integer-only byte statistics used by the encoder shortlist. Values ending
// in _per_mille are rounded down to a 0..1000 range.
struct BlockFeaturesV1 {
    std::uint32_t byte_count = 0;
    std::uint16_t printable_per_mille = 0;
    std::uint16_t whitespace_per_mille = 0;
    std::uint16_t markup_per_mille = 0;
    std::uint16_t zero_per_mille = 0;
    std::uint16_t equal_lag1_per_mille = 0;
    std::uint16_t equal_lag2_per_mille = 0;
    std::uint16_t equal_lag4_per_mille = 0;
    std::uint16_t equal_lag8_per_mille = 0;
    std::uint16_t x86_branch_per_mille = 0;
    std::uint16_t unique_bytes = 0;

    BlockClass classify() const noexcept;
};

BlockFeaturesV1 extract_block_features(ByteView input) noexcept;

}  // namespace hz::r2
