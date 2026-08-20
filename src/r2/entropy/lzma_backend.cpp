#include "r2/entropy/lzma_backend.h"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <string>

#include "Alloc.h"
#include "LzmaDec.h"
#include "LzmaEnc.h"

namespace hz::r2 {
namespace {

constexpr std::array<std::uint8_t, 4> kPayloadMagic{'H', 'Z', 'L', '1'};
constexpr std::uint8_t kPayloadVersion = 1;
constexpr std::uint8_t kPayloadFlags = 0x07U;
constexpr std::uint8_t kPropertiesSize = LZMA_PROPS_SIZE;
constexpr std::size_t kUncompressedSizeOffset = 8;
constexpr std::size_t kCompressedSizeOffset = 12;
constexpr std::size_t kUncompressedCrcOffset = 16;
constexpr std::size_t kCompressedCrcOffset = 20;
constexpr std::size_t kPropertiesOffset = 24;
constexpr std::size_t kReservedOffset = 29;
constexpr std::size_t kEncoderSlack = 128;
constexpr std::size_t kMaximumEncoderSlack = 1U << 20U;
constexpr std::uint32_t kMinimumDictionarySize = 1U << 12U;

static_assert(kPropertiesOffset + LZMA_PROPS_SIZE <=
                  LzmaBackend::kPayloadHeaderSize,
              "LZMA payload header is too small");

std::size_t checked_add(const std::size_t left,
                        const std::size_t right,
                        const char* message) {
    if (left > std::numeric_limits<std::size_t>::max() - right) {
        throw std::length_error(message);
    }
    return left + right;
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

void require_valid_view(const ByteView input, const char* operation) {
    if (!input.empty() && input.data() == nullptr) {
        throw std::invalid_argument(std::string(operation) +
                                    " received a null byte view");
    }
}

[[noreturn]] void throw_lzma_error(const char* operation,
                                   const SRes result) {
    if (result == SZ_ERROR_MEM) {
        throw std::bad_alloc();
    }
    throw std::runtime_error(std::string(operation) +
                             " (7-Zip status " +
                             std::to_string(result) + ")");
}

void validate_properties(const std::uint8_t* properties) {
    CLzmaProps decoded{};
    const SRes result =
        LzmaProps_Decode(&decoded, properties, LZMA_PROPS_SIZE);
    if (result != SZ_OK) {
        throw std::runtime_error("Invalid LZMA properties");
    }

    const std::uint32_t dictionary_size =
        static_cast<std::uint32_t>(properties[1]) |
        (static_cast<std::uint32_t>(properties[2]) << 8U) |
        (static_cast<std::uint32_t>(properties[3]) << 16U) |
        (static_cast<std::uint32_t>(properties[4]) << 24U);
    if (dictionary_size < kMinimumDictionarySize ||
        dictionary_size > LzmaBackend::kMaximumDictionarySize ||
        decoded.dicSize != dictionary_size) {
        throw std::runtime_error("Unsupported LZMA dictionary size");
    }
}

std::size_t initial_payload_size(const std::size_t input_size) {
    const std::size_t expansion =
        input_size / 3U + (input_size % 3U != 0 ? 1U : 0U);
    std::size_t bound = checked_add(input_size, expansion,
                                    "LZMA initial bound overflow");
    bound = checked_add(bound, kEncoderSlack,
                        "LZMA initial bound overflow");
    return checked_add(bound, LzmaBackend::kPayloadHeaderSize,
                       "LZMA initial bound overflow");
}

}  // namespace

LzmaBackend::LzmaBackend(const int compression_level,
                         const std::uint32_t dictionary_size,
                         const std::size_t maximum_output_size)
    : compression_level_(compression_level),
      dictionary_size_(dictionary_size),
      maximum_output_size_(maximum_output_size) {
    if (compression_level < 0 || compression_level > 9) {
        throw std::invalid_argument("LZMA compression level must be 0 through 9");
    }
    if (dictionary_size != 0 &&
        (dictionary_size < kMinimumDictionarySize ||
         dictionary_size > kMaximumDictionarySize)) {
        throw std::invalid_argument("LZMA dictionary size is outside limits");
    }
    if (maximum_output_size >
        std::numeric_limits<std::uint32_t>::max()) {
        throw std::invalid_argument("LZMA output limit exceeds payload format");
    }
}

std::vector<std::uint8_t> LzmaBackend::encode(const ByteView input) const {
    require_valid_view(input, "LZMA encoder");
    if (input.size() > maximum_output_size_ ||
        input.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("LZMA input exceeds configured block limit");
    }

    const std::size_t maximum_size = maximum_payload_size(input.size());
    std::size_t output_size = initial_payload_size(input.size());
    std::vector<std::uint8_t> output;
    std::array<std::uint8_t, LZMA_PROPS_SIZE> properties{};

    CLzmaEncProps encoder_properties{};
    LzmaEncProps_Init(&encoder_properties);
    encoder_properties.level = compression_level_;
    encoder_properties.dictSize = dictionary_size_;
    encoder_properties.reduceSize = static_cast<UInt64>(input.size());
    encoder_properties.writeEndMark = 1;
    encoder_properties.numThreads = 1;

    std::uint8_t empty_input = 0;
    const std::uint8_t* source =
        input.empty() ? &empty_input : input.data();
    SRes result = SZ_ERROR_OUTPUT_EOF;
    SizeT properties_size = 0;
    SizeT compressed_size = 0;
    while (result == SZ_ERROR_OUTPUT_EOF) {
        output.assign(output_size, 0);
        properties.fill(0);
        properties_size = properties.size();
        compressed_size = output.size() - kPayloadHeaderSize;
        result = LzmaEncode(
            output.data() + kPayloadHeaderSize, &compressed_size, source,
            input.size(), &encoder_properties, properties.data(),
            &properties_size, 1, nullptr, &g_Alloc, &g_Alloc);
        if (result == SZ_ERROR_OUTPUT_EOF) {
            if (output_size == maximum_size) {
                throw std::runtime_error(
                    "LZMA output exceeded the configured safety limit");
            }
            const std::size_t doubled =
                output_size > maximum_size / 2U
                    ? maximum_size
                    : output_size * 2U;
            output_size = std::min(maximum_size, doubled);
        }
    }
    if (result != SZ_OK) {
        throw_lzma_error("LZMA compression failed", result);
    }
    if (properties_size != LZMA_PROPS_SIZE ||
        compressed_size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("LZMA encoder returned invalid sizes");
    }
    validate_properties(properties.data());

    std::copy(kPayloadMagic.begin(), kPayloadMagic.end(), output.begin());
    output[4] = kPayloadVersion;
    output[5] = kPayloadFlags;
    output[6] = kPropertiesSize;
    output[7] = static_cast<std::uint8_t>(kPayloadHeaderSize);
    write_u32_le(output, kUncompressedSizeOffset,
                 static_cast<std::uint32_t>(input.size()));
    write_u32_le(output, kCompressedSizeOffset,
                 static_cast<std::uint32_t>(compressed_size));
    write_u32_le(output, kUncompressedCrcOffset, crc32(input));
    const ByteView compressed(output.data() + kPayloadHeaderSize,
                              compressed_size);
    write_u32_le(output, kCompressedCrcOffset, crc32(compressed));
    std::copy(properties.begin(), properties.end(),
              output.begin() + kPropertiesOffset);
    output.resize(kPayloadHeaderSize + compressed_size);
    return output;
}

std::vector<std::uint8_t> LzmaBackend::decode(
    const ByteView payload,
    const std::size_t expected_size) const {
    require_valid_view(payload, "LZMA decoder");
    if (payload.size() < kPayloadHeaderSize) {
        throw std::runtime_error("Truncated LZMA payload header");
    }
    if (!std::equal(kPayloadMagic.begin(), kPayloadMagic.end(),
                    payload.data())) {
        throw std::runtime_error("Invalid LZMA payload magic");
    }
    if (payload[4] != kPayloadVersion || payload[5] != kPayloadFlags ||
        payload[6] != kPropertiesSize ||
        payload[7] != kPayloadHeaderSize) {
        throw std::runtime_error("Unsupported LZMA payload format");
    }
    for (std::size_t index = kReservedOffset;
         index < kPayloadHeaderSize; ++index) {
        if (payload[index] != 0) {
            throw std::runtime_error("Invalid LZMA payload reserved bytes");
        }
    }

    const std::size_t declared_size =
        read_u32_le(payload, kUncompressedSizeOffset);
    const std::size_t compressed_size =
        read_u32_le(payload, kCompressedSizeOffset);
    if (declared_size != expected_size) {
        throw std::runtime_error("LZMA output size does not match metadata");
    }
    if (declared_size > maximum_output_size_) {
        throw std::length_error("LZMA output exceeds configured block limit");
    }
    if (compressed_size != payload.size() - kPayloadHeaderSize) {
        throw std::runtime_error("LZMA compressed size does not match payload");
    }
    if (compressed_size < 5) {
        throw std::runtime_error("Truncated LZMA range stream");
    }

    const std::uint8_t* properties = payload.data() + kPropertiesOffset;
    validate_properties(properties);
    const ByteView compressed(payload.data() + kPayloadHeaderSize,
                              compressed_size);
    if (crc32(compressed) != read_u32_le(payload, kCompressedCrcOffset)) {
        throw std::runtime_error("LZMA compressed payload checksum mismatch");
    }

    std::vector<std::uint8_t> output(declared_size);
    std::uint8_t empty_output = 0;
    std::uint8_t* destination =
        output.empty() ? &empty_output : output.data();
    SizeT output_size = output.size();
    SizeT consumed_size = compressed_size;
    ELzmaStatus status = LZMA_STATUS_NOT_SPECIFIED;
    const SRes result = LzmaDecode(
        destination, &output_size, compressed.data(), &consumed_size,
        properties, LZMA_PROPS_SIZE, LZMA_FINISH_END, &status, &g_Alloc);
    if (result != SZ_OK) {
        throw_lzma_error("LZMA decompression failed", result);
    }
    if (output_size != declared_size || consumed_size != compressed_size ||
        status != LZMA_STATUS_FINISHED_WITH_MARK) {
        throw std::runtime_error("LZMA stream did not finish exactly");
    }
    if (crc32(ByteView(output)) !=
        read_u32_le(payload, kUncompressedCrcOffset)) {
        throw std::runtime_error("LZMA output checksum mismatch");
    }
    return output;
}

std::size_t LzmaBackend::maximum_payload_size(
    const std::size_t input_size) {
    if (input_size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("LZMA input exceeds payload format");
    }
    std::size_t bound = checked_add(input_size, input_size,
                                    "LZMA payload bound overflow");
    bound = checked_add(bound, kMaximumEncoderSlack,
                        "LZMA payload bound overflow");
    return checked_add(bound, kPayloadHeaderSize,
                       "LZMA payload bound overflow");
}

}  // namespace hz::r2
