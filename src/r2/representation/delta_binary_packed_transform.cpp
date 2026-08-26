#include "r2/representation/delta_binary_packed_transform.h"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hz::r2 {
namespace {

constexpr std::uint32_t kMiniBlocksPerBlock = 4U;

bool valid_width(const std::uint8_t width) noexcept {
    return width == 4U || width == 8U;
}

std::uint32_t values_per_block(const std::uint8_t width) noexcept {
    return width == 4U ? 128U : 256U;
}

std::uint64_t mask_for(const std::uint8_t width) noexcept {
    return width == 8U ? std::numeric_limits<std::uint64_t>::max()
                       : 0xFFFFFFFFULL;
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
        output[offset + byte] =
            static_cast<std::uint8_t>(value >> (8U * byte));
    }
}

void append_uleb(std::vector<std::uint8_t>& output, std::uint64_t value) {
    do {
        std::uint8_t byte = static_cast<std::uint8_t>(value & 0x7FU);
        value >>= 7U;
        if (value != 0) {
            byte = static_cast<std::uint8_t>(byte | 0x80U);
        }
        output.push_back(byte);
    } while (value != 0);
}

std::uint64_t read_uleb(ByteView input, std::size_t& offset) {
    std::uint64_t value = 0;
    for (unsigned byte_index = 0; byte_index < 10U; ++byte_index) {
        if (offset >= input.size()) {
            throw std::runtime_error("DeltaBinaryPacked header is truncated");
        }
        const std::uint8_t byte = input[offset++];
        if (byte_index == 9U && (byte & 0x7FU) > 1U) {
            throw std::runtime_error("DeltaBinaryPacked ULEB128 overflows");
        }
        value |= static_cast<std::uint64_t>(byte & 0x7FU)
            << (7U * byte_index);
        if ((byte & 0x80U) == 0) {
            return value;
        }
    }
    throw std::runtime_error("DeltaBinaryPacked ULEB128 is too long");
}

std::uint64_t zigzag_encode(const std::uint64_t raw,
                            const std::uint8_t width) noexcept {
    const unsigned sign_bit = static_cast<unsigned>(width) * 8U - 1U;
    const std::uint64_t sign = raw >> sign_bit;
    const std::uint64_t encoded = (raw << 1U) ^ (0U - sign);
    return encoded & (width == 8U ? std::numeric_limits<std::uint64_t>::max()
                                  : 0xFFFFFFFFULL);
}

std::uint64_t zigzag_decode(const std::uint64_t encoded,
                            const std::uint8_t width) noexcept {
    const std::uint64_t raw = (encoded >> 1U) ^ (0U - (encoded & 1U));
    return raw & mask_for(width);
}

unsigned required_bits(std::uint64_t value, const std::uint8_t width) noexcept {
    unsigned bits = 0;
    const unsigned limit = static_cast<unsigned>(width) * 8U;
    while (value != 0 && bits < limit) {
        value >>= 1U;
        ++bits;
    }
    return bits;
}

class BitWriter final {
public:
    explicit BitWriter(std::vector<std::uint8_t>& output)
        : output_(output), bit_offset_(output.size() * 8U) {}

    void put(std::uint64_t value, unsigned bit_count) {
        for (unsigned bit = 0; bit < bit_count; ++bit) {
            const std::size_t byte = bit_offset_ / 8U;
            if (byte >= output_.size()) {
                output_.push_back(0);
            }
            if ((value >> bit) & 1U) {
                output_[byte] = static_cast<std::uint8_t>(
                    output_[byte] | (1U << (bit_offset_ % 8U)));
            }
            ++bit_offset_;
        }
    }

private:
    std::vector<std::uint8_t>& output_;
    std::size_t bit_offset_ = 0;
};

class BitReader final {
public:
    explicit BitReader(const ByteView input) : input_(input) {}

