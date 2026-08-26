#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/core/byte_view.h"
#include "r2/representation/transform.h"

namespace hz::r2 {

// Wraps CharLS as an 8-bit, one-component raw-frame coding path.
class JpegLsTransform final {
public:
    bool applicable(ByteView input, std::uint32_t width) const noexcept;
    TransformResult forward(ByteView input, std::uint32_t width) const;
    std::vector<std::uint8_t> inverse(ByteView payload,
                                      ByteView side_information,
                                      std::size_t expected_size) const;

    static std::size_t maximum_payload_size(std::size_t input_size);
};

}  // namespace hz::r2
