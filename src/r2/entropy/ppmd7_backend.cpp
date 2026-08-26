#include "r2/entropy/ppmd7_backend.h"

#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

#include "Alloc.h"
#include "Ppmd7.h"

namespace hz::r2 {
namespace {

constexpr std::array<std::uint8_t, 4> kPayloadMagic{'H', 'Z', 'P', '7'};
constexpr std::uint8_t kPayloadVersion = 1;
constexpr std::uint8_t kPayloadFlags = 1;  // Output length terminates the stream.
constexpr std::size_t kMemorySizeOffset = 8;
constexpr std::size_t kUncompressedSizeOffset = 12;
constexpr std::size_t kCompressedSizeOffset = 16;
constexpr std::size_t kUncompressedCrcOffset = 20;
constexpr std::size_t kCompressedCrcOffset = 24;
constexpr std::size_t kReservedOffset = 28;
constexpr std::size_t kOutputChunkSize = 64U * 1024U;
// One symbol can visit at most order + 1 contexts, and the donor range coder
// normalizes by at most two bytes per visit.  Order 8 therefore fits below
// 18 bytes per input byte; 20 leaves framing slack without an unbounded sink.
constexpr std::size_t kMaximumExpansionFactor = 20;
constexpr std::size_t kRangeCoderSlack = 64;

std::size_t checked_add(const std::size_t left,
                        const std::size_t right,
                        const char* message) {
    if (left > std::numeric_limits<std::size_t>::max() - right) {
        throw std::length_error(message);
    }
    return left + right;
}

std::size_t checked_multiply(const std::size_t left,
                             const std::size_t right,
                             const char* message) {
    if (left != 0 && right > std::numeric_limits<std::size_t>::max() / left) {
        throw std::length_error(message);
    }
    return left * right;
}

std::uint32_t crc32(const ByteView input) noexcept {
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

void write_u32_le(std::vector<std::uint8_t>& output,
                  const std::size_t offset,
                  const std::uint32_t value) noexcept {
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        output[offset + shift / 8U] =
            static_cast<std::uint8_t>(value >> shift);
    }
}

std::uint32_t read_u32_le(const ByteView input,
                          const std::size_t offset) noexcept {
    std::uint32_t value = 0;
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        value |= static_cast<std::uint32_t>(input[offset + shift / 8U])
                 << shift;
    }
    return value;
}

void require_valid_view(const ByteView input, const char* operation) {
    if (!input.empty() && input.data() == nullptr) {
        throw std::invalid_argument(std::string(operation) +
                                    " received a null byte view");
    }
}

std::size_t stream_capacity_for(const std::size_t input_size) {
    const std::size_t expanded = checked_multiply(
        input_size, kMaximumExpansionFactor, "PPMd7 stream bound overflow");
    return checked_add(expanded, kRangeCoderSlack,
                       "PPMd7 stream bound overflow");
}

struct ChunkedByteOut {
    IByteOut vt{};
    std::vector<std::unique_ptr<Byte[]>> chunks;
    std::size_t size = 0;
    std::size_t capacity = 0;
    bool failed = false;
};

void write_byte(IByteOutPtr stream, const Byte value) noexcept {
    auto* output = const_cast<ChunkedByteOut*>(
        reinterpret_cast<const ChunkedByteOut*>(stream));
    if (output->failed || output->size >= output->capacity) {
        output->failed = true;
        return;
    }
    const std::size_t chunk_index = output->size / kOutputChunkSize;
    const std::size_t chunk_offset = output->size % kOutputChunkSize;
    if (!output->chunks[chunk_index]) {
        output->chunks[chunk_index].reset(
            new (std::nothrow) Byte[kOutputChunkSize]);
        if (!output->chunks[chunk_index]) {
            output->failed = true;
            return;
        }
    }
    output->chunks[chunk_index][chunk_offset] = value;
    ++output->size;
}

ChunkedByteOut make_output(const std::size_t capacity) {
    ChunkedByteOut output{};
    output.vt.Write = write_byte;
    output.capacity = capacity;
    const std::size_t chunk_count =
        capacity / kOutputChunkSize +
        (capacity % kOutputChunkSize != 0 ? 1U : 0U);
    output.chunks.resize(chunk_count);
    return output;
}

void copy_output(const ChunkedByteOut& source,
                 std::vector<std::uint8_t>& destination,
                 const std::size_t destination_offset) {
    std::size_t copied = 0;
    while (copied < source.size) {
        const std::size_t chunk_index = copied / kOutputChunkSize;
        const std::size_t chunk_offset = copied % kOutputChunkSize;
        const std::size_t amount = std::min(
            source.size - copied, kOutputChunkSize - chunk_offset);
        std::copy_n(source.chunks[chunk_index].get() + chunk_offset,
                    amount, destination.data() + destination_offset + copied);
        copied += amount;
    }
}

struct BoundedByteIn {
    IByteIn vt{};
    const Byte* data = nullptr;
    std::size_t size = 0;
    std::size_t position = 0;
    bool exhausted = false;
};

Byte read_byte(IByteInPtr stream) noexcept {
    auto* input = const_cast<BoundedByteIn*>(
        reinterpret_cast<const BoundedByteIn*>(stream));
    if (input->position >= input->size) {
        input->exhausted = true;
        return 0;
    }
    return input->data[input->position++];
}

class Ppmd7Model {
public:
    explicit Ppmd7Model(const std::uint32_t memory_size) {
        Ppmd7_Construct(&model_);
        if (!Ppmd7_Alloc(&model_, memory_size, &g_Alloc)) {
            throw std::bad_alloc();
        }
    }

