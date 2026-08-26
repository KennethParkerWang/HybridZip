#pragma once

#include <cstdint>
#include <vector>

#include "r2/representation/transform.h"

namespace hz::r2 {

// A fixed-record column view based on Parquet BYTE_STREAM_SPLIT semantics.
class RecordTransposeTransform final {
public:
    bool applicable(ByteView input, std::uint8_t record_width) const noexcept;
    TransformResult forward(ByteView input, std::uint8_t record_width) const;
    std::vector<std::uint8_t> inverse(ByteView input,
                                      std::uint8_t record_width) const;
};

}  // namespace hz::r2
