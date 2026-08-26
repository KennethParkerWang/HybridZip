#pragma once

#include <cstddef>
#include <cstdint>

#include "r2/representation/transform.h"

namespace hz::r2 {

// A reversible second-order numeric view.  Values are little-endian unsigned
// words; the first value and first delta are retained, then subsequent values
// store delta-of-delta modulo the selected word width.  This follows the
// decoder-visible numeric-view principle used by Parquet DELTA_BINARY_PACKED:
// the width and byte order are part of the framing, while the transformed
// stream remains independently compressible by the selected backend.
class DeltaOfDeltaTransform final {
public:
    bool applicable(ByteView input, std::uint8_t element_width) const noexcept;

    TransformResult forward(ByteView input,
                            std::uint8_t element_width) const;

    std::vector<std::uint8_t> inverse(ByteView transformed,
                                      std::uint8_t element_width) const;
};

}  // namespace hz::r2
