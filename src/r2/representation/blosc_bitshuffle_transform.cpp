#include "r2/representation/blosc_bitshuffle_transform.h"

#include <stdexcept>

namespace hz::r2 {
namespace {
bool valid(ByteView input, std::uint8_t width) noexcept {
    return !input.empty() && (width == 2 || width == 4 || width == 8) &&
           input.size() % width == 0 && (input.size() / width) % 8 == 0;
}
}
bool BloscBitshuffleTransform::applicable(ByteView input, std::uint8_t width) const noexcept { return valid(input, width); }
TransformResult BloscBitshuffleTransform::forward(ByteView input, std::uint8_t width) const {
    if (!valid(input, width)) throw std::invalid_argument("C-Blosc2 bitshuffle requires 2/4/8-byte elements in groups of eight");
    const std::size_t elements = input.size() / width;
    const std::size_t groups = elements / 8;
    TransformResult result{};
    result.bytes.resize(input.size());
    for (std::size_t group = 0; group < groups; ++group)
        for (std::size_t byte = 0; byte < width; ++byte)
            for (std::size_t bit = 0; bit < 8; ++bit)
                for (std::size_t lane = 0; lane < 8; ++lane)
                    result.bytes[(byte * 8 + bit) * groups + group] |= static_cast<std::uint8_t>(
                        ((input[(group * 8 + lane) * width + byte] >> bit) & 1U) << lane);
    return result;
}
std::vector<std::uint8_t> BloscBitshuffleTransform::inverse(ByteView input, std::uint8_t width) const {
    if (!valid(input, width)) throw std::invalid_argument("C-Blosc2 bitunshuffle requires 2/4/8-byte elements in groups of eight");
    const std::size_t elements = input.size() / width;
    const std::size_t groups = elements / 8;
    std::vector<std::uint8_t> result(input.size(), 0);
    for (std::size_t group = 0; group < groups; ++group)
        for (std::size_t byte = 0; byte < width; ++byte)
            for (std::size_t bit = 0; bit < 8; ++bit)
                for (std::size_t lane = 0; lane < 8; ++lane)
                    result[(group * 8 + lane) * width + byte] |= static_cast<std::uint8_t>(
                        ((input[(byte * 8 + bit) * groups + group] >> lane) & 1U) << bit);
    return result;
}
}  // namespace hz::r2
