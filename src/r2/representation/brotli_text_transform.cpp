#include "r2/representation/brotli_text_transform.h"

#include <cstddef>
#include <limits>
#include <stdexcept>

#include <brotli/decode.h>
#include <brotli/encode.h>

namespace hz::r2 {
namespace {

constexpr int kQuality = 11;
constexpr int kWindowBits = BROTLI_DEFAULT_WINDOW;

void destroy_decoder(BrotliDecoderState* const state) noexcept {
    if (state != nullptr) {
        BrotliDecoderDestroyInstance(state);
    }
}

}  // namespace

bool BrotliTextTransform::applicable(const ByteView input) const noexcept {
    return !input.empty();
}

TransformResult BrotliTextTransform::forward(const ByteView input) const {
    if (!applicable(input)) {
        throw std::invalid_argument("Brotli text requires a non-empty block");
    }
    const std::size_t donor_bound = BrotliEncoderMaxCompressedSize(input.size());
    const std::size_t payload_bound = maximum_payload_size(input.size());
    if (donor_bound == 0 || donor_bound > payload_bound) {
        throw std::runtime_error("Brotli payload bound is unavailable");
    }

    TransformResult result{};
    result.bytes.resize(donor_bound);
    std::size_t encoded_size = result.bytes.size();
    if (BrotliEncoderCompress(kQuality, kWindowBits, BROTLI_MODE_TEXT,
                              input.size(), input.data(), &encoded_size,
                              result.bytes.data()) == BROTLI_FALSE ||
        encoded_size == 0 || encoded_size > payload_bound) {
        throw std::runtime_error("Brotli text encoding failed");
    }
    result.bytes.resize(encoded_size);
    return result;
}

std::vector<std::uint8_t> BrotliTextTransform::inverse(
    const ByteView payload, const std::size_t expected_size) const {
    if (payload.empty() || expected_size == 0 ||
        payload.size() > maximum_payload_size(expected_size)) {
        throw std::runtime_error("Brotli text payload violates HZ02 bounds");
    }

    BrotliDecoderState* const state =
        BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
    if (state == nullptr) {
        throw std::runtime_error("Brotli text decoder allocation failed");
    }

    try {
        std::vector<std::uint8_t> decoded(expected_size);
        const std::uint8_t* next_input = payload.data();
        std::size_t available_input = payload.size();
        std::uint8_t* next_output = decoded.data();
        std::size_t available_output = decoded.size();
        std::size_t total_output = 0;

        while (true) {
            const BrotliDecoderResult status = BrotliDecoderDecompressStream(
                state, &available_input, &next_input, &available_output,
                &next_output, &total_output);
            if (status == BROTLI_DECODER_RESULT_SUCCESS) {
                if (available_input != 0 || total_output != expected_size) {
                    throw std::runtime_error(
                        "Brotli text stream has trailing input or wrong output size");
                }
                destroy_decoder(state);
                return decoded;
            }
            if (status == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT) {
                throw std::runtime_error("Brotli text stream is truncated");
            }
            if (status == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
                throw std::runtime_error("Brotli text stream exceeds declared output size");
            }
            throw std::runtime_error("Brotli text decoding failed");
        }
    } catch (...) {
        destroy_decoder(state);
        throw;
    }
}

std::size_t BrotliTextTransform::maximum_payload_size(
    const std::size_t input_size) {
    constexpr std::size_t kOverhead = 4096U;
    const std::size_t available =
        std::numeric_limits<std::size_t>::max() - kOverhead;
    if (input_size > available ||
        input_size > available - input_size / 8U) {
        throw std::runtime_error("Brotli text payload bound overflow");
    }
    return input_size + input_size / 8U + kOverhead;
}

}  // namespace hz::r2
