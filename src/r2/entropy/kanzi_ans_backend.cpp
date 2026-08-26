#include "r2/entropy/kanzi_ans_backend.h"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <string>

#include "EntropyDecoder.hpp"
#include "EntropyEncoder.hpp"
#include "InputBitStream.hpp"
#include "OutputBitStream.hpp"
#include "entropy/ANSRangeDecoder.hpp"
#include "entropy/ANSRangeEncoder.hpp"

namespace hz::r2 {
namespace {

constexpr std::array<std::uint8_t, 4> kMagic{'H', 'Z', 'K', '1'};
constexpr std::uint8_t kVersion = 1;
constexpr std::uint8_t kOrder = 0;
constexpr std::uint8_t kLogRange = 12;
constexpr std::uint32_t kChunkSize = 16384;
constexpr std::size_t kOrderOffset = 5;
constexpr std::size_t kLogRangeOffset = 6;
constexpr std::size_t kValidBitsOffset = 8;
constexpr std::size_t kOriginalSizeOffset = 12;
constexpr std::size_t kRawCrcOffset = 16;
constexpr std::size_t kStreamCrcOffset = 20;
constexpr std::size_t kReservedOffset = 24;

std::size_t checked_add(std::size_t a, std::size_t b, const char* message) {
    if (a > std::numeric_limits<std::size_t>::max() - b) {
        throw std::length_error(message);
    }
    return a + b;
}

std::size_t checked_multiply(std::size_t a, std::size_t b, const char* message) {
    if (a != 0 && b > std::numeric_limits<std::size_t>::max() / a) {
        throw std::length_error(message);
    }
    return a * b;
}

std::uint32_t crc32(ByteView input) noexcept {
    std::uint32_t checksum = 0xFFFFFFFFU;
    for (std::size_t index = 0; index < input.size(); ++index) {
        checksum ^= input[index];
        for (unsigned bit = 0; bit < 8U; ++bit) {
            const std::uint32_t mask =
                0U - static_cast<std::uint32_t>(checksum & 1U);
            checksum = (checksum >> 1U) ^ (0xEDB88320U & mask);
        }
    }
    return ~checksum;
}

void write_u32_le(std::vector<std::uint8_t>& output, std::size_t offset,
                  std::uint32_t value) noexcept {
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        output[offset + shift / 8U] =
            static_cast<std::uint8_t>(value >> shift);
    }
}

std::uint32_t read_u32_le(ByteView input, std::size_t offset) noexcept {
    std::uint32_t value = 0;
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        value |= static_cast<std::uint32_t>(input[offset + shift / 8U]) << shift;
    }
    return value;
}

class MemoryOutputBitStream final : public kanzi::OutputBitStream {
public:
    void writeBit(int bit) override {
        writeBits(static_cast<kanzi::uint64>(bit & 1), 1);
    }

    kanzi::uint writeBits(kanzi::uint64 value, kanzi::uint count) override {
        if (count > 64U) {
            throw std::invalid_argument("Kanzi ANS output bit count is invalid");
        }
        for (kanzi::uint index = 0; index < count; ++index) {
            const unsigned shift = count - index - 1U;
            append_bit(static_cast<unsigned>((value >> shift) & 1U));
        }
        return count;
    }

    kanzi::uint writeBits(const kanzi::byte bits[], kanzi::uint count) override {
        if (bits == nullptr && count != 0U) {
            throw std::invalid_argument("Kanzi ANS output received null bytes");
        }
        for (kanzi::uint index = 0; index < count; ++index) {
            const unsigned shift = 7U - (index & 7U);
            append_bit((std::to_integer<unsigned>(bits[index >> 3U]) >> shift) &
                       1U);
        }
        return count;
    }

    void close() override { closed_ = true; }
    kanzi::uint64 written() const override { return bit_count_; }
    const std::vector<std::uint8_t>& bytes() const noexcept { return bytes_; }

private:
    void append_bit(unsigned bit) {
        if (closed_) {
            throw std::runtime_error("Kanzi ANS output is closed");
        }
        if ((bit_count_ & 7U) == 0U) {
            bytes_.push_back(0);
        }
        if (bit != 0U) {
            bytes_.back() |= static_cast<std::uint8_t>(
                1U << (7U - (bit_count_ & 7U)));
        }
        ++bit_count_;
    }

    std::vector<std::uint8_t> bytes_;
    kanzi::uint64 bit_count_ = 0;
    bool closed_ = false;
};

class MemoryInputBitStream final : public kanzi::InputBitStream {
public:
    MemoryInputBitStream(const std::uint8_t* data, std::size_t bytes,
                         std::uint64_t valid_bits)
        : data_(data), size_(bytes), valid_bits_(valid_bits) {}

    int readBit() override {
        return static_cast<int>(readBits(1));
    }

    kanzi::uint64 readBits(kanzi::uint count) override {
        if (count == 0U || count > 64U ||
            count > valid_bits_ - position_) {
            throw std::runtime_error("Truncated Kanzi ANS bitstream");
        }
        kanzi::uint64 value = 0;
        for (kanzi::uint index = 0; index < count; ++index) {
            value = (value << 1U) | read_one();
        }
        return value;
    }

