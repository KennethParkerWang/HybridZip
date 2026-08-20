#include "r2/representation/xz_x86_bcj_transform.h"

#include <limits>
#include <stdexcept>

#include "bcj_x86.h"

namespace hz::r2 {
namespace {
std::vector<std::uint8_t> apply(ByteView input, bool inverse) {
    if (input.empty()) throw std::invalid_argument("XZ x86 BCJ requires a nonempty block");
    std::vector<std::uint8_t> output = copy_bytes(input);
    const size_t consumed = inverse ? hz_xz_bcj_x86_decode(0, output.data(), output.size())
                                    : hz_xz_bcj_x86_encode(0, output.data(), output.size());
    if (consumed > output.size()) throw std::runtime_error("XZ x86 BCJ reported invalid consumption");
    return output;
}
}

bool XzX86BcjTransform::applicable(ByteView input, const StructureFeatures&) const {
    return !input.empty() && input.size() <= static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max());
}
TransformResult XzX86BcjTransform::forward(ByteView input) const { return {apply(input, false), {}}; }
std::vector<std::uint8_t> XzX86BcjTransform::inverse(ByteView input, ByteView side) const {
    if (!side.empty()) throw std::runtime_error("XZ x86 BCJ has no side information");
    return apply(input, true);
}
}  // namespace hz::r2
