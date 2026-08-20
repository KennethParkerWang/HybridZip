#pragma once

#include "r2/representation/transform.h"

namespace hz::r2 {
class BloscShuffleTransform final {
public:
    TransformResult forward(ByteView input, std::uint8_t width) const;
    std::vector<std::uint8_t> inverse(ByteView input, std::uint8_t width) const;
};
}  // namespace hz::r2
