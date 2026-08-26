#pragma once

#include <cstddef>
#include <cstdint>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

class ZpaqBackend final : public IBlockEntropyBackend {
public:
    static constexpr std::size_t kPayloadHeaderSize = 32;
    static constexpr std::uint8_t kDefaultMethodLevel = 3;
    static constexpr std::size_t kDefaultMaximumOutputSize =
        16U * 1024U * 1024U;

    explicit ZpaqBackend(
        std::uint8_t method_level = kDefaultMethodLevel,
        std::size_t maximum_output_size = kDefaultMaximumOutputSize);

    const char* name() const noexcept override { return "zpaq-libzpaq"; }
    EntropyKind kind() const noexcept override { return EntropyKind::Zpaq; }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;

    static std::size_t maximum_payload_size(std::size_t input_size);

private:
    std::uint8_t method_level_;
    std::size_t maximum_output_size_;
};

}  // namespace hz::r2