    std::uint64_t get(unsigned bit_count) {
        if (bit_count > 64U || bit_offset_ > input_.size() * 8U ||
            bit_count > input_.size() * 8U - bit_offset_) {
            throw std::runtime_error("DeltaBinaryPacked bit stream is truncated");
        }
        std::uint64_t value = 0;
        for (unsigned bit = 0; bit < bit_count; ++bit) {
            if ((input_[bit_offset_ / 8U] >> (bit_offset_ % 8U)) & 1U) {
                value |= std::uint64_t{1} << bit;
            }
            ++bit_offset_;
        }
        return value;
    }

    std::size_t bytes_consumed() const noexcept {
        return (bit_offset_ + 7U) / 8U;
    }

private:
    ByteView input_;
    std::size_t bit_offset_ = 0;
};

}  // namespace

bool DeltaBinaryPackedTransform::applicable(
    const ByteView input, const std::uint8_t element_width) const noexcept {
    return valid_width(element_width) && input.size() >= element_width * 2U &&
        input.size() % element_width == 0U;
}

TransformResult DeltaBinaryPackedTransform::forward(
    const ByteView input, const std::uint8_t element_width) const {
    if (!applicable(input, element_width)) {
        throw std::runtime_error(
            "DeltaBinaryPacked requires at least two 32/64-bit words");
    }

    const std::uint64_t mask = mask_for(element_width);
    const std::size_t count = input.size() / element_width;
    const std::uint32_t block_values = values_per_block(element_width);
    const std::uint32_t mini_values = block_values / kMiniBlocksPerBlock;
    std::vector<std::uint8_t> output;
    output.reserve(maximum_transformed_size(input.size(), element_width));
    append_uleb(output, block_values);
    append_uleb(output, kMiniBlocksPerBlock);
    append_uleb(output, count);
    append_uleb(output, zigzag_encode(read_word(input, 0, element_width),
                                      element_width));

    std::vector<std::uint64_t> deltas;
    deltas.reserve(block_values);
    std::uint64_t previous = read_word(input, 0, element_width);
    for (std::size_t index = 1; index < count; ++index) {
        const std::uint64_t current = read_word(input, index * element_width,
                                                element_width);
        deltas.push_back((current - previous) & mask);
        previous = current;
        if (deltas.size() == block_values || index + 1U == count) {
            std::uint64_t min_delta = deltas.front();
            auto signed_value = [element_width](const std::uint64_t value) {
                return element_width == 4U
                    ? static_cast<std::int64_t>(static_cast<std::int32_t>(value))
                    : static_cast<std::int64_t>(value);
            };
            for (const std::uint64_t delta : deltas) {
                if (signed_value(delta) < signed_value(min_delta)) {
                    min_delta = delta;
                }
            }
            append_uleb(output, zigzag_encode(min_delta, element_width));
            const std::size_t block_start = output.size();
            output.resize(output.size() + kMiniBlocksPerBlock, 0);
            BitWriter writer(output);
            for (std::uint32_t mini = 0; mini < kMiniBlocksPerBlock; ++mini) {
                const std::size_t begin = static_cast<std::size_t>(mini) * mini_values;
                const std::size_t actual =
                    begin < deltas.size()
                        ? std::min<std::size_t>(mini_values, deltas.size() - begin)
                        : 0U;
                std::uint64_t maximum = 0;
                for (std::size_t item = 0; item < actual; ++item) {
                    maximum = std::max(maximum,
                        (deltas[begin + item] - min_delta) & mask);
                }
                const unsigned bit_width = required_bits(maximum, element_width);
                output[block_start + mini] = static_cast<std::uint8_t>(bit_width);
                for (std::size_t item = 0; item < mini_values; ++item) {
                    const std::uint64_t residual = item < actual
                        ? (deltas[begin + item] - min_delta) & mask
                        : 0U;
                    writer.put(residual, bit_width);
                }
            }
            deltas.clear();
        }
    }
    return TransformResult{std::move(output), {}};
}

