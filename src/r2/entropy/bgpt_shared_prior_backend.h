#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

constexpr std::uint32_t kBgptSharedPriorModelId = 0x31504742U; // "BGP1"
constexpr std::array<std::uint8_t, 32> kBgptTextCheckpointSha256{{
    0xF3, 0x0E, 0xD5, 0xA8, 0x14, 0x08, 0x6C, 0x5B,
    0x9E, 0x64, 0xF5, 0x6A, 0x76, 0xCC, 0xFC, 0xDE,
    0xD0, 0x0A, 0x82, 0xFC, 0x71, 0xC3, 0xBA, 0x6D,
    0xE3, 0x22, 0xB7, 0x08, 0xD2, 0x9F, 0x6A, 0xC7,
}};

// A frozen, checkpoint-derived byte prior. The complete 110M bGPT runtime is
// intentionally not claimed here: this backend uses the reproducible 257x256
// bigram projection identified in third_party/bgpt-shared-prior/projection.json.
class BgptSharedPriorBackend final : public IBlockEntropyBackend {
public:
    const char* name() const noexcept override {
        return "bgpt-text-shared-prior-v1";
    }
    EntropyKind kind() const noexcept override {
        return EntropyKind::SymbolArithmetic;
    }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;
};

}  // namespace hz::r2
