#include "r2/entropy/fse_backend.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "hybridzip_fse_api.h"

namespace hz::r2 {
namespace {

enum class FsePayloadKind : std::uint8_t {
    Raw = 0,
    Rle = 1,
    Compressed = 2
};

void require_fse_success(const std::size_t result, const char* operation) {
    if (HZFSE_isError(result) != 0U) {
        throw std::runtime_error(std::string(operation) + ": " +
                                 HZFSE_getErrorName(result));
    }
}

std::uint8_t payload_kind(const FsePayloadKind kind) noexcept {
    return static_cast<std::uint8_t>(kind);
}

}  // namespace

std::vector<std::uint8_t> FseBackend::encode(const ByteView input) const {
    if (input.empty()) {
        return {payload_kind(FsePayloadKind::Raw)};
    }
    const bool run_length = std::all_of(
        input.data() + 1U, input.data() + input.size(),
        [&](const std::uint8_t value) { return value == input[0]; });
    if (run_length) {
        return {payload_kind(FsePayloadKind::Rle), input[0]};
    }

    const std::size_t bound = HZFSE_compressBound(input.size());
    if (bound == 0U || bound == std::numeric_limits<std::size_t>::max()) {
        throw std::runtime_error("FSE compression bound is invalid");
    }
    std::vector<std::uint8_t> output(bound + 1U);
    const std::size_t written = HZFSE_compress(
        output.data() + 1U, bound, input.data(), input.size());
    require_fse_success(written, "FSE compression failed");
    if (written == 0U) {
        output.resize(input.size() + 1U);
        output[0] = payload_kind(FsePayloadKind::Raw);
        std::copy(input.data(), input.data() + input.size(),
                  output.begin() + 1);
        return output;
    }
    if (written == 1U) {
        return {payload_kind(FsePayloadKind::Rle), input[0]};
    }
    output[0] = payload_kind(FsePayloadKind::Compressed);
    output.resize(written + 1U);
    return output;
}

std::vector<std::uint8_t> FseBackend::decode(
    const ByteView payload,
    const std::size_t expected_size) const {
    if (payload.empty()) {
        throw std::runtime_error("FSE payload is empty");
    }
    const auto kind = static_cast<FsePayloadKind>(payload[0]);
    switch (kind) {
        case FsePayloadKind::Raw: {
            if (payload.size() != expected_size + 1U) {
                throw std::runtime_error(
                    "Raw FSE payload size does not match metadata");
            }
            return {payload.data() + 1U,
                    payload.data() + payload.size()};
        }
        case FsePayloadKind::Rle:
            if (expected_size == 0U || payload.size() != 2U) {
                throw std::runtime_error("Invalid RLE FSE payload");
            }
            return std::vector<std::uint8_t>(expected_size, payload[1]);
        case FsePayloadKind::Compressed: {
            if (expected_size == 0U || payload.size() <= 1U) {
                throw std::runtime_error("Invalid compressed FSE payload");
            }
            std::vector<std::uint8_t> output(expected_size);
            const std::size_t written = HZFSE_decompress(
                output.data(), output.size(), payload.data() + 1U,
                payload.size() - 1U);
            require_fse_success(written, "FSE decompression failed");
            if (written != expected_size) {
                throw std::runtime_error(
                    "FSE output size does not match metadata");
            }
            return output;
        }
    }
    throw std::runtime_error("Unknown FSE payload kind");
}

std::size_t FseBackend::maximum_payload_size(
    const std::size_t input_size) {
    if (input_size == std::numeric_limits<std::size_t>::max()) {
        throw std::overflow_error("FSE payload bound overflow");
    }
    const std::size_t compressed_bound = HZFSE_compressBound(input_size);
    const std::size_t maximum = std::max(input_size, compressed_bound);
    if (maximum == std::numeric_limits<std::size_t>::max()) {
        throw std::overflow_error("FSE payload bound overflow");
    }
    return maximum + 1U;
}

}  // namespace hz::r2
