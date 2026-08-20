#pragma once

#include "r2/representation/transform.h"

namespace hz::r2 {

class KanziMtfTransform final : public IReversibleTransform {
public:
    const char* name() const noexcept override { return "kanzi-sbrt-mtf"; }
    bool applicable(ByteView input, const StructureFeatures&) const override;
    TransformResult forward(ByteView input) const override;
    std::vector<std::uint8_t> inverse(ByteView transformed,
                                      ByteView side_information) const override;
};

}  // namespace hz::r2
