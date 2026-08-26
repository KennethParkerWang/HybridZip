#pragma once

#include <cstdint>

#include "r2/core/byte_view.h"

namespace hz::r2 {

struct Paq8pxBlockProfile {
  std::uint8_t donor_type = 0;
  std::int32_t block_info = -1;
  std::uint32_t data_start = 0;
  std::uint32_t data_length = 0;
};

Paq8pxBlockProfile detect_paq8px_block_profile(ByteView input);
bool paq8px_profile_uses_specialist(const Paq8pxBlockProfile &profile) noexcept;

} // namespace hz::r2
