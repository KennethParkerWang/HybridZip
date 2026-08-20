#include "r2/representation/blosc_shuffle_transform.h"

#include <stdexcept>

#include "shuffle_generic.h"

namespace hz::r2 {
namespace {
void validate(ByteView input, std::uint8_t width) {
    if (input.empty() || (width != 2 && width != 4 && width != 8))
        throw std::invalid_argument("C-Blosc2 shuffle requires a nonempty 2/4/8-byte view");
}
}
TransformResult BloscShuffleTransform::forward(ByteView input, std::uint8_t width) const {
    validate(input, width);
    TransformResult result{};
    result.bytes.resize(input.size());
    hz_blosc_shuffle_generic(width, input.size(), input.data(), result.bytes.data());
    return result;
}
std::vector<std::uint8_t> BloscShuffleTransform::inverse(ByteView input, std::uint8_t width) const {
    validate(input, width);
    std::vector<std::uint8_t> result(input.size());
    hz_blosc_unshuffle_generic(width, input.size(), input.data(), result.data());
    return result;
}
}  // namespace hz::r2
