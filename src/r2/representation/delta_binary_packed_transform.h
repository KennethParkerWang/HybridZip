#pragma once

#include <cstddef>
#include <cstdint>

#include "r2/representation/transform.h"

namespace hz::r2 {

// Apache Arrow/Parquet DELTA_BINARY_PACKED port for little-endian INT32 and
// INT64 byte streams.  The resulting stream is independently compressed by
// the selected entropy backend.
class DeltaBinaryPackedTransform final {
public:
    bool applicable(ByteView input, std::uint8_t element_width) const noexcept;

    TransformResult forward(ByteView input,
                            std::uint8_t element_width) const;

    std::vector<std::uint8_t> inverse(ByteView transformed,
                                      std::uint8_t element_width,
                                      std::size_t expected_size) const;

    static std::size_t maximum_transformed_size(
        std::size_t input_size, std::uint8_t element_width);
};

}  // namespace hz::r2
