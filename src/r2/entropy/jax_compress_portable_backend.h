#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

constexpr std::uint32_t kJaxCompressPortableModelId = 0x3150434AU; // "JCP1"
constexpr std::array<std::uint8_t, 20> kJaxCompressSourceRevision{{
    0x77, 0xAD, 0xBC, 0x58, 0x1E, 0xB0, 0x81, 0x9A, 0x77, 0xE4,
    0x7C, 0x50, 0xFF, 0x6E, 0xD8, 0xEC, 0xE3, 0x38, 0xE6, 0x0C,
}};
constexpr std::array<std::uint8_t, 32> kJaxCompressPortableProfileSha256{{
    0x32, 0xF2, 0x6C, 0x00, 0x71, 0x52, 0x9F, 0x7C,
    0xDF, 0x0B, 0x68, 0xB4, 0x15, 0x18, 0x70, 0x9A,
    0xE8, 0xD0, 0x9B, 0x05, 0x05, 0x86, 0xB1, 0xA9,
    0x89, 0x6A, 0x7C, 0x50, 0x39, 0xF7, 0x3B, 0xE7,
}};

// A CPU-portable adaptation of jax-compress's decoder-synchronized online
// training lifecycle. The exact reduced profile is carried in HZ02 metadata;
// this class does not claim equivalence to the donor's 8x1400 TPU profile.
class JaxCompressPortableBackend final : public IBlockEntropyBackend {
public:
    const char* name() const noexcept override {
        return "jax-compress-portable-v1";
    }
    EntropyKind kind() const noexcept override {
        return EntropyKind::SymbolArithmetic;
    }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;
};

}  // namespace hz::r2
