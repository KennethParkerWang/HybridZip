#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

class Lz4Backend final : public IBlockEntropyBackend {
public:
    static constexpr std::size_t kPayloadHeaderSize = 24;
    static constexpr std::uint8_t kCompressionLevel = 12;

    const char* name() const noexcept override { return "lz4-hc"; }
    EntropyKind kind() const noexcept override { return EntropyKind::Lz4; }

    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;

    static std::size_t maximum_payload_size(std::size_t input_size);
};

}  // namespace hz::r2
