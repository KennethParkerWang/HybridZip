#include "r2/representation/jpegls_transform.h"

#include <array>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include "charls/jpegls_decoder.hpp"
#include "charls/jpegls_encoder.hpp"

namespace hz::r2 {
namespace {

constexpr std::size_t kGeometryMetadataSize = 8;
constexpr std::uint32_t kMaximumDimension = 100000;

void append_u32_le(std::vector<std::uint8_t>& output,
                   const std::uint32_t value) {
    output.push_back(static_cast<std::uint8_t>(value));
    output.push_back(static_cast<std::uint8_t>(value >> 8U));
    output.push_back(static_cast<std::uint8_t>(value >> 16U));
    output.push_back(static_cast<std::uint8_t>(value >> 24U));
}

std::uint32_t read_u32_le(const ByteView input, const std::size_t offset) {
    if (offset > input.size() || input.size() - offset < 4U) {
        throw std::runtime_error("JPEG-LS geometry metadata is malformed");
    }
    return static_cast<std::uint32_t>(input[offset]) |
           (static_cast<std::uint32_t>(input[offset + 1U]) << 8U) |
           (static_cast<std::uint32_t>(input[offset + 2U]) << 16U) |
           (static_cast<std::uint32_t>(input[offset + 3U]) << 24U);
}

bool valid_geometry(const std::size_t input_size, const std::uint32_t width,
                    const std::uint32_t height) noexcept {
    return width != 0U && height != 0U && width <= kMaximumDimension &&
           height <= kMaximumDimension &&
           static_cast<std::size_t>(width) <=
               std::numeric_limits<std::size_t>::max() /
                   static_cast<std::size_t>(height) &&
           static_cast<std::size_t>(width) * static_cast<std::size_t>(height) ==
               input_size;
}

}  // namespace

bool JpegLsTransform::applicable(const ByteView input,
                                 const std::uint32_t width) const noexcept {
    if (input.empty() || width == 0U || width > kMaximumDimension ||
        input.size() % width != 0U) {
        return false;
    }
    const std::size_t height = input.size() / width;
    return height != 0U && height <= kMaximumDimension;
}

TransformResult JpegLsTransform::forward(const ByteView input,
                                         const std::uint32_t width) const {
    if (!applicable(input, width)) {
        throw std::invalid_argument(
            "JPEG-LS requires a complete 8-bit one-component frame");
    }
    const std::uint32_t height = static_cast<std::uint32_t>(input.size() / width);
    const std::vector<std::uint8_t> source(input.data(), input.data() + input.size());
    const charls::frame_info frame{width, height, 8, 1};

    TransformResult result{};
    try {
        result.bytes = charls::jpegls_encoder::encode(source, frame);
    } catch (const std::exception& error) {
        throw std::runtime_error(std::string("CharLS JPEG-LS encoding failed: ") +
                                 error.what());
    }
    if (result.bytes.empty() || result.bytes.size() > maximum_payload_size(input.size())) {
        throw std::runtime_error("CharLS JPEG-LS output exceeds its bounded framing");
    }
    append_u32_le(result.side_information, width);
    append_u32_le(result.side_information, height);
    return result;
}

std::vector<std::uint8_t> JpegLsTransform::inverse(
    const ByteView payload, const ByteView side_information,
    const std::size_t expected_size) const {
    if (payload.size() < 2U || side_information.size() != kGeometryMetadataSize ||
        payload[payload.size() - 2U] != 0xFFU ||
        payload[payload.size() - 1U] != 0xD9U) {
        throw std::runtime_error("JPEG-LS payload framing is malformed");
    }
    const std::uint32_t width = read_u32_le(side_information, 0U);
    const std::uint32_t height = read_u32_le(side_information, 4U);
    if (!valid_geometry(expected_size, width, height)) {
        throw std::runtime_error("JPEG-LS geometry does not match the raw block");
    }

    const std::vector<std::uint8_t> source(payload.data(),
                                           payload.data() + payload.size());
    std::vector<std::uint8_t> output;
    try {
        const auto [frame, interleave] = charls::jpegls_decoder::decode(
            source, output, expected_size);
        if (frame.width != width || frame.height != height ||
            frame.bits_per_sample != 8 || frame.component_count != 1 ||
            interleave != charls::interleave_mode::none ||
            output.size() != expected_size) {
            throw std::runtime_error("JPEG-LS stream frame disagrees with HZ02 metadata");
        }
    } catch (const std::runtime_error&) {
        throw;
    } catch (const std::exception& error) {
        throw std::runtime_error(std::string("CharLS JPEG-LS decoding failed: ") +
                                 error.what());
    }
    return output;
}

std::size_t JpegLsTransform::maximum_payload_size(const std::size_t input_size) {
    constexpr std::size_t kOverhead = 1056U;
    const std::size_t limit =
        (std::numeric_limits<std::size_t>::max() - kOverhead) / 17U * 16U;
    if (input_size > limit) {
        throw std::runtime_error("JPEG-LS payload bound overflow");
    }
    return input_size + input_size / 16U + kOverhead;
}

}  // namespace hz::r2
