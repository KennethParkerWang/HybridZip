#include "archive/archive_header.h"

#include <array>
#include <istream>
#include <ostream>
#include <stdexcept>

namespace hz {
namespace {

void write_byte(std::ostream& output, const std::uint8_t value) {
    output.put(static_cast<char>(value));
    if (!output) {
        throw std::runtime_error("Failed to write HZ01 header");
    }
}

void write_u16_le(std::ostream& output, const std::uint16_t value) {
    for (unsigned shift = 0; shift < 16; shift += 8) {
        write_byte(output, static_cast<std::uint8_t>(value >> shift));
    }
}

void write_u32_le(std::ostream& output, const std::uint32_t value) {
    for (unsigned shift = 0; shift < 32; shift += 8) {
        write_byte(output, static_cast<std::uint8_t>(value >> shift));
    }
}

void write_u64_le(std::ostream& output, const std::uint64_t value) {
    for (unsigned shift = 0; shift < 64; shift += 8) {
        write_byte(output, static_cast<std::uint8_t>(value >> shift));
    }
}

std::uint8_t read_byte(std::istream& input) {
    const int value = input.get();
    if (value == std::char_traits<char>::eof()) {
        throw std::runtime_error("Truncated HZ01 header");
    }
    return static_cast<std::uint8_t>(value);
}

std::uint16_t read_u16_le(std::istream& input) {
    std::uint16_t result = 0;
    for (unsigned shift = 0; shift < 16; shift += 8) {
        result |= static_cast<std::uint16_t>(read_byte(input)) << shift;
    }
    return result;
}

std::uint32_t read_u32_le(std::istream& input) {
    std::uint32_t result = 0;
    for (unsigned shift = 0; shift < 32; shift += 8) {
        result |= static_cast<std::uint32_t>(read_byte(input)) << shift;
    }
    return result;
}

std::uint64_t read_u64_le(std::istream& input) {
    std::uint64_t result = 0;
    for (unsigned shift = 0; shift < 64; shift += 8) {
        result |= static_cast<std::uint64_t>(read_byte(input)) << shift;
    }
    return result;
}

}  // namespace

void write_archive_header(std::ostream& output, const ArchiveHeader& header) {
    output.write("HZ01", 4);
    write_u16_le(output, kArchiveVersion);
    write_u16_le(output, kArchiveHeaderSize);
    write_u64_le(output, header.original_size);
    write_u32_le(output, header.profile_id);
    write_u32_le(output, header.flags);
    write_u64_le(output, header.model_seed);
    write_byte(output, header.cdf_bits);
    write_byte(output, header.coder_state_bits);
    for (std::size_t index = 0; index < 6; ++index) {
        write_byte(output, 0);
    }
}

ArchiveHeader read_archive_header(std::istream& input) {
    std::array<char, 4> magic{};
    input.read(magic.data(), static_cast<std::streamsize>(magic.size()));
    if (!input || magic != std::array<char, 4>{'H', 'Z', '0', '1'}) {
        throw std::runtime_error("Invalid HZ01 magic");
    }
    if (read_u16_le(input) != kArchiveVersion ||
        read_u16_le(input) != kArchiveHeaderSize) {
        throw std::runtime_error("Unsupported HZ01 version or header size");
    }

    ArchiveHeader header{};
    header.original_size = read_u64_le(input);
    header.profile_id = read_u32_le(input);
    header.flags = read_u32_le(input);
    header.model_seed = read_u64_le(input);
    header.cdf_bits = read_byte(input);
    header.coder_state_bits = read_byte(input);
    for (std::size_t index = 0; index < 6; ++index) {
        if (read_byte(input) != 0) {
            throw std::runtime_error("HZ01 reserved header byte is non-zero");
        }
    }

    if (header.profile_id != 1U || header.flags != 0U ||
        header.cdf_bits != kCdfBits ||
        header.coder_state_bits != kCoderStateBits) {
        throw std::runtime_error("Unsupported HZ01 profile");
    }
    return header;
}

}  // namespace hz
