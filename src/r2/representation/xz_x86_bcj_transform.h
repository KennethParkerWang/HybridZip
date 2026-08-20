#pragma once

#include "r2/representation/transform.h"

namespace hz::r2 {
class XzX86BcjTransform final : public IReversibleTransform {
public:
    const char* name() const noexcept override { return "xz-x86-bcj"; }
    bool applicable(ByteView input, const StructureFeatures& features) const override;
    TransformResult forward(ByteView input) const override;
    std::vector<std::uint8_t> inverse(ByteView transformed, ByteView side_information) const override;
};
}  // namespace hz::r2
