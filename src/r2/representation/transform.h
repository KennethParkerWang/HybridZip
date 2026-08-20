#pragma once

#include <cstdint>
#include <vector>

#include "r2/core/byte_view.h"

namespace hz::r2 {

struct StructureFeatures {
    double entropy_bits = 0.0;
    double printable_fraction = 0.0;
    double zero_fraction = 0.0;
    std::uint32_t longest_match = 0;
};

struct TransformResult {
    std::vector<std::uint8_t> bytes;
    std::vector<std::uint8_t> side_information;
};

class IReversibleTransform {
public:
    virtual ~IReversibleTransform() = default;

    virtual const char* name() const noexcept = 0;
    virtual bool applicable(ByteView input,
                            const StructureFeatures& features) const = 0;
    virtual TransformResult forward(ByteView input) const = 0;
    virtual std::vector<std::uint8_t> inverse(
        ByteView transformed,
        ByteView side_information) const = 0;
};

}  // namespace hz::r2

