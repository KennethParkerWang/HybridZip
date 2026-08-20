#pragma once

#include <optional>
#include <vector>

#include "r2/core/byte_view.h"

namespace hz::r2 {
class KanziRltTransform final {
public:
    std::optional<std::vector<std::uint8_t>> forward_if_smaller(ByteView input) const;
    std::vector<std::uint8_t> inverse(ByteView input, std::size_t output_size) const;
};
}  // namespace hz::r2
