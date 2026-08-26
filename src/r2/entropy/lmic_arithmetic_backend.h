#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

// "LMC1" identifies the real LMIC arithmetic-coder lifecycle paired with a
// decoder-synchronised frozen bGPT byte prior.  This is not the unavailable
// LMIC Transformer checkpoint codec.
constexpr std::uint32_t kLmicArithmeticModelId = 0x31434D4CU;
constexpr std::array<std::uint8_t, 32> kLmicArithmeticProfileSha256{{
    0x28, 0xEF, 0x1A, 0xF1, 0x64, 0xA4, 0xBF, 0x41,
    0x76, 0xD4, 0xF2, 0xC0, 0x5B, 0xA4, 0xAD, 0x5C,
    0xC3, 0x8A, 0x33, 0x6E, 0x11, 0x42, 0x20, 0x20,
    0x9E, 0x19, 0x6C, 0x89, 0xE3, 0xD8, 0xA1, 0x81,
}};

constexpr std::size_t kLmicArithmeticPayloadHeaderSize = 8U;

class LmicArithmeticBackend final : public IBlockEntropyBackend {
public:
    const char* name() const noexcept override {
        return "lmic-arithmetic-frozen-bgpt-v1";
    }
    EntropyKind kind() const noexcept override {
        return EntropyKind::LmicArithmetic;
    }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;

    static std::size_t maximum_payload_size(std::size_t input_size);
};

}  // namespace hz::r2
