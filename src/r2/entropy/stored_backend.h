#pragma once

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

class StoredBackend final : public IBlockEntropyBackend {
public:
    const char* name() const noexcept override { return "stored"; }
    EntropyKind kind() const noexcept override { return EntropyKind::Stored; }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;
};

}  // namespace hz::r2

