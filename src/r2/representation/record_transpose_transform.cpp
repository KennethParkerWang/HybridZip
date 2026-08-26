#include "r2/representation/record_transpose_transform.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace hz::r2 {
namespace {

bool valid_width(const std::uint8_t width) noexcept {
    return width == 16U || width == 32U;
}

void require_applicable(const ByteView input, const std::uint8_t width) {
    if (input.empty() || !valid_width(width) || input.size() % width != 0U) {
        throw std::invalid_argument("record transpose requires whole 16/32-byte records");
    }
}

}  // namespace

bool RecordTransposeTransform::applicable(const ByteView input,
                                          const std::uint8_t width) const noexcept {
    return !input.empty() && valid_width(width) && input.size() % width == 0U;
}

TransformResult RecordTransposeTransform::forward(const ByteView input,
                                                   const std::uint8_t width) const {
    require_applicable(input, width);
    const std::size_t rows = input.size() / width;
    TransformResult result{};
    result.bytes.resize(input.size());
    for (std::size_t column = 0; column < width; ++column) {
        for (std::size_t row = 0; row < rows; ++row) {
            result.bytes[column * rows + row] = input[row * width + column];
        }
    }
    return result;
}

std::vector<std::uint8_t> RecordTransposeTransform::inverse(
    const ByteView input, const std::uint8_t width) const {
    require_applicable(input, width);
    const std::size_t rows = input.size() / width;
    std::vector<std::uint8_t> result(input.size());
    for (std::size_t column = 0; column < width; ++column) {
        for (std::size_t row = 0; row < rows; ++row) {
            result[row * width + column] = input[column * rows + row];
        }
    }
    return result;
}

}  // namespace hz::r2
