#pragma once

#include <cstdint>
#include <iosfwd>

#include "core/types.h"

namespace hz {

constexpr std::uint16_t kArchiveVersion = 1;
constexpr std::uint16_t kArchiveHeaderSize = 40;

struct ArchiveHeader {
    std::uint64_t original_size = 0;
    std::uint32_t profile_id = 1;
    std::uint32_t flags = 0;
    std::uint64_t model_seed = kDefaultModelSeed;
    std::uint8_t cdf_bits = static_cast<std::uint8_t>(kCdfBits);
    std::uint8_t coder_state_bits = static_cast<std::uint8_t>(kCoderStateBits);
};

void write_archive_header(std::ostream& output, const ArchiveHeader& header);
ArchiveHeader read_archive_header(std::istream& input);

}  // namespace hz
