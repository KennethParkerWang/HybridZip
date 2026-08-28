#include "r2/entropy/paq8px_record_model_backend.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "r2/entropy/binary_arithmetic_codec.h"

#include "Mixer_Scalar.hpp"
#include "Shared.hpp"
#include "model/RecordModel.hpp"

namespace hz::r2 {
namespace {

constexpr std::uint32_t kProbabilityScale = 1U << 24U;
constexpr std::uint32_t kDonorPrecision = 31U;

std::uint32_t context_table_size(const std::size_t block_size) {
    // ContextMap stores one 64-byte bucket per table entry. It must have at
    // least two entries: a one-entry table yields hashBits == 0 and makes the
    // donor's 64-bit range reduction undefined.
    std::uint32_t size = 128U;
    while (size < block_size && size < (1U << 31U)) {
        size <<= 1U;
    }
    if (size < block_size) {
        throw std::length_error("PAQ8px RecordModel context table is too large");
    }
    return size;
}

class RecordModelSession {
public:
    explicit RecordModelSession(const std::size_t block_size) {
        if (block_size > std::numeric_limits<std::uint32_t>::max()) {
            throw std::length_error("PAQ8px RecordModel block exceeds 32-bit size");
        }
        shared_.init(0, 1U << 20U);
        shared_.State.blockType = BlockType::DEFAULT;
        shared_.State.blockInfo = -1;
        shared_.State.blockPos = std::numeric_limits<std::uint32_t>::max();
        shared_.State.Match.expectedByte = 256U;
        model_ = std::make_unique<RecordModel>(
            &shared_, context_table_size(block_size));
        mixer_ = std::make_unique<Mixer_Scalar>(
            &shared_, RecordModel::MIXERINPUTS, RecordModel::MIXERCONTEXTS,
            RecordModel::MIXERCONTEXTSETS, 0);
        mixer_->setScaleFactor(980, 90);
        model_->setParam(0);
    }

    std::uint32_t predict() {
        if (shared_.State.bitPosition == 0U) {
            ++shared_.State.blockPos;
        }
        model_->mix(*mixer_);
        const int p12 = std::clamp(mixer_->p(), 1, 4095);
        return static_cast<std::uint32_t>(p12) << 12U;
    }

    void observe(const std::uint8_t bit, const std::uint32_t p24) {
        if (bit > 1U || p24 == 0U || p24 >= kProbabilityScale) {
            throw std::invalid_argument("PAQ8px RecordModel received invalid bit state");
        }
        const std::uint32_t p31 = p24 << (kDonorPrecision - 24U);
        shared_.update(bit, p31, false);
    }

private:
    Shared shared_;
    std::unique_ptr<RecordModel> model_;
    std::unique_ptr<Mixer_Scalar> mixer_;
};

template <typename ReadBit>
std::uint8_t process_byte(RecordModelSession& session,
                          const std::size_t position,
                          ReadBit&& read_bit) {
    std::uint8_t value = 0;
    for (std::uint8_t bit_index = 0; bit_index < 8U; ++bit_index) {
        const std::uint32_t probability = session.predict();
        const std::uint8_t bit = read_bit(probability, bit_index);
        if (bit > 1U) {
            throw std::logic_error("PAQ8px RecordModel bit callback returned non-bit");
        }
        session.observe(bit, probability);
        value = static_cast<std::uint8_t>((value << 1U) | bit);
    }
    static_cast<void>(position);
    return value;
}

}  // namespace

std::size_t Paq8pxRecordModelBackend::maximum_payload_size(
    const std::size_t input_size) {
    if (input_size > (std::numeric_limits<std::size_t>::max() - 64U) / 4U) {
        throw std::length_error("PAQ8px RecordModel payload bound overflows size_t");
    }
    return input_size * 4U + 64U;
}

std::vector<std::uint8_t> Paq8pxRecordModelBackend::encode(
    const ByteView input) const {
    if (input.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("PAQ8px RecordModel input exceeds 32-bit block limit");
    }
    if (!input.empty() && input.data() == nullptr) {
        throw std::invalid_argument("PAQ8px RecordModel input has null data");
    }

    RecordModelSession session(input.size());
    std::ostringstream encoded(std::ios::out | std::ios::binary);
    BinaryArithmeticEncoderStream coder(encoded);
    for (std::size_t position = 0; position < input.size(); ++position) {
        std::uint8_t bit_index = 0;
        const std::uint8_t reconstructed = process_byte(
            session, position,
            [&](const std::uint32_t probability,
                const std::uint8_t index) -> std::uint8_t {
                bit_index = index;
                const std::uint8_t bit = static_cast<std::uint8_t>(
                    (input[position] >> (7U - bit_index)) & 1U);
                coder.write_bit(probability, kProbabilityScale, bit);
                return bit;
            });
        if (reconstructed != input[position]) {
            throw std::logic_error("PAQ8px RecordModel encoder reconstructed wrong byte");
        }
    }
    coder.finish();
    const std::string bytes = encoded.str();
    if (bytes.size() > maximum_payload_size(input.size())) {
        throw std::runtime_error("PAQ8px RecordModel payload exceeded safe bound");
    }
    return {reinterpret_cast<const std::uint8_t*>(bytes.data()),
            reinterpret_cast<const std::uint8_t*>(bytes.data()) + bytes.size()};
}

std::vector<std::uint8_t> Paq8pxRecordModelBackend::decode(
    const ByteView payload, const std::size_t expected_size) const {
    if (expected_size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("PAQ8px RecordModel output exceeds 32-bit block limit");
    }
    if (payload.size() > maximum_payload_size(expected_size) ||
        (expected_size != 0U && payload.empty())) {
        throw std::invalid_argument("PAQ8px RecordModel payload violates size contract");
    }
    if (!payload.empty() && payload.data() == nullptr) {
        throw std::invalid_argument("PAQ8px RecordModel payload has null data");
    }

    const std::string bytes = payload.empty()
        ? std::string{}
        : std::string(reinterpret_cast<const char*>(payload.data()), payload.size());
    std::istringstream encoded(bytes, std::ios::in | std::ios::binary);
    BinaryArithmeticDecoderStream coder(encoded);
    RecordModelSession session(expected_size);
    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    for (std::size_t position = 0; position < expected_size; ++position) {
        output.push_back(process_byte(
            session, position,
            [&](const std::uint32_t probability,
                const std::uint8_t) -> std::uint8_t {
                return coder.read_bit(probability, kProbabilityScale);
            }));
    }
    return output;
}

}  // namespace hz::r2
