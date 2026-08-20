#pragma once

#include <cstddef>
#include <cstdint>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

class LzmaBackend final : public IBlockEntropyBackend {
public:
    static constexpr std::size_t kPayloadHeaderSize = 40;
    static constexpr std::size_t kDefaultMaximumOutputSize =
        16U * 1024U * 1024U;
    static constexpr std::uint32_t kMaximumDictionarySize =
        64U * 1024U * 1024U;

    explicit LzmaBackend(
        int compression_level = 9,
        std::uint32_t dictionary_size = 0,
        std::size_t maximum_output_size = kDefaultMaximumOutputSize);

    const char* name() const noexcept override { return "lzma"; }
    EntropyKind kind() const noexcept override { return EntropyKind::Lzma; }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;

    static std::size_t maximum_payload_size(std::size_t input_size);

private:
    int compression_level_;
    std::uint32_t dictionary_size_;
    std::size_t maximum_output_size_;
};

}  // namespace hz::r2
