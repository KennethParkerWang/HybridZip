#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/core/byte_view.h"

namespace hz::r2 {

enum class EntropyKind : std::uint8_t {
    Stored = 0,
    SymbolArithmetic = 1,
    ZstdFse = 2,
    Rans = 3,
    Fse = 4,
    Lzma = 5
};

class IBlockEntropyBackend {
public:
    virtual ~IBlockEntropyBackend() = default;

    virtual const char* name() const noexcept = 0;
    virtual EntropyKind kind() const noexcept = 0;
    virtual std::vector<std::uint8_t> encode(ByteView input) const = 0;
    virtual std::vector<std::uint8_t> decode(
        ByteView payload,
        std::size_t expected_size) const = 0;
};

}  // namespace hz::r2
