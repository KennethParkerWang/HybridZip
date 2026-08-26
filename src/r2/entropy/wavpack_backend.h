#pragma once

#include <cstddef>
#include <cstdint>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

class WavpackBackend final : public IBlockEntropyBackend {
public:
    static constexpr std::size_t kPayloadHeaderSize = 16;

    const char* name() const noexcept override { return "wavpack-lossless"; }
    EntropyKind kind() const noexcept override { return EntropyKind::Wavpack; }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(
        ByteView payload,
        std::size_t expected_size) const override;

    static std::size_t maximum_payload_size(std::size_t input_size);
};

}  // namespace hz::r2
