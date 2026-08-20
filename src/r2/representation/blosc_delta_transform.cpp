#include "r2/representation/blosc_delta_transform.h"

#include <stdexcept>

namespace hz::r2 {
namespace {
bool valid(ByteView input, std::uint8_t width) noexcept {
    return !input.empty() && (width == 1 || width == 2 || width == 4 || width == 8) && input.size() % width == 0;
}
}
bool BloscDeltaTransform::applicable(ByteView input, std::uint8_t width) const noexcept { return valid(input, width); }
TransformResult BloscDeltaTransform::forward(ByteView input, std::uint8_t width) const {
    if (!valid(input, width)) throw std::invalid_argument("C-Blosc2 delta requires 1/2/4/8-byte elements");
    TransformResult result{};
    result.bytes.resize(input.size());
    for (std::size_t offset = 0; offset < width; ++offset) result.bytes[offset] = input[offset];
    for (std::size_t base = width; base < input.size(); base += width)
        for (std::size_t offset = 0; offset < width; ++offset)
            result.bytes[base + offset] = static_cast<std::uint8_t>(input[base + offset] ^ input[base - width + offset]);
    return result;
}
std::vector<std::uint8_t> BloscDeltaTransform::inverse(ByteView input, std::uint8_t width) const {
    if (!valid(input, width)) throw std::invalid_argument("C-Blosc2 delta inverse requires 1/2/4/8-byte elements");
    std::vector<std::uint8_t> result = copy_bytes(input);
    for (std::size_t base = width; base < result.size(); base += width)
        for (std::size_t offset = 0; offset < width; ++offset)
            result[base + offset] ^= result[base - width + offset];
    return result;
}
}  // namespace hz::r2
