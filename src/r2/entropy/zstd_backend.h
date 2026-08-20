#pragma once

#include <cstddef>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

class ZstdBackend final : public IBlockEntropyBackend {
public:
    explicit ZstdBackend(int compression_level = 19)
        : compression_level_(compression_level) {}

    const char* name() const noexcept override { return "zstd"; }
    EntropyKind kind() const noexcept override { return EntropyKind::ZstdFse; }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;

    static std::size_t maximum_payload_size(std::size_t input_size);

private:
    int compression_level_;
};

}  // namespace hz::r2

