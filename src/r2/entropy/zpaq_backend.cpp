#include "r2/entropy/zpaq_backend.h"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "libzpaq.h"

namespace libzpaq {

// libzpaq reports malformed streams and allocation failures through this
// callback. Throwing keeps the donor boundary inside the HZ02 exception API.
void error(const char* message) {
    throw std::runtime_error(std::string("ZPAQ: ") +
                             (message == nullptr ? "unknown error" : message));
}

}  // namespace libzpaq

namespace hz::r2 {
namespace {

constexpr std::array<std::uint8_t, 4> kPayloadMagic{'H', 'Z', 'Q', '1'};
constexpr std::uint8_t kPayloadVersion = 1;
constexpr std::uint8_t kPayloadFlags = 0;
constexpr std::size_t kMethodOffset = 6;
constexpr std::size_t kHeaderSizeOffset = 7;
constexpr std::size_t kUncompressedSizeOffset = 8;
constexpr std::size_t kCompressedSizeOffset = 12;
constexpr std::size_t kUncompressedCrcOffset = 16;
constexpr std::size_t kCompressedCrcOffset = 20;
constexpr std::size_t kReservedOffset = 24;
constexpr std::size_t kExpansionFactor = 8;
constexpr std::size_t kExpansionSlack = 1024U * 1024U;

std::size_t checked_add(const std::size_t left,
                        const std::size_t right,
                        const char* message) {
    if (left > std::numeric_limits<std::size_t>::max() - right) {
        throw std::length_error(message);
    }
    return left + right;
}

std::size_t checked_multiply(const std::size_t left,
                             const std::size_t right,
                             const char* message) {
    if (left != 0 && right > std::numeric_limits<std::size_t>::max() / left) {
        throw std::length_error(message);
    }
    return left * right;
}

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

std::vector<std::uint8_t> copy_buffer(const libzpaq::StringBuffer& buffer) {
    const auto* data = reinterpret_cast<const std::uint8_t*>(buffer.c_str());
    return std::vector<std::uint8_t>(data, data + buffer.size());
}

void validate_method(const std::uint8_t method_level) {
    if (method_level > 5U) {
        throw std::invalid_argument("ZPAQ method level must be 0 through 5");
    }
}

}  // namespace

ZpaqBackend::ZpaqBackend(const std::uint8_t method_level,
                         const std::size_t maximum_output_size)
    : method_level_(method_level), maximum_output_size_(maximum_output_size) {
    validate_method(method_level_);
    if (maximum_output_size_ > std::numeric_limits<std::uint32_t>::max()) {
        throw std::invalid_argument("ZPAQ output limit exceeds payload format");
    }
}

std::vector<std::uint8_t> ZpaqBackend::encode(const ByteView input) const {
    require_valid_view(input, "ZPAQ encoder");
    if (input.empty()) {
        throw std::invalid_argument("ZPAQ does not encode an empty block");
    }
    if (input.size() > maximum_output_size_ ||
        input.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("ZPAQ input exceeds configured block limit");
    }

    libzpaq::StringBuffer source(input.size());
    source.write(reinterpret_cast<const char*>(input.data()),
                 static_cast<int>(input.size()));
    libzpaq::StringBuffer compressed(maximum_payload_size(input.size()));
    compressed.setLimit(maximum_payload_size(input.size()));
    const char method[] = {static_cast<char>('0' + method_level_), '\0'};
    libzpaq::compressBlock(&source, &compressed, method, nullptr, nullptr,
                           false);
    if (compressed.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("ZPAQ payload exceeds payload format");
    }

    const std::size_t stream_size = compressed.size();
    std::vector<std::uint8_t> output(
        checked_add(kPayloadHeaderSize, stream_size,
                    "ZPAQ payload size overflow"),
        0);
    std::copy(kPayloadMagic.begin(), kPayloadMagic.end(), output.begin());
    output[4] = kPayloadVersion;
    output[5] = kPayloadFlags;
    output[kMethodOffset] = method_level_;
    output[kHeaderSizeOffset] = static_cast<std::uint8_t>(kPayloadHeaderSize);
    write_u32_le(output, kUncompressedSizeOffset,
                 static_cast<std::uint32_t>(input.size()));
    write_u32_le(output, kCompressedSizeOffset,
                 static_cast<std::uint32_t>(stream_size));
    write_u32_le(output, kUncompressedCrcOffset, crc32(input));
    const ByteView stream(reinterpret_cast<const std::uint8_t*>(
                              compressed.c_str()),
                          stream_size);
    write_u32_le(output, kCompressedCrcOffset, crc32(stream));
    std::copy(stream.data(), stream.data() + stream.size(),
              output.begin() + kPayloadHeaderSize);
    return output;
}

std::vector<std::uint8_t> ZpaqBackend::decode(
    const ByteView payload, const std::size_t expected_size) const {
    require_valid_view(payload, "ZPAQ decoder");
    if (payload.size() < kPayloadHeaderSize + 16U) {
        throw std::runtime_error("Truncated ZPAQ payload");
    }
    if (!std::equal(kPayloadMagic.begin(), kPayloadMagic.end(),
                    payload.data())) {
        throw std::runtime_error("Invalid ZPAQ payload magic");
    }
    if (payload[4] != kPayloadVersion || payload[5] != kPayloadFlags ||
        payload[7] != kPayloadHeaderSize) {
        throw std::runtime_error("Unsupported ZPAQ payload format");
    }
    validate_method(payload[kMethodOffset]);
    for (std::size_t index = kReservedOffset; index < kPayloadHeaderSize;
         ++index) {
        if (payload[index] != 0) {
            throw std::runtime_error("Invalid ZPAQ payload reserved bytes");
        }
    }

    const std::size_t declared_size =
        read_u32_le(payload, kUncompressedSizeOffset);
    const std::size_t stream_size =
        read_u32_le(payload, kCompressedSizeOffset);
    if (declared_size != expected_size || declared_size == 0 ||
        declared_size > maximum_output_size_) {
        throw std::runtime_error("ZPAQ output size does not match metadata");
    }
    if (stream_size != payload.size() - kPayloadHeaderSize || stream_size == 0 ||
        stream_size > maximum_payload_size(declared_size)) {
        throw std::runtime_error("ZPAQ stream size does not match payload");
    }
    const ByteView stream(payload.data() + kPayloadHeaderSize, stream_size);
    if (crc32(stream) != read_u32_le(payload, kCompressedCrcOffset)) {
        throw std::runtime_error("ZPAQ compressed payload checksum mismatch");
    }

    libzpaq::StringBuffer compressed(stream_size);
    compressed.write(reinterpret_cast<const char*>(stream.data()),
                     static_cast<int>(stream.size()));
    libzpaq::StringBuffer restored(declared_size);
    restored.setLimit(declared_size);
    libzpaq::Decompresser decompressor;
    decompressor.setInput(&compressed);
    if (!decompressor.findBlock()) {
        throw std::runtime_error("ZPAQ stream contains no block");
    }
    if (!decompressor.findFilename()) {
        throw std::runtime_error("ZPAQ block contains no segment");
    }
    decompressor.readComment();
    decompressor.setOutput(&restored);
    while (decompressor.decompress(64U * 1024U)) {
    }
    std::array<char, 21> donor_checksum{};
    decompressor.readSegmentEnd(donor_checksum.data());
    if (donor_checksum[0] != 0) {
        throw std::runtime_error("ZPAQ stream has an unexpected donor checksum");
    }
    if (decompressor.findFilename()) {
        throw std::runtime_error("ZPAQ block contains multiple segments");
    }
    if (compressed.remaining() != 0 || restored.size() != declared_size) {
        throw std::runtime_error("ZPAQ stream did not finish exactly");
    }
    std::vector<std::uint8_t> output = copy_buffer(restored);
    if (crc32(ByteView(output)) != read_u32_le(payload, kUncompressedCrcOffset)) {
        throw std::runtime_error("ZPAQ output checksum mismatch");
    }
    return output;
}

std::size_t ZpaqBackend::maximum_payload_size(const std::size_t input_size) {
    return checked_add(
        checked_multiply(input_size, kExpansionFactor,
                         "ZPAQ payload bound overflow"),
        checked_add(kExpansionSlack, kPayloadHeaderSize,
                    "ZPAQ payload bound overflow"),
        "ZPAQ payload bound overflow");
}

}  // namespace hz::r2
