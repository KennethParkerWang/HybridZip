#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

// Port of fumin/ctw's CTW + Willems arithmetic-coder lifecycle. The depth,
// raw length, encoded bit count, and checksums are decoder-visible in HZC1.
class CtwBackend final : public IBlockEntropyBackend {
public:
    static constexpr std::uint8_t kDefaultDepth = 48;
    static constexpr std::uint8_t kMaximumDepth = 64;
    static constexpr std::size_t kPayloadHeaderSize = 40;
    static constexpr std::size_t kMaximumOutputSize = 64U * 1024U;

    explicit CtwBackend(
        std::uint8_t depth = kDefaultDepth,
        std::size_t maximum_output_size = kMaximumOutputSize);

    const char* name() const noexcept override { return "ctw-fumin"; }
    EntropyKind kind() const noexcept override { return EntropyKind::Ctw; }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;

    static std::size_t maximum_payload_size(std::size_t input_size);

private:
    std::uint8_t depth_;
    std::size_t maximum_output_size_;
};

}  // namespace hz::r2
