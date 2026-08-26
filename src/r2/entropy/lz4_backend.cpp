#include "r2/entropy/lz4_backend.h"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <string>

#include "lz4.h"
#include "lz4hc.h"

namespace hz::r2 {
namespace {

constexpr std::array<std::uint8_t, 4> kPayloadMagic{'H', 'Z', '4', '1'};
constexpr std::uint8_t kPayloadVersion = 1;
constexpr std::uint8_t kPayloadFlags = 0x03U;
constexpr std::size_t kUncompressedSizeOffset = 8;
constexpr std::size_t kCompressedSizeOffset = 12;
constexpr std::size_t kUncompressedCrcOffset = 16;
constexpr std::size_t kCompressedCrcOffset = 20;

void require_valid_view(const ByteView input, const char* operation) {
    if (!input.empty() && input.data() == nullptr) {
        throw std::invalid_argument(std::string(operation) +
                                    " received a null byte view");
    }
}

std::uint32_t crc32(const ByteView input) noexcept {
    std::uint32_t checksum = 0xFFFFFFFFU;
    for (std::size_t index = 0; index < input.size(); ++index) {
        checksum ^= input[index];
        for (unsigned bit = 0; bit < 8U; ++bit) {
            const std::uint32_t mask =
                0U - static_cast<std::uint32_t>(checksum & 1U);
            checksum = (checksum >> 1U) ^ (0xEDB88320U & mask);
        }
    }
    return ~checksum;
}

void write_u32_le(std::vector<std::uint8_t>& output,
                  const std::size_t offset,
                  const std::uint32_t value) noexcept {
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        output[offset + shift / 8U] =
            static_cast<std::uint8_t>(value >> shift);
    }
}

std::uint32_t read_u32_le(const ByteView input,
                          const std::size_t offset) noexcept {
    std::uint32_t value = 0;
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        value |= static_cast<std::uint32_t>(input[offset + shift / 8U])
                 << shift;
    }
    return value;
}

int checked_lz4_size(const std::size_t size, const char* label) {
    if (size > static_cast<std::size_t>(LZ4_MAX_INPUT_SIZE) ||
        size > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::length_error(std::string(label) +
                                " exceeds the LZ4 block limit");
    }
    return static_cast<int>(size);
}

}  // namespace

std::vector<std::uint8_t> Lz4Backend::encode(const ByteView input) const {
    require_valid_view(input, "LZ4 encoder");
    const int source_size = checked_lz4_size(input.size(), "LZ4 input");
    const int compressed_bound = LZ4_compressBound(source_size);
    if (compressed_bound <= 0) {
        throw std::runtime_error("LZ4 could not compute a compression bound");
    }

    std::vector<std::uint8_t> output(
        kPayloadHeaderSize + static_cast<std::size_t>(compressed_bound), 0);
    const char* source = reinterpret_cast<const char*>(input.data());
    char* destination = reinterpret_cast<char*>(
        output.data() + kPayloadHeaderSize);
    const int compressed_size = LZ4_compress_HC(
        source, destination, source_size, compressed_bound,
        static_cast<int>(kCompressionLevel));
    if (compressed_size <= 0) {
        throw std::runtime_error("LZ4 HC compression failed");
    }

    std::copy(kPayloadMagic.begin(), kPayloadMagic.end(), output.begin());
    output[4] = kPayloadVersion;
    output[5] = kCompressionLevel;
    output[6] = kPayloadFlags;
    output[7] = static_cast<std::uint8_t>(kPayloadHeaderSize);
    write_u32_le(output, kUncompressedSizeOffset,
                 static_cast<std::uint32_t>(input.size()));
    write_u32_le(output, kCompressedSizeOffset,
                 static_cast<std::uint32_t>(compressed_size));
    write_u32_le(output, kUncompressedCrcOffset, crc32(input));
    const ByteView compressed(output.data() + kPayloadHeaderSize,
                              static_cast<std::size_t>(compressed_size));
    write_u32_le(output, kCompressedCrcOffset, crc32(compressed));
    output.resize(kPayloadHeaderSize +
                  static_cast<std::size_t>(compressed_size));
    return output;
}

std::vector<std::uint8_t> Lz4Backend::decode(
    const ByteView payload,
    const std::size_t expected_size) const {
    require_valid_view(payload, "LZ4 decoder");
    if (payload.size() < kPayloadHeaderSize) {
        throw std::runtime_error("Truncated LZ4 payload header");
    }
    if (!std::equal(kPayloadMagic.begin(), kPayloadMagic.end(),
                    payload.data())) {
        throw std::runtime_error("Invalid LZ4 payload magic");
    }
    if (payload[4] != kPayloadVersion ||
        payload[5] != kCompressionLevel || payload[6] != kPayloadFlags ||
        payload[7] != kPayloadHeaderSize) {
        throw std::runtime_error("Unsupported LZ4 payload format");
    }

    const std::size_t declared_size =
        read_u32_le(payload, kUncompressedSizeOffset);
    const std::size_t compressed_size =
        read_u32_le(payload, kCompressedSizeOffset);
    if (declared_size != expected_size) {
        throw std::runtime_error("LZ4 output size does not match metadata");
    }
    const int destination_capacity =
        checked_lz4_size(declared_size, "LZ4 output");
    if (compressed_size == 0 ||
        compressed_size != payload.size() - kPayloadHeaderSize ||
        compressed_size > static_cast<std::size_t>(
            LZ4_compressBound(destination_capacity))) {
        throw std::runtime_error("Invalid LZ4 compressed size");
    }
    if (compressed_size >
        static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::length_error("LZ4 compressed input exceeds decoder limit");
    }

    const ByteView compressed(payload.data() + kPayloadHeaderSize,
                              compressed_size);
    if (crc32(compressed) != read_u32_le(payload, kCompressedCrcOffset)) {
        throw std::runtime_error("LZ4 compressed payload checksum mismatch");
    }

    std::vector<std::uint8_t> output(declared_size);
    const int decoded_size = LZ4_decompress_safe(
        reinterpret_cast<const char*>(compressed.data()),
        reinterpret_cast<char*>(output.data()),
        static_cast<int>(compressed_size), destination_capacity);
    if (decoded_size < 0 ||
        static_cast<std::size_t>(decoded_size) != declared_size) {
        throw std::runtime_error("LZ4 stream did not decode exactly");
    }
    if (crc32(ByteView(output)) !=
        read_u32_le(payload, kUncompressedCrcOffset)) {
        throw std::runtime_error("LZ4 output checksum mismatch");
    }
    return output;
}

std::size_t Lz4Backend::maximum_payload_size(
    const std::size_t input_size) {
    const int source_size = checked_lz4_size(input_size, "LZ4 input");
    const int compressed_bound = LZ4_compressBound(source_size);
    if (compressed_bound <= 0 ||
        static_cast<std::size_t>(compressed_bound) >
            std::numeric_limits<std::size_t>::max() - kPayloadHeaderSize) {
        throw std::length_error("LZ4 payload bound overflow");
    }
    return kPayloadHeaderSize + static_cast<std::size_t>(compressed_bound);
}

}  // namespace hz::r2
