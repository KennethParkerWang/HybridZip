#include "r2/entropy/paq8px_linear_prediction_backend.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "r2/entropy/binary_arithmetic_codec.h"

#include "Mixer_Scalar.hpp"
#include "Shared.hpp"
#include "model/LinearPredictionModel.hpp"

namespace hz::r2 {
namespace {

constexpr std::uint32_t kProbabilityScale = 1U << 24U;
constexpr std::uint32_t kDonorPrecision = 31U;

class LinearPredictionSession {
public:
    explicit LinearPredictionSession(const std::size_t block_size) {
        if (block_size > std::numeric_limits<std::uint32_t>::max()) {
            throw std::length_error(
                "PAQ8px LinearPredictionModel block exceeds 32-bit size");
        }
        shared_.init(0, 1U << 20U);
        shared_.chosenSimd = SIMDType::SIMD_NONE;
        shared_.State.blockType = BlockType::DEFAULT;
        shared_.State.blockInfo = -1;
        shared_.State.blockPos = std::numeric_limits<std::uint32_t>::max();
        shared_.State.Match.expectedByte = 256U;
        model_ = std::make_unique<LinearPredictionModel>(&shared_);
        mixer_ = std::make_unique<Mixer_Scalar>(
            &shared_, 1 + LinearPredictionModel::MIXERINPUTS, 1, 1, 0);
        mixer_->setScaleFactor(980, 90);
    }

    std::uint32_t predict() {
        if (shared_.State.bitPosition == 0U) {
            ++shared_.State.blockPos;
        }
        mixer_->add(256);
        model_->mix(*mixer_);
        mixer_->set(0, 1);
        const int p12 = std::clamp(mixer_->p(), 1, 4095);
        return static_cast<std::uint32_t>(p12) << 12U;
    }

    void observe(const std::uint8_t bit, const std::uint32_t p24) {
        if (bit > 1U || p24 == 0U || p24 >= kProbabilityScale) {
            throw std::invalid_argument(
                "PAQ8px LinearPredictionModel received invalid bit state");
        }
        const std::uint32_t p31 = p24 << (kDonorPrecision - 24U);
        shared_.update(bit, p31, false);
    }

private:
    Shared shared_;
    std::unique_ptr<LinearPredictionModel> model_;
    std::unique_ptr<Mixer_Scalar> mixer_;
};

template <typename ReadBit>
std::uint8_t process_byte(LinearPredictionSession& session,
                          ReadBit&& read_bit) {
    std::uint8_t value = 0;
    for (std::uint8_t bit_index = 0; bit_index < 8U; ++bit_index) {
        const std::uint32_t probability = session.predict();
        const std::uint8_t bit = read_bit(probability, bit_index);
        if (bit > 1U) {
            throw std::logic_error(
                "PAQ8px LinearPredictionModel callback returned non-bit");
        }
        session.observe(bit, probability);
        value = static_cast<std::uint8_t>((value << 1U) | bit);
    }
    return value;
}

}  // namespace

std::size_t Paq8pxLinearPredictionBackend::maximum_payload_size(
    const std::size_t input_size) {
    if (input_size > (std::numeric_limits<std::size_t>::max() - 64U) / 4U) {
        throw std::length_error(
            "PAQ8px LinearPredictionModel payload bound overflows size_t");
    }
    return input_size * 4U + 64U;
}

std::vector<std::uint8_t> Paq8pxLinearPredictionBackend::encode(
    const ByteView input) const {
    if (input.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error(
            "PAQ8px LinearPredictionModel input exceeds 32-bit block limit");
    }
    if (!input.empty() && input.data() == nullptr) {
        throw std::invalid_argument(
            "PAQ8px LinearPredictionModel input has null data");
    }

    LinearPredictionSession session(input.size());
    std::ostringstream encoded(std::ios::out | std::ios::binary);
    BinaryArithmeticEncoderStream coder(encoded);
    for (std::size_t position = 0; position < input.size(); ++position) {
        const std::uint8_t reconstructed = process_byte(
            session,
            [&](const std::uint32_t probability,
                const std::uint8_t bit_index) -> std::uint8_t {
                const std::uint8_t bit = static_cast<std::uint8_t>(
                    (input[position] >> (7U - bit_index)) & 1U);
                coder.write_bit(probability, kProbabilityScale, bit);
                return bit;
            });
        if (reconstructed != input[position]) {
            throw std::logic_error(
                "PAQ8px LinearPredictionModel encoder reconstructed wrong byte");
        }
    }
    coder.finish();
    const std::string bytes = encoded.str();
    if (bytes.size() > maximum_payload_size(input.size())) {
        throw std::runtime_error(
            "PAQ8px LinearPredictionModel payload exceeded safe bound");
    }
    return {reinterpret_cast<const std::uint8_t*>(bytes.data()),
            reinterpret_cast<const std::uint8_t*>(bytes.data()) + bytes.size()};
}

std::vector<std::uint8_t> Paq8pxLinearPredictionBackend::decode(
    const ByteView payload, const std::size_t expected_size) const {
    if (expected_size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error(
            "PAQ8px LinearPredictionModel output exceeds 32-bit block limit");
    }
    if (payload.size() > maximum_payload_size(expected_size) ||
        (expected_size != 0U && payload.empty())) {
        throw std::invalid_argument(
            "PAQ8px LinearPredictionModel payload violates size contract");
    }
    if (!payload.empty() && payload.data() == nullptr) {
        throw std::invalid_argument(
            "PAQ8px LinearPredictionModel payload has null data");
    }

    const std::string bytes = payload.empty()
        ? std::string{}
        : std::string(reinterpret_cast<const char*>(payload.data()),
                      payload.size());
    std::istringstream encoded(bytes, std::ios::in | std::ios::binary);
    BinaryArithmeticDecoderStream coder(encoded);
    LinearPredictionSession session(expected_size);
    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    for (std::size_t position = 0; position < expected_size; ++position) {
        static_cast<void>(position);
        output.push_back(process_byte(
            session,
            [&](const std::uint32_t probability,
                const std::uint8_t) -> std::uint8_t {
                return coder.read_bit(probability, kProbabilityScale);
            }));
    }
    return output;
}

}  // namespace hz::r2
