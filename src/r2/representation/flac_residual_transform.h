#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/core/byte_view.h"
#include "r2/representation/transform.h"

namespace hz::r2 {

// Frames raw little-endian signed 16-bit PCM around libFLAC fixed/LPC and
// Rice primitives. It deliberately does not claim that arbitrary bytes are a
// standard FLAC file; HZ02 carries the PCM interpretation in side information.
class FlacResidualTransform final {
public:
    bool applicable(ByteView input) const noexcept;
    TransformResult forward(ByteView input) const;
    std::vector<std::uint8_t> inverse(ByteView payload,
                                      ByteView side_information,
                                      std::size_t expected_size) const;

    static std::size_t maximum_payload_size(std::size_t input_size);
};

}  // namespace hz::r2
