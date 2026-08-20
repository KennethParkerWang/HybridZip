#pragma once

#include <cstddef>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

class FseBackend final : public IBlockEntropyBackend {
public:
    const char* name() const noexcept override { return "fse"; }
    EntropyKind kind() const noexcept override { return EntropyKind::Fse; }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(
        ByteView payload,
        std::size_t expected_size) const override;

    static std::size_t maximum_payload_size(std::size_t input_size);
};

}  // namespace hz::r2
