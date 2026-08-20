#pragma once

#include <cstdint>
#include <vector>

#include "r2/core/byte_view.h"

namespace hz::r2 {

struct FastPforEncodedBlock {
    std::vector<std::uint8_t> payload;
    std::vector<std::uint8_t> metadata;
};

class FastPforBackend {
public:
    static constexpr std::size_t kBlockBytes = 1024;

    bool applicable(ByteView input) const noexcept;
    FastPforEncodedBlock encode(ByteView input) const;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     ByteView metadata,
                                     std::size_t expected_size) const;
};

}  // namespace hz::r2
