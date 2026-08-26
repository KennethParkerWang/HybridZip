#include "r2/entropy/wavpack_backend.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "wavpack.h"

namespace hz::r2 {
namespace {

constexpr std::array<std::uint8_t, 4> kPayloadMagic{'H', 'Z', 'W', '1'};
constexpr std::uint8_t kPayloadVersion = 1;
constexpr std::size_t kStreamSizeOffset = 8;
constexpr std::size_t kReservedOffset = 12;
constexpr std::size_t kDecodeFrames = 8192;

struct PcmProfile {
    std::uint8_t bytes_per_sample;
    std::uint8_t channels;
};

// Favor normal PCM layouts on ties. The 8-bit mono profile guarantees that
// every non-empty HZ02 block still has a forced-mode representation.
constexpr std::array<PcmProfile, 8> kProfiles{{
    {2, 2}, {2, 1}, {3, 2}, {3, 1},
    {4, 2}, {4, 1}, {1, 2}, {1, 1},
}};

struct OutputBuffer {
    std::vector<std::uint8_t> bytes;
    std::size_t limit = 0;
    bool failed = false;
};

struct InputBuffer {
    const std::uint8_t* data = nullptr;
    std::size_t size = 0;
    std::size_t position = 0;
};

struct WavpackCloser {
    void operator()(WavpackContext* context) const noexcept {
        if (context != nullptr) {
            WavpackCloseFile(context);
        }
    }
};

using WavpackHandle = std::unique_ptr<WavpackContext, WavpackCloser>;

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

void require_valid_view(const ByteView input, const char* operation) {
    if (!input.empty() && input.data() == nullptr) {
        throw std::invalid_argument(std::string(operation) +
                                    " received a null byte view");
    }
}

void append_u32_le(std::vector<std::uint8_t>& output,
                   const std::uint32_t value) {
    output.push_back(static_cast<std::uint8_t>(value));
    output.push_back(static_cast<std::uint8_t>(value >> 8U));
    output.push_back(static_cast<std::uint8_t>(value >> 16U));
    output.push_back(static_cast<std::uint8_t>(value >> 24U));
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

std::int32_t load_sample(const std::uint8_t* source,
                         const std::uint8_t width) noexcept {
    std::uint32_t bits = 0;
    for (std::uint8_t byte = 0; byte < width; ++byte) {
        bits |= static_cast<std::uint32_t>(source[byte]) << (byte * 8U);
    }
    if (width < 4U && (source[width - 1U] & 0x80U) != 0U) {
        bits |= std::numeric_limits<std::uint32_t>::max() << (width * 8U);
    }
    std::int32_t sample = 0;
    static_assert(sizeof(sample) == sizeof(bits), "WavPack sample width");
    std::memcpy(&sample, &bits, sizeof(sample));
    return sample;
}

void append_sample(std::vector<std::uint8_t>& output,
                   const std::int32_t sample,
                   const std::uint8_t width) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &sample, sizeof(bits));
    for (std::uint8_t byte = 0; byte < width; ++byte) {
        output.push_back(static_cast<std::uint8_t>(bits >> (byte * 8U)));
    }
}

int write_block(void* identifier, void* data, const std::int32_t count) {
    auto* output = static_cast<OutputBuffer*>(identifier);
    if (output == nullptr || data == nullptr || count <= 0) {
        return 0;
    }
    const std::size_t size = static_cast<std::size_t>(count);
    if (size > output->limit - std::min(output->limit, output->bytes.size())) {
        output->failed = true;
        return 0;
    }
    try {
        const auto* first = static_cast<const std::uint8_t*>(data);
        output->bytes.insert(output->bytes.end(), first, first + size);
        return 1;
    } catch (...) {
        output->failed = true;
        return 0;
    }
}

std::int32_t read_bytes(void* identifier, void* data, std::int32_t count) {
    auto* input = static_cast<InputBuffer*>(identifier);
    if (input == nullptr || data == nullptr || count <= 0) {
        return 0;
    }
    const std::size_t available = input->size - input->position;
    const std::size_t requested = static_cast<std::size_t>(count);
    const std::size_t copied = std::min(available, requested);
    if (copied != 0U) {
        std::memcpy(data, input->data + input->position, copied);
        input->position += copied;
    }
    return static_cast<std::int32_t>(copied);
}

std::int32_t write_bytes(void*, void*, std::int32_t) { return 0; }

std::int64_t get_position(void* identifier) {
    const auto* input = static_cast<const InputBuffer*>(identifier);
    return input == nullptr ? -1 : static_cast<std::int64_t>(input->position);
}

int set_position_absolute(void* identifier, const std::int64_t position) {
    auto* input = static_cast<InputBuffer*>(identifier);
    if (input == nullptr || position < 0 ||
        static_cast<std::uint64_t>(position) > input->size) {
        return -1;
    }
    input->position = static_cast<std::size_t>(position);
    return 0;
}

int set_position_relative(void* identifier,
                          const std::int64_t delta,
                          const int mode) {
    auto* input = static_cast<InputBuffer*>(identifier);
    if (input == nullptr) {
        return -1;
    }
    std::size_t base = 0;
    if (mode == SEEK_CUR) {
        base = input->position;
    } else if (mode == SEEK_END) {
        base = input->size;
    } else if (mode != SEEK_SET) {
        return -1;
    }

    if (delta >= 0) {
        const std::uint64_t amount = static_cast<std::uint64_t>(delta);
        if (amount > input->size - base) {
            return -1;
        }
        input->position = base + static_cast<std::size_t>(amount);
    } else {
        const std::uint64_t amount = 0U - static_cast<std::uint64_t>(delta);
        if (amount > base) {
            return -1;
        }
        input->position = base - static_cast<std::size_t>(amount);
    }
    return 0;
}

int push_back_byte(void* identifier, const int value) {
    auto* input = static_cast<InputBuffer*>(identifier);
    if (input == nullptr || input->position == 0U) {
        return EOF;
    }
    --input->position;
    return value;
}

std::int64_t get_length(void* identifier) {
    const auto* input = static_cast<const InputBuffer*>(identifier);
    return input == nullptr ? -1 : static_cast<std::int64_t>(input->size);
}

int can_seek(void*) { return 1; }
int truncate_here(void*) { return -1; }
int close_input(void*) { return 0; }

WavpackStreamReader64 memory_reader() {
    WavpackStreamReader64 reader{};
    reader.read_bytes = read_bytes;
    reader.write_bytes = write_bytes;
    reader.get_pos = get_position;
    reader.set_pos_abs = set_position_absolute;
    reader.set_pos_rel = set_position_relative;
    reader.push_back_byte = push_back_byte;
    reader.get_length = get_length;
    reader.can_seek = can_seek;
    reader.truncate_here = truncate_here;
    reader.close = close_input;
    return reader;
}

std::string donor_error(WavpackContext* context, const char* fallback) {
    if (context != nullptr) {
        const char* message = WavpackGetErrorMessage(context);
        if (message != nullptr && *message != '\0') {
            return std::string("WavPack: ") + message;
        }
    }
    return std::string("WavPack: ") + fallback;
}

std::vector<std::uint8_t> encode_profile(const ByteView input,
                                         const PcmProfile profile,
                                         const std::size_t stream_limit) {
    const std::size_t frame_bytes =
        static_cast<std::size_t>(profile.bytes_per_sample) * profile.channels;
    const std::size_t frame_count = input.size() / frame_bytes;
    if (frame_count == 0U ||
        frame_count > std::numeric_limits<std::uint32_t>::max()) {
        return {};
    }

    const std::size_t sample_count = checked_multiply(
        frame_count, profile.channels, "WavPack sample count overflow");
    std::vector<std::int32_t> samples(sample_count);
    const std::uint8_t* source = input.data();
    for (std::size_t index = 0; index < sample_count; ++index) {
        samples[index] = load_sample(source, profile.bytes_per_sample);
        source += profile.bytes_per_sample;
    }

    OutputBuffer output{};
    output.limit = stream_limit;
    output.bytes.reserve(std::min(stream_limit, input.size() + 4096U));
    WavpackHandle context(WavpackOpenFileOutput(write_block, &output, nullptr));
    if (!context) {
        throw std::runtime_error("WavPack: failed to allocate encoder");
    }

    WavpackConfig config{};
    config.bits_per_sample = profile.bytes_per_sample * 8;
    config.bytes_per_sample = profile.bytes_per_sample;
    config.num_channels = profile.channels;
    config.channel_mask = profile.channels == 1U ? 4 : 3;
    config.sample_rate = 44100;
    config.qmode = QMODE_RAW_PCM | QMODE_SIGNED_BYTES;
    config.flags = CONFIG_HIGH_FLAG | CONFIG_OPTIMIZE_MONO;
    if (profile.bytes_per_sample == 4U) {
        config.flags |= CONFIG_OPTIMIZE_32BIT;
    }
    if (!WavpackSetConfiguration64(context.get(), &config,
                                   static_cast<std::int64_t>(frame_count),
                                   nullptr) ||
        !WavpackPackInit(context.get()) ||
        !WavpackPackSamples(context.get(), samples.data(),
                            static_cast<std::uint32_t>(frame_count)) ||
        !WavpackFlushSamples(context.get()) || output.failed) {
        throw std::runtime_error(donor_error(context.get(),
                                             "lossless encode failed"));
    }
    return output.bytes;
}

}  // namespace

