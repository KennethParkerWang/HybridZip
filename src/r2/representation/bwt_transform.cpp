#include "r2/representation/bwt_transform.h"

#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

#include "libsais.h"

namespace hz::r2 {
namespace {

constexpr std::size_t kPrimaryIndexSize = 4;

std::int32_t checked_length(const ByteView input, const char* operation) {
    if (input.empty() || input.size() >
                             static_cast<std::size_t>(
                                 std::numeric_limits<std::int32_t>::max())) {
        throw std::invalid_argument(std::string(operation) +
                                    " requires 1..INT32_MAX bytes");
    }
    return static_cast<std::int32_t>(input.size());
}

std::uint32_t read_u32_le(const ByteView input) {
    if (input.size() != kPrimaryIndexSize) {
        throw std::runtime_error("BWT primary index metadata is malformed");
    }
    return static_cast<std::uint32_t>(input[0]) |
           (static_cast<std::uint32_t>(input[1]) << 8U) |
           (static_cast<std::uint32_t>(input[2]) << 16U) |
           (static_cast<std::uint32_t>(input[3]) << 24U);
}

std::vector<std::uint8_t> write_u32_le(const std::uint32_t value) {
    return {
        static_cast<std::uint8_t>(value),
        static_cast<std::uint8_t>(value >> 8U),
        static_cast<std::uint8_t>(value >> 16U),
        static_cast<std::uint8_t>(value >> 24U),
    };
}

}  // namespace

bool BwtTransform::applicable(const ByteView input,
                              const StructureFeatures&) const {
    return !input.empty() && input.size() <=
                                 static_cast<std::size_t>(
                                     std::numeric_limits<std::int32_t>::max());
}

TransformResult BwtTransform::forward(const ByteView input) const {
    const std::int32_t length = checked_length(input, "BWT transform");
    TransformResult result{};
    result.bytes.resize(input.size());
    std::vector<std::int32_t> workspace(input.size());
    std::array<std::int32_t, 256> frequencies{};
    const std::int32_t primary = libsais_bwt(
        input.data(), result.bytes.data(), workspace.data(), length, 0,
        frequencies.data());
    if (primary <= 0 || primary > length) {
        throw std::runtime_error("libsais_bwt failed");
    }
    result.side_information = write_u32_le(
        static_cast<std::uint32_t>(primary));
    return result;
}

std::vector<std::uint8_t> BwtTransform::inverse(
    const ByteView transformed,
    const ByteView side_information) const {
    const std::int32_t length = checked_length(transformed, "Inverse BWT");
    const std::uint32_t primary = read_u32_le(side_information);
    if (primary == 0 || primary > static_cast<std::uint32_t>(length)) {
        throw std::runtime_error("BWT primary index is outside the block");
    }

    std::vector<std::uint8_t> result(transformed.size());
    std::vector<std::int32_t> workspace(transformed.size() + 1U);
    if (libsais_unbwt(transformed.data(), result.data(), workspace.data(),
                      length, nullptr,
                      static_cast<std::int32_t>(primary)) != 0) {
        throw std::runtime_error("libsais_unbwt failed");
    }
    return result;
}

}  // namespace hz::r2
