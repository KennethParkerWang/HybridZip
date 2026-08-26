#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/core/byte_view.h"
#include "r2/representation/transform.h"

namespace hz::r2 {

// Wraps the complete donor codec as a self-contained text-mode Brotli stream.
class BrotliTextTransform final {
public:
    bool applicable(ByteView input) const noexcept;
    TransformResult forward(ByteView input) const;
    std::vector<std::uint8_t> inverse(ByteView payload,
                                      std::size_t expected_size) const;

    static std::size_t maximum_payload_size(std::size_t input_size);
};

}  // namespace hz::r2