std::size_t WavpackBackend::maximum_payload_size(
    const std::size_t input_size) {
    return checked_add(
        checked_multiply(input_size, 4U, "WavPack payload bound overflow"),
        4096U + kPayloadHeaderSize,
        "WavPack payload bound overflow");
}

std::vector<std::uint8_t> WavpackBackend::encode(const ByteView input) const {
    require_valid_view(input, "WavPack encoder");
    if (input.empty()) {
        throw std::invalid_argument("WavPack does not encode an empty block");
    }
    if (input.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("WavPack input exceeds payload format");
    }

    const std::size_t payload_limit = maximum_payload_size(input.size());
    std::vector<std::uint8_t> best_stream;
    PcmProfile best_profile{};
    std::size_t best_tail = 0;
    for (const PcmProfile profile : kProfiles) {
        const std::size_t frame_bytes =
            static_cast<std::size_t>(profile.bytes_per_sample) *
            profile.channels;
        const std::size_t tail = input.size() % frame_bytes;
        const std::size_t stream_limit =
            payload_limit - kPayloadHeaderSize - tail;
        std::vector<std::uint8_t> stream =
            encode_profile(input, profile, stream_limit);
        if (!stream.empty() &&
            (best_stream.empty() || stream.size() + tail <
                                      best_stream.size() + best_tail)) {
            best_stream = std::move(stream);
            best_profile = profile;
            best_tail = tail;
        }
    }
    if (best_stream.empty() ||
        best_stream.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("WavPack produced no bounded lossless profile");
    }

    std::vector<std::uint8_t> output;
    output.reserve(kPayloadHeaderSize + best_stream.size() + best_tail);
    output.insert(output.end(), kPayloadMagic.begin(), kPayloadMagic.end());
    output.push_back(kPayloadVersion);
    output.push_back(best_profile.bytes_per_sample);
    output.push_back(best_profile.channels);
    output.push_back(static_cast<std::uint8_t>(best_tail));
    append_u32_le(output, static_cast<std::uint32_t>(best_stream.size()));
    append_u32_le(output, 0U);
    output.insert(output.end(), best_stream.begin(), best_stream.end());
    if (best_tail != 0U) {
        output.insert(output.end(), input.data() + input.size() - best_tail,
                      input.data() + input.size());
    }
    if (output.size() > payload_limit) {
        throw std::runtime_error("WavPack payload exceeded bound");
    }
    return output;
}

