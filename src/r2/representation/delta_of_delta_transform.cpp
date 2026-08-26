#include "r2/representation/delta_of_delta_transform.h"

#include <limits>
#include <stdexcept>

namespace hz::r2 {
namespace {

bool valid_width(const std::uint8_t width) noexcept {
    return width == 4U || width == 8U;
}

std::uint64_t read_word(const ByteView input, const std::size_t offset,
                        const std::uint8_t width) noexcept {
    std::uint64_t value = 0;
    for (std::uint8_t byte = 0; byte < width; ++byte) {
        value |= static_cast<std::uint64_t>(input[offset + byte])
            << (8U * byte);
    }
    return value;
}

void write_word(std::vector<std::uint8_t>& output, const std::size_t offset,
                const std::uint64_t value, const std::uint8_t width) noexcept {
    for (std::uint8_t byte = 0; byte < width; ++byte) {
        output[offset + byte] = static_cast<std::uint8_t>(value >> (8U * byte));
    }
}

}  // namespace

bool DeltaOfDeltaTransform::applicable(
    const ByteView input, const std::uint8_t element_width) const noexcept {
    return valid_width(element_width) && input.size() >= element_width * 3U &&
        input.size() % element_width == 0U;
}

TransformResult DeltaOfDeltaTransform::forward(
    const ByteView input, const std::uint8_t element_width) const {
    if (!applicable(input, element_width)) {
        throw std::runtime_error(
            "Delta-of-delta requires at least three 32/64-bit words");
    }

    TransformResult result;
    result.bytes.resize(input.size());
    const std::size_t count = input.size() / element_width;
    const std::uint64_t mask = element_width == 8U
        ? std::numeric_limits<std::uint64_t>::max()
        : 0xFFFFFFFFULL;
    std::uint64_t previous_value = read_word(input, 0U, element_width);
    write_word(result.bytes, 0U, previous_value, element_width);
    std::uint64_t previous_delta = 0;
    for (std::size_t index = 1; index < count; ++index) {
        const std::size_t offset = index * element_width;
        const std::uint64_t value = read_word(input, offset, element_width);
        const std::uint64_t delta = (value - previous_value) & mask;
        const std::uint64_t encoded = index == 1U
            ? delta
            : (delta - previous_delta) & mask;
        write_word(result.bytes, offset, encoded, element_width);
        previous_value = value;
        previous_delta = delta;
    }
    return result;
}

std::vector<std::uint8_t> DeltaOfDeltaTransform::inverse(
    const ByteView transformed, const std::uint8_t element_width) const {
    if (!applicable(transformed, element_width)) {
        throw std::runtime_error(
            "Delta-of-delta stream requires at least three 32/64-bit words");
    }

    std::vector<std::uint8_t> output(transformed.size());
    const std::size_t count = transformed.size() / element_width;
    const std::uint64_t mask = element_width == 8U
        ? std::numeric_limits<std::uint64_t>::max()
        : 0xFFFFFFFFULL;
    std::uint64_t previous_value = read_word(transformed, 0U, element_width);
    write_word(output, 0U, previous_value, element_width);
    std::uint64_t previous_delta = 0;
    for (std::size_t index = 1; index < count; ++index) {
        const std::size_t offset = index * element_width;
        const std::uint64_t encoded =
            read_word(transformed, offset, element_width);
        const std::uint64_t delta = index == 1U
            ? encoded
            : (previous_delta + encoded) & mask;
        const std::uint64_t value = (previous_value + delta) & mask;
        write_word(output, offset, value, element_width);
        previous_value = value;
        previous_delta = delta;
    }
    return output;
}

}  // namespace hz::r2
