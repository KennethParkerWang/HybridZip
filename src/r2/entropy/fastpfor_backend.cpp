#include "r2/entropy/fastpfor_backend.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

#include "fastpfor.h"

namespace hz::r2 {
namespace {

std::uint16_t read_u16_le(const ByteView bytes) {
    return static_cast<std::uint16_t>(bytes[0]) |
        (static_cast<std::uint16_t>(bytes[1]) << 8U);
}

void append_u16_le(std::vector<std::uint8_t>& bytes, const std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value));
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
}

void append_u32_le(std::vector<std::uint8_t>& bytes, const std::uint32_t value) {
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        bytes.push_back(static_cast<std::uint8_t>(value >> shift));
    }
}

std::uint32_t read_u32_le(const ByteView bytes, const std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
        (static_cast<std::uint32_t>(bytes[offset + 1]) << 8U) |
        (static_cast<std::uint32_t>(bytes[offset + 2]) << 16U) |
        (static_cast<std::uint32_t>(bytes[offset + 3]) << 24U);
}

std::vector<std::uint32_t> read_words(const ByteView input,
                                      const std::size_t count) {
    std::vector<std::uint32_t> words(count);
    for (std::size_t index = 0; index < count; ++index) {
        words[index] = read_u32_le(input, index * 4U);
    }
    return words;
}

std::vector<std::uint8_t> write_words(const std::vector<std::uint32_t>& words,
                                      const std::size_t count) {
    std::vector<std::uint8_t> bytes;
    bytes.reserve(count * 4U);
    for (std::size_t index = 0; index < count; ++index) {
        append_u32_le(bytes, words[index]);
    }
    return bytes;
}

}  // namespace

bool FastPforBackend::applicable(const ByteView input) const noexcept {
    return input.size() >= kBlockBytes;
}

FastPforEncodedBlock FastPforBackend::encode(const ByteView input) const {
    if (!applicable(input)) {
        throw std::invalid_argument("FastPFOR requires at least one 1024-byte group");
    }
    const std::size_t encoded_bytes = input.size() / kBlockBytes * kBlockBytes;
    const std::size_t word_count = encoded_bytes / 4U;
    const std::size_t tail_size = input.size() - encoded_bytes;
    const std::vector<std::uint32_t> words = read_words(input, word_count);
    std::vector<std::uint32_t> compressed(word_count * 2U + 1024U);
    std::size_t compressed_words = compressed.size();
    FastPForLib::FastPFor<8> donor;
    donor.encodeArray(words.data(), words.size(), compressed.data(), compressed_words);
    if (compressed_words == 0 || compressed_words > compressed.size()) {
        throw std::runtime_error("FastPFOR encoder returned an invalid size");
    }

    FastPforEncodedBlock result{};
    result.payload = write_words(compressed, compressed_words);
    append_u16_le(result.metadata, static_cast<std::uint16_t>(tail_size));
    result.metadata.insert(result.metadata.end(), input.data() + encoded_bytes,
                           input.data() + input.size());
    return result;
}

std::vector<std::uint8_t> FastPforBackend::decode(
    const ByteView payload, const ByteView metadata,
    const std::size_t expected_size) const {
    if (payload.empty() || payload.size() % 4U != 0 || metadata.size() < 2U) {
        throw std::runtime_error("Invalid FastPFOR framing");
    }
    const std::size_t tail_size = read_u16_le(metadata);
    if (tail_size != metadata.size() - 2U || tail_size >= kBlockBytes ||
        expected_size < tail_size ||
        (expected_size - tail_size) % kBlockBytes != 0) {
        throw std::runtime_error("Invalid FastPFOR tail metadata");
    }
    const std::size_t decoded_words = (expected_size - tail_size) / 4U;
    const std::vector<std::uint32_t> compressed = read_words(
        payload, payload.size() / 4U);
    std::vector<std::uint32_t> decoded(decoded_words);
    std::size_t reported_words = decoded.size();
    FastPForLib::FastPFor<8> donor;
    const std::uint32_t* const consumed = donor.decodeArray(
        compressed.data(), compressed.size(), decoded.data(), reported_words);
    if (reported_words != decoded_words ||
        consumed != compressed.data() + compressed.size()) {
        throw std::runtime_error("FastPFOR decoder did not consume its framing");
    }
    std::vector<std::uint8_t> result = write_words(decoded, decoded.size());
    result.insert(result.end(), metadata.data() + 2U,
                  metadata.data() + metadata.size());
    return result;
}

}  // namespace hz::r2
