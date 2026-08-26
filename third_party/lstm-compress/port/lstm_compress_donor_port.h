#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/core/byte_view.h"

namespace hz::r2::lstm_compress_donor {

std::vector<std::uint8_t> encode(ByteView input);
std::vector<std::uint8_t> decode(ByteView payload, std::size_t expected_size);
std::size_t maximum_payload_size(std::size_t input_size);

}  // namespace hz::r2::lstm_compress_donor
