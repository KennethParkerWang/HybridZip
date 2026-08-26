#include "r2/representation/bcj2_transform.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

#include "Bcj2.h"

namespace hz::r2 {
namespace {

constexpr std::size_t kStreamCount = BCJ2_NUM_STREAMS;
constexpr std::size_t kStreamSlack = 16;

void append_u32_le(std::vector<std::uint8_t>& output,
                   const std::uint32_t value) {
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        output.push_back(static_cast<std::uint8_t>(value >> shift));
    }
}

std::uint32_t read_u32_le(const ByteView input, const std::size_t offset) {
    return static_cast<std::uint32_t>(input[offset]) |
           (static_cast<std::uint32_t>(input[offset + 1U]) << 8U) |
           (static_cast<std::uint32_t>(input[offset + 2U]) << 16U) |
           (static_cast<std::uint32_t>(input[offset + 3U]) << 24U);
}

std::size_t checked_capacity(const std::size_t input_size) {
    if (input_size > std::numeric_limits<std::size_t>::max() - kStreamSlack) {
        throw std::overflow_error("BCJ2 stream capacity overflow");
    }
    return input_size + kStreamSlack;
}

}  // namespace

TransformResult Bcj2Transform::forward(const ByteView input) const {
    if (input.empty() || input.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::invalid_argument("BCJ2 requires a nonempty 32-bit block");
    }
    const std::size_t capacity = checked_capacity(input.size());
    std::array<std::vector<std::uint8_t>, kStreamCount> streams;
    CBcj2Enc encoder{};
    for (std::size_t stream = 0; stream < kStreamCount; ++stream) {
        streams[stream].resize(capacity);
        encoder.bufs[stream] = streams[stream].data();
        encoder.lims[stream] = streams[stream].data() + streams[stream].size();
    }
    encoder.src = input.data();
    encoder.srcLim = input.data() + input.size();
    Bcj2Enc_Init(&encoder);
    Bcj2Enc_SET_FileSize(&encoder, input.size());
    encoder.finishMode = BCJ2_ENC_FINISH_MODE_END_STREAM;

    for (unsigned calls = 0; !Bcj2Enc_IsFinished(&encoder); ++calls) {
        if (calls == 32U) {
            throw std::runtime_error("BCJ2 encoder did not finish");
        }
        Bcj2Enc_Encode(&encoder);
        if (encoder.state < BCJ2_NUM_STREAMS) {
            throw std::runtime_error("BCJ2 stream capacity is insufficient");
        }
    }
    if (encoder.src != encoder.srcLim || encoder.state != BCJ2_ENC_STATE_FINISHED) {
        throw std::runtime_error("BCJ2 encoder did not consume its input");
    }

    std::array<std::size_t, kStreamCount> lengths{};
    std::size_t total = 0;
    for (std::size_t stream = 0; stream < kStreamCount; ++stream) {
        lengths[stream] = static_cast<std::size_t>(encoder.bufs[stream] - streams[stream].data());
        if (lengths[stream] > std::numeric_limits<std::uint32_t>::max() ||
            total > std::numeric_limits<std::size_t>::max() - lengths[stream]) {
            throw std::runtime_error("BCJ2 stream length is invalid");
        }
        total += lengths[stream];
    }
    if (lengths[BCJ2_STREAM_MAIN] + lengths[BCJ2_STREAM_CALL] +
            lengths[BCJ2_STREAM_JUMP] != input.size()) {
        throw std::runtime_error("BCJ2 stream split does not reconstruct input size");
    }

    TransformResult result{};
    result.bytes.reserve(total);
    result.side_information.reserve(kSideInformationSize);
    for (std::size_t stream = 0; stream < kStreamCount; ++stream) {
        append_u32_le(result.side_information,
                      static_cast<std::uint32_t>(lengths[stream]));
        result.bytes.insert(result.bytes.end(), streams[stream].begin(),
                            streams[stream].begin() + lengths[stream]);
    }
    return result;
}

std::vector<std::uint8_t> Bcj2Transform::inverse(
    const ByteView transformed, const ByteView side_information,
    const std::size_t expected_size) const {
    if (side_information.size() != kSideInformationSize || expected_size == 0U) {
        throw std::runtime_error("BCJ2 metadata is malformed");
    }
    std::array<std::size_t, kStreamCount> lengths{};
    std::size_t total = 0;
    for (std::size_t stream = 0; stream < kStreamCount; ++stream) {
        lengths[stream] = read_u32_le(side_information, stream * 4U);
        if (total > std::numeric_limits<std::size_t>::max() - lengths[stream]) {
            throw std::runtime_error("BCJ2 metadata length overflows");
        }
        total += lengths[stream];
    }
    if (total != transformed.size() ||
        lengths[BCJ2_STREAM_MAIN] + lengths[BCJ2_STREAM_CALL] +
            lengths[BCJ2_STREAM_JUMP] != expected_size ||
        lengths[BCJ2_STREAM_CALL] % 4U != 0U ||
        lengths[BCJ2_STREAM_JUMP] % 4U != 0U ||
        lengths[BCJ2_STREAM_RC] < 5U) {
        throw std::runtime_error("BCJ2 stream framing is invalid");
    }

    CBcj2Dec decoder{};
    const std::uint8_t* cursor = transformed.data();
    for (std::size_t stream = 0; stream < kStreamCount; ++stream) {
        decoder.bufs[stream] = cursor;
        decoder.lims[stream] = cursor + lengths[stream];
        cursor += lengths[stream];
    }
    std::vector<std::uint8_t> output(expected_size);
    decoder.dest = output.data();
    decoder.destLim = output.data() + output.size();
    Bcj2Dec_Init(&decoder);
    for (unsigned calls = 0; decoder.dest != decoder.destLim; ++calls) {
        if (calls == 32U) throw std::runtime_error("BCJ2 decoder did not finish");
        const std::uint8_t* const before = decoder.dest;
        if (Bcj2Dec_Decode(&decoder) != SZ_OK) {
            throw std::runtime_error("BCJ2 decoder rejected its stream");
        }
        if (decoder.dest == before) throw std::runtime_error("BCJ2 decoder stalled");
    }
    for (std::size_t stream = 0; stream < kStreamCount; ++stream) {
        if (decoder.bufs[stream] != decoder.lims[stream]) {
            throw std::runtime_error("BCJ2 decoder left unconsumed stream data");
        }
    }
    if (!Bcj2Dec_IsMaybeFinished(&decoder)) {
        throw std::runtime_error("BCJ2 decoder did not reach a final state");
    }
    return output;
}

}  // namespace hz::r2