    ~Ppmd7Model() { Ppmd7_Free(&model_, &g_Alloc); }

    Ppmd7Model(const Ppmd7Model&) = delete;
    Ppmd7Model& operator=(const Ppmd7Model&) = delete;

    CPpmd7* get() noexcept { return &model_; }

private:
    CPpmd7 model_{};
};

void validate_profile(const unsigned order,
                      const std::uint32_t memory_size) {
    if (order < PPMD7_MIN_ORDER || order > PPMD7_MAX_ORDER) {
        throw std::invalid_argument("PPMd7 order must be 2 through 64");
    }
    if (memory_size < PPMD7_MIN_MEM_SIZE ||
        memory_size > Ppmd7Backend::kMaximumMemorySize) {
        throw std::invalid_argument("PPMd7 model memory is outside limits");
    }
}

}  // namespace

Ppmd7Backend::Ppmd7Backend(const unsigned order,
                           const std::uint32_t memory_size,
                           const std::size_t maximum_output_size)
    : order_(order),
      memory_size_(memory_size),
      maximum_output_size_(maximum_output_size) {
    validate_profile(order_, memory_size_);
    if (maximum_output_size_ > std::numeric_limits<std::uint32_t>::max()) {
        throw std::invalid_argument("PPMd7 output limit exceeds payload format");
    }
}

std::vector<std::uint8_t> Ppmd7Backend::encode(const ByteView input) const {
    require_valid_view(input, "PPMd7 encoder");
    if (input.size() > maximum_output_size_ ||
        input.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("PPMd7 input exceeds configured block limit");
    }

    Ppmd7Model model(memory_size_);
    ChunkedByteOut stream = make_output(stream_capacity_for(input.size()));
    model.get()->rc.enc.Stream = &stream.vt;
    Ppmd7z_Init_RangeEnc(model.get());
    Ppmd7_Init(model.get(), order_);
    const Byte* const begin = input.data();
    Ppmd7z_EncodeSymbols(model.get(), begin, begin + input.size());
    Ppmd7z_Flush_RangeEnc(model.get());
    if (stream.failed) {
        throw std::length_error("PPMd7 range stream exceeded safety bound");
    }
    if (stream.size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("PPMd7 range stream exceeds payload format");
    }

    std::vector<std::uint8_t> output(
        checked_add(kPayloadHeaderSize, stream.size,
                    "PPMd7 payload size overflow"));
    std::copy(kPayloadMagic.begin(), kPayloadMagic.end(), output.begin());
    output[4] = kPayloadVersion;
    output[5] = kPayloadFlags;
    output[6] = static_cast<std::uint8_t>(order_);
    output[7] = static_cast<std::uint8_t>(kPayloadHeaderSize);
    write_u32_le(output, kMemorySizeOffset, memory_size_);
    write_u32_le(output, kUncompressedSizeOffset,
                 static_cast<std::uint32_t>(input.size()));
    write_u32_le(output, kCompressedSizeOffset,
                 static_cast<std::uint32_t>(stream.size));
    write_u32_le(output, kUncompressedCrcOffset, crc32(input));
    copy_output(stream, output, kPayloadHeaderSize);
    write_u32_le(output, kCompressedCrcOffset,
                 crc32(ByteView(output.data() + kPayloadHeaderSize,
                                stream.size)));
    return output;
}

