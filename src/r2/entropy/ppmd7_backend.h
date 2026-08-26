#pragma once

#include <cstddef>
#include <cstdint>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

class Ppmd7Backend final : public IBlockEntropyBackend {
public:
    static constexpr std::size_t kPayloadHeaderSize = 32;
    static constexpr unsigned kDefaultOrder = 8;
    static constexpr std::uint32_t kDefaultMemorySize = 8U * 1024U * 1024U;
    static constexpr std::uint32_t kMaximumMemorySize = 64U * 1024U * 1024U;
    static constexpr std::size_t kDefaultMaximumOutputSize =
        16U * 1024U * 1024U;

    explicit Ppmd7Backend(
        unsigned order = kDefaultOrder,
        std::uint32_t memory_size = kDefaultMemorySize,
        std::size_t maximum_output_size = kDefaultMaximumOutputSize);

    const char* name() const noexcept override { return "ppmd7-7zip"; }
    EntropyKind kind() const noexcept override { return EntropyKind::Ppmd7; }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;

    static std::size_t maximum_payload_size(std::size_t input_size);

private:
    unsigned order_;
    std::uint32_t memory_size_;
    std::size_t maximum_output_size_;
};

}  // namespace hz::r2
