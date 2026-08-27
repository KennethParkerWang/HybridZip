#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/core/byte_view.h"

namespace hz::r2 {

enum class FastExtensionTransform : std::uint8_t {
    None = 0,
    ByteShuffle = 1,
    BitShuffle = 2,
    XorDelta = 3,
    X86Bcj = 4
};

struct FastExtensionMetadata {
    FastExtensionTransform transform = FastExtensionTransform::None;
    std::vector<std::uint8_t> side_information;
};

struct FastExtensionEncodedBlock {
    std::vector<std::uint8_t> payload;
    std::vector<std::uint8_t> metadata;
};

struct FastExtensionDecodedBlock {
    std::vector<std::uint8_t> bytes;
    FastExtensionMetadata metadata;
};

class FastExtensionBackend final {
public:
    explicit FastExtensionBackend(int compression_level)
        : compression_level_(compression_level) {}

    FastExtensionEncodedBlock encode_zstd(
        ByteView input,
        FastExtensionTransform transform,
        ByteView side_information) const;

    FastExtensionEncodedBlock encode_raw_zstd(ByteView input) const;

    static FastExtensionDecodedBlock decode_zstd(
        ByteView payload,
        ByteView metadata,
        std::size_t expected_size);

    static std::size_t maximum_payload_size(std::size_t input_size);

private:
    int compression_level_;
};

}  // namespace hz::r2