std::vector<std::uint8_t> Ppmd7Backend::decode(
    const ByteView payload,
    const std::size_t expected_size) const {
    require_valid_view(payload, "PPMd7 decoder");
    if (payload.size() < kPayloadHeaderSize + 5U) {
        throw std::runtime_error("Truncated PPMd7 payload");
    }
    if (!std::equal(kPayloadMagic.begin(), kPayloadMagic.end(),
                    payload.data())) {
        throw std::runtime_error("Invalid PPMd7 payload magic");
    }
    if (payload[4] != kPayloadVersion || payload[5] != kPayloadFlags ||
        payload[7] != kPayloadHeaderSize) {
        throw std::runtime_error("Unsupported PPMd7 payload format");
    }
    for (std::size_t index = kReservedOffset;
         index < kPayloadHeaderSize; ++index) {
        if (payload[index] != 0) {
            throw std::runtime_error("Invalid PPMd7 payload reserved bytes");
        }
    }

    const unsigned order = payload[6];
    const std::uint32_t memory_size =
        read_u32_le(payload, kMemorySizeOffset);
    try {
        validate_profile(order, memory_size);
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Unsupported PPMd7 payload profile");
    }
    const std::size_t declared_size =
        read_u32_le(payload, kUncompressedSizeOffset);
    const std::size_t compressed_size =
        read_u32_le(payload, kCompressedSizeOffset);
    if (declared_size != expected_size) {
        throw std::runtime_error("PPMd7 output size does not match metadata");
    }
    if (declared_size > maximum_output_size_) {
        throw std::length_error("PPMd7 output exceeds configured block limit");
    }
    if (compressed_size != payload.size() - kPayloadHeaderSize) {
        throw std::runtime_error("PPMd7 stream size does not match payload");
    }
    const ByteView compressed(payload.data() + kPayloadHeaderSize,
                              compressed_size);
    if (crc32(compressed) != read_u32_le(payload, kCompressedCrcOffset)) {
        throw std::runtime_error("PPMd7 compressed payload checksum mismatch");
    }

    Ppmd7Model model(memory_size);
    BoundedByteIn stream{};
    stream.vt.Read = read_byte;
    stream.data = compressed.data();
    stream.size = compressed.size();
    model.get()->rc.dec.Stream = &stream.vt;
    if (!Ppmd7z_RangeDec_Init(&model.get()->rc.dec) || stream.exhausted) {
        throw std::runtime_error("Invalid PPMd7 range stream header");
    }
    Ppmd7_Init(model.get(), order);

    std::vector<std::uint8_t> output(declared_size);
    for (std::size_t index = 0; index < declared_size; ++index) {
        const int symbol = Ppmd7z_DecodeSymbol(model.get());
        if (stream.exhausted || symbol < 0 || symbol > 255) {
            throw std::runtime_error("PPMd7 range stream ended before output");
        }
        output[index] = static_cast<std::uint8_t>(symbol);
    }
    // The 7z PPMd7 stream is length-terminated and can retain at most four
    // bytes from its five-byte range-coder flush.  More indicates trailing
    // data inside the declared stream.
    if (stream.position > stream.size ||
        stream.size - stream.position > 4U) {
        throw std::runtime_error("PPMd7 range stream has trailing bytes");
    }
    if (crc32(ByteView(output)) !=
        read_u32_le(payload, kUncompressedCrcOffset)) {
        throw std::runtime_error("PPMd7 output checksum mismatch");
    }
    return output;
}

std::size_t Ppmd7Backend::maximum_payload_size(
    const std::size_t input_size) {
    return checked_add(kPayloadHeaderSize, stream_capacity_for(input_size),
                       "PPMd7 payload bound overflow");
}

}  // namespace hz::r2
