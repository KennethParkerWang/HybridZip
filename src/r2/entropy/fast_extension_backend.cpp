#include "r2/entropy/fast_extension_backend.h"

#include <stdexcept>

#include "r2/entropy/zstd_backend.h"

namespace hz::r2 {
namespace {

constexpr std::uint8_t kExtensionVersion = 1;
constexpr std::uint8_t kZstdCodec = 0;

void append_uleb128(std::vector<std::uint8_t>& output,
                    std::uint32_t value) {
    do {
        std::uint8_t byte = static_cast<std::uint8_t>(value & 0x7FU);
        value >>= 7U;
        if (value != 0U) {
            byte |= 0x80U;
        }
        output.push_back(byte);
    } while (value != 0U);
}

std::uint32_t read_uleb128(const ByteView input, std::size_t& offset) {
    std::uint32_t value = 0;
    for (unsigned shift = 0; shift < 32U; shift += 7U) {
        if (offset >= input.size()) {
            throw std::runtime_error("HZ02 fast extension metadata is truncated");
        }
        const std::uint8_t byte = input[offset++];
        if (shift == 28U && (byte & 0xF0U) != 0U) {
            throw std::runtime_error("HZ02 fast extension size overflows");
        }
        value |= static_cast<std::uint32_t>(byte & 0x7FU) << shift;
        if ((byte & 0x80U) == 0U) {
            return value;
        }
    }
    throw std::runtime_error("HZ02 fast extension size is malformed");
}

FastExtensionTransform decode_transform(const std::uint8_t value) {
    switch (value) {
    case static_cast<std::uint8_t>(FastExtensionTransform::None):
        return FastExtensionTransform::None;
    case static_cast<std::uint8_t>(FastExtensionTransform::ByteShuffle):
        return FastExtensionTransform::ByteShuffle;
    case static_cast<std::uint8_t>(FastExtensionTransform::BitShuffle):
        return FastExtensionTransform::BitShuffle;
    case static_cast<std::uint8_t>(FastExtensionTransform::XorDelta):
        return FastExtensionTransform::XorDelta;
    case static_cast<std::uint8_t>(FastExtensionTransform::X86Bcj):
        return FastExtensionTransform::X86Bcj;
    default:
        throw std::runtime_error("Unsupported HZ02 fast extension transform");
    }
}

bool is_shuffle_width(const std::uint8_t width) noexcept {
    return width == 2U || width == 4U || width == 8U;
}

bool is_delta_width(const std::uint8_t width) noexcept {
    return width == 1U || is_shuffle_width(width);
}

void validate_side_information(const FastExtensionMetadata& metadata) {
    switch (metadata.transform) {
    case FastExtensionTransform::None:
    case FastExtensionTransform::X86Bcj:
        if (!metadata.side_information.empty()) {
            throw std::runtime_error(
                "HZ02 fast extension transform has unexpected side information");
        }
        return;
    case FastExtensionTransform::ByteShuffle:
    case FastExtensionTransform::BitShuffle:
        if (metadata.side_information.size() != 1U ||
            !is_shuffle_width(metadata.side_information[0])) {
            throw std::runtime_error(
                "HZ02 fast extension shuffle width is malformed");
        }
        return;
    case FastExtensionTransform::XorDelta:
        if (metadata.side_information.size() != 1U ||
            !is_delta_width(metadata.side_information[0])) {
            throw std::runtime_error(
                "HZ02 fast extension delta width is malformed");
        }
        return;
    }
    throw std::runtime_error("Unsupported HZ02 fast extension transform");
}

FastExtensionMetadata parse_metadata(const ByteView metadata) {
    if (metadata.size() < 4U || metadata[0] != kExtensionVersion ||
        metadata[1] != kZstdCodec) {
        throw std::runtime_error("Unsupported HZ02 fast extension metadata");
    }
    std::size_t offset = 3U;
    const std::uint32_t side_information_size = read_uleb128(metadata, offset);
    if (side_information_size > metadata.size() - offset ||
        side_information_size != metadata.size() - offset) {
        throw std::runtime_error(
            "HZ02 fast extension side information is malformed");
    }
    FastExtensionMetadata result;
    result.transform = decode_transform(metadata[2]);
    result.side_information.assign(metadata.data() + offset,
                                   metadata.data() + metadata.size());
    validate_side_information(result);
    return result;
}

}  // namespace

FastExtensionEncodedBlock FastExtensionBackend::encode_zstd(
    const ByteView input,
    const FastExtensionTransform transform,
    const ByteView side_information) const {
    FastExtensionMetadata descriptor;
    descriptor.transform = transform;
    descriptor.side_information = copy_bytes(side_information);
    validate_side_information(descriptor);

    FastExtensionEncodedBlock encoded;
    encoded.metadata.reserve(4U + descriptor.side_information.size());
    encoded.metadata.push_back(kExtensionVersion);
    encoded.metadata.push_back(kZstdCodec);
    encoded.metadata.push_back(static_cast<std::uint8_t>(transform));
    append_uleb128(encoded.metadata, static_cast<std::uint32_t>(
        descriptor.side_information.size()));
    encoded.metadata.insert(encoded.metadata.end(),
                            descriptor.side_information.begin(),
                            descriptor.side_information.end());
    encoded.payload = ZstdBackend(compression_level_, false, false, false)
                          .encode(input);
    return encoded;
}

FastExtensionEncodedBlock FastExtensionBackend::encode_raw_zstd(
    const ByteView input) const {
    return encode_zstd(input, FastExtensionTransform::None, ByteView{});
}

FastExtensionDecodedBlock FastExtensionBackend::decode_zstd(
    const ByteView payload,
    const ByteView metadata,
    const std::size_t expected_size) {
    FastExtensionDecodedBlock decoded;
    decoded.metadata = parse_metadata(metadata);
    decoded.bytes = ZstdBackend().decode(payload, expected_size);
    return decoded;
}

std::size_t FastExtensionBackend::maximum_payload_size(
    const std::size_t input_size) {
    return ZstdBackend::maximum_payload_size(input_size);
}

}  // namespace hz::r2