std::vector<std::uint8_t> DeltaBinaryPackedTransform::inverse(
    const ByteView transformed, const std::uint8_t element_width,
    const std::size_t expected_size) const {
    if (!valid_width(element_width) || expected_size < element_width * 2U ||
        expected_size % element_width != 0U) {
        throw std::runtime_error("DeltaBinaryPacked output size is invalid");
    }
    std::size_t offset = 0;
    const std::uint64_t block_values = read_uleb(transformed, offset);
    const std::uint64_t miniblocks = read_uleb(transformed, offset);
    const std::uint64_t total_values = read_uleb(transformed, offset);
    const std::uint64_t first = read_uleb(transformed, offset);
    if (block_values != values_per_block(element_width) || miniblocks != 4U ||
        total_values != expected_size / element_width || total_values < 2U) {
        throw std::runtime_error("DeltaBinaryPacked header contract is invalid");
    }
    std::vector<std::uint8_t> output(expected_size);
    const std::uint64_t mask = mask_for(element_width);
    std::uint64_t previous = zigzag_decode(first, element_width);
    write_word(output, 0, previous, element_width);
    const std::uint32_t mini_values =
        values_per_block(element_width) / kMiniBlocksPerBlock;
    std::size_t remaining = static_cast<std::size_t>(total_values - 1U);
    while (remaining != 0) {
        const std::uint64_t min_delta =
            zigzag_decode(read_uleb(transformed, offset), element_width);
        if (offset > transformed.size() ||
            kMiniBlocksPerBlock > transformed.size() - offset) {
            throw std::runtime_error("DeltaBinaryPacked bit widths are truncated");
        }
        std::array<std::uint8_t, kMiniBlocksPerBlock> widths{};
        for (std::uint32_t mini = 0; mini < kMiniBlocksPerBlock; ++mini) {
            widths[mini] = transformed[offset++];
            if (widths[mini] > element_width * 8U) {
                throw std::runtime_error("DeltaBinaryPacked bit width is invalid");
            }
        }
        BitReader reader(ByteView(transformed.data() + offset,
                                  transformed.size() - offset));
        const std::size_t block_count =
            std::min<std::size_t>(remaining, values_per_block(element_width));
        std::size_t produced = 0;
        for (std::uint32_t mini = 0; mini < kMiniBlocksPerBlock; ++mini) {
            const std::size_t actual =
                std::min<std::size_t>(mini_values, block_count - produced);
            for (std::size_t item = 0; item < mini_values; ++item) {
                const std::uint64_t residual = reader.get(widths[mini]);
                if (item < actual) {
                    const std::uint64_t delta = (min_delta + residual) & mask;
                    previous = (previous + delta) & mask;
                    write_word(output,
                               (total_values - remaining) * element_width,
                               previous, element_width);
                    remaining--;
                }
            }
            produced += actual;
        }
        const std::size_t consumed = reader.bytes_consumed();
        if (consumed > transformed.size() - offset) {
            throw std::runtime_error("DeltaBinaryPacked bit stream is truncated");
        }
        offset += consumed;
    }
    if (offset != transformed.size()) {
        throw std::runtime_error(
            "DeltaBinaryPacked transformed stream has trailing bytes");
    }
    return output;
}

std::size_t DeltaBinaryPackedTransform::maximum_transformed_size(
    const std::size_t input_size, const std::uint8_t element_width) {
    if (!valid_width(element_width)) {
        throw std::invalid_argument("DeltaBinaryPacked element width is invalid");
    }
    const std::size_t maximum = std::numeric_limits<std::size_t>::max();
    const std::size_t overhead = input_size / 32U + 4096U;
    if (overhead > maximum - input_size) {
        throw std::overflow_error("DeltaBinaryPacked transformed size overflows");
    }
    return input_size + overhead;
}

}  // namespace hz::r2
