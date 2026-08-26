#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/core/byte_view.h"
#include "r2/representation/transform.h"

namespace hz::r2 {

// Frames cmix's WRT word dictionary as a self-contained, build-embedded
// reversible transform. zstd is applied by the block planner afterward.
class CmixWordDictionaryTransform final {
public:
    bool applicable(ByteView input) const noexcept;
    TransformResult forward(ByteView input) const;
    std::vector<std::uint8_t> inverse(ByteView transformed,
                                      std::size_t expected_size) const;

    static std::size_t maximum_transformed_size(std::size_t input_size);
};

}  // namespace hz::r2