std::vector<std::uint8_t> WavpackBackend::decode(
    const ByteView payload,
    const std::size_t expected_size) const {
    require_valid_view(payload, "WavPack decoder");
    if (expected_size == 0U || payload.size() < kPayloadHeaderSize + 32U ||
        payload.size() > maximum_payload_size(expected_size)) {
        throw std::runtime_error("Invalid WavPack payload size");
    }
    if (!std::equal(kPayloadMagic.begin(), kPayloadMagic.end(),
                    payload.data()) || payload[4] != kPayloadVersion) {
        throw std::runtime_error("Unsupported WavPack payload format");
    }
    const std::uint8_t bytes_per_sample = payload[5];
    const std::uint8_t channels = payload[6];
    const std::size_t tail_size = payload[7];
    const std::size_t stream_size = read_u32_le(payload, kStreamSizeOffset);
    for (std::size_t index = kReservedOffset;
         index < kPayloadHeaderSize; ++index) {
        if (payload[index] != 0U) {
            throw std::runtime_error("Invalid WavPack reserved payload bytes");
        }
    }
    if (bytes_per_sample < 1U || bytes_per_sample > 4U ||
        (channels != 1U && channels != 2U)) {
        throw std::runtime_error("Invalid WavPack PCM profile");
    }
    const std::size_t frame_bytes =
        static_cast<std::size_t>(bytes_per_sample) * channels;
    if (tail_size >= frame_bytes || tail_size > expected_size ||
        stream_size < 32U ||
        stream_size != payload.size() - kPayloadHeaderSize - tail_size) {
        throw std::runtime_error("Invalid WavPack stream framing");
    }
    const std::size_t decoded_bytes = expected_size - tail_size;
    if (decoded_bytes == 0U || decoded_bytes % frame_bytes != 0U) {
        throw std::runtime_error("WavPack frame count does not match output");
    }
    const std::size_t expected_frames = decoded_bytes / frame_bytes;

    InputBuffer input{payload.data() + kPayloadHeaderSize, stream_size, 0U};
    WavpackStreamReader64 reader = memory_reader();
    std::array<char, 80> error{};
    WavpackHandle context(WavpackOpenFileInputEx64(
        &reader, &input, nullptr, error.data(), 0, 0));
    if (!context) {
        throw std::runtime_error(std::string("WavPack: ") + error.data());
    }
    if ((WavpackGetMode(context.get()) & MODE_LOSSLESS) == 0 ||
        WavpackLossyBlocks(context.get()) != 0 ||
        WavpackGetBytesPerSample(context.get()) != bytes_per_sample ||
        WavpackGetBitsPerSample(context.get()) != bytes_per_sample * 8 ||
        WavpackGetNumChannels(context.get()) != channels ||
        WavpackGetNumSamples64(context.get()) !=
            static_cast<std::int64_t>(expected_frames)) {
        throw std::runtime_error("WavPack stream profile mismatch");
    }

    const std::size_t buffer_frames = std::min(kDecodeFrames, expected_frames);
    std::vector<std::int32_t> samples(buffer_frames * channels);
    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    std::size_t restored_frames = 0;
    while (restored_frames < expected_frames) {
        const std::uint32_t requested = static_cast<std::uint32_t>(
            std::min(buffer_frames, expected_frames - restored_frames));
        const std::uint32_t restored =
            WavpackUnpackSamples(context.get(), samples.data(), requested);
        if (restored == 0U || restored > requested) {
            throw std::runtime_error(donor_error(context.get(),
                                                 "truncated lossless stream"));
        }
        for (std::size_t index = 0;
             index < static_cast<std::size_t>(restored) * channels; ++index) {
            append_sample(output, samples[index], bytes_per_sample);
        }
        restored_frames += restored;
    }
    if (WavpackUnpackSamples(context.get(), samples.data(), 1U) != 0U ||
        WavpackGetNumErrors(context.get()) != 0) {
        throw std::runtime_error("WavPack stream contains extra or bad samples");
    }

    if (tail_size != 0U) {
        output.insert(output.end(), payload.data() + payload.size() - tail_size,
                      payload.data() + payload.size());
    }
    if (output.size() != expected_size) {
        throw std::runtime_error("WavPack decoded size mismatch");
    }
    return output;
}

}  // namespace hz::r2