    kanzi::uint readBits(kanzi::byte bits[], kanzi::uint count) override {
        if (bits == nullptr && count != 0U) {
            throw std::invalid_argument("Kanzi ANS input received null bytes");
        }
        for (kanzi::uint index = 0; index < count; ++index) {
            if ((index & 7U) == 0U) bits[index >> 3U] = kanzi::byte{0};
            bits[index >> 3U] |= kanzi::byte(
                read_one() << (7U - (index & 7U)));
        }
        return count;
    }

    void close() override { closed_ = true; }
    kanzi::uint64 read() const override { return position_; }
    bool hasMoreToRead() override { return position_ < valid_bits_; }

private:
    unsigned read_one() {
        if (closed_ || position_ >= valid_bits_ ||
            position_ / 8U >= size_) {
            throw std::runtime_error("Truncated Kanzi ANS bitstream");
        }
        const unsigned bit = (data_[position_ / 8U] >>
                              (7U - (position_ & 7U))) & 1U;
        ++position_;
        return bit;
    }

    const std::uint8_t* data_;
    std::size_t size_;
    std::uint64_t valid_bits_;
    std::uint64_t position_ = 0;
    bool closed_ = false;
};

void require_input(ByteView input, const char* operation) {
    if (!input.empty() && input.data() == nullptr) {
        throw std::invalid_argument(std::string(operation) +
                                    " received null bytes");
    }
}

}  // namespace

std::vector<std::uint8_t> KanziAnsBackend::encode(ByteView input) const {
    require_input(input, "Kanzi ANS encoder");
    if (input.empty() || input.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::invalid_argument("Kanzi ANS requires a non-empty 32-bit block");
    }
    MemoryOutputBitStream stream;
    kanzi::ANSRangeEncoder encoder(stream, kOrder, kChunkSize, kLogRange);
    encoder.encode(reinterpret_cast<const kanzi::byte*>(input.data()), 0,
                   static_cast<kanzi::uint>(input.size()));
    stream.close();
    const std::size_t stream_size = stream.bytes().size();
    std::vector<std::uint8_t> output(kPayloadHeaderSize + stream_size, 0);
    std::copy(kMagic.begin(), kMagic.end(), output.begin());
    output[4] = kVersion;
    output[kOrderOffset] = kOrder;
    output[kLogRangeOffset] = kLogRange;
    write_u32_le(output, kValidBitsOffset,
                 static_cast<std::uint32_t>(stream.written()));
    write_u32_le(output, kOriginalSizeOffset,
                 static_cast<std::uint32_t>(input.size()));
    write_u32_le(output, kRawCrcOffset, crc32(input));
    const ByteView encoded(stream.bytes().data(), stream_size);
    write_u32_le(output, kStreamCrcOffset, crc32(encoded));
    std::copy(stream.bytes().begin(), stream.bytes().end(),
              output.begin() + kPayloadHeaderSize);
    return output;
}

std::vector<std::uint8_t> KanziAnsBackend::decode(
    ByteView payload, std::size_t expected_size) const {
    require_input(payload, "Kanzi ANS decoder");
    if (payload.size() < kPayloadHeaderSize + 1U ||
        !std::equal(kMagic.begin(), kMagic.end(), payload.data()) ||
        payload[4] != kVersion || payload[kOrderOffset] != kOrder ||
        payload[kLogRangeOffset] != kLogRange) {
        throw std::runtime_error("Invalid Kanzi ANS payload header");
    }
    for (std::size_t index = kReservedOffset; index < kPayloadHeaderSize; ++index) {
        if (payload[index] != 0) throw std::runtime_error("Invalid Kanzi ANS reserved bytes");
    }
    const std::size_t stream_size = payload.size() - kPayloadHeaderSize;
    const std::uint64_t valid_bits = read_u32_le(payload, kValidBitsOffset);
    const std::size_t declared_size = read_u32_le(payload, kOriginalSizeOffset);
    if (declared_size != expected_size || declared_size == 0 ||
        valid_bits == 0 || valid_bits > stream_size * 8ULL ||
        (valid_bits + 7U) / 8U != stream_size ||
        crc32(ByteView(payload.data() + kPayloadHeaderSize, stream_size)) !=
            read_u32_le(payload, kStreamCrcOffset)) {
        throw std::runtime_error("Invalid Kanzi ANS payload bounds or checksum");
    }
    MemoryInputBitStream stream(payload.data() + kPayloadHeaderSize, stream_size,
                                valid_bits);
    std::vector<std::uint8_t> output(declared_size);
    kanzi::ANSRangeDecoder decoder(stream, kOrder, kChunkSize);
    if (decoder.decode(reinterpret_cast<kanzi::byte*>(output.data()), 0,
                       static_cast<kanzi::uint>(output.size())) !=
        static_cast<int>(output.size()) || stream.read() != valid_bits) {
        throw std::runtime_error("Kanzi ANS stream did not finish exactly");
    }
    if (crc32(ByteView(output)) != read_u32_le(payload, kRawCrcOffset)) {
        throw std::runtime_error("Kanzi ANS decoded checksum mismatch");
    }
    return output;
}

std::size_t KanziAnsBackend::maximum_payload_size(std::size_t input_size) {
    return checked_add(checked_multiply(input_size, 2U,
                                        "Kanzi ANS payload bound overflow"),
                       1024U * 1024U + kPayloadHeaderSize,
                       "Kanzi ANS payload bound overflow");
}

}  // namespace hz::r2
