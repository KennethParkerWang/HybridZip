#include "r2/entropy/stored_backend.h"

#include <stdexcept>

namespace hz::r2 {

std::vector<std::uint8_t> StoredBackend::encode(const ByteView input) const {
    return copy_bytes(input);
}

std::vector<std::uint8_t> StoredBackend::decode(
    const ByteView payload,
    const std::size_t expected_size) const {
    if (payload.size() != expected_size) {
        throw std::runtime_error("Stored block size does not match metadata");
    }
    return copy_bytes(payload);
}

}  // namespace hz::r2

