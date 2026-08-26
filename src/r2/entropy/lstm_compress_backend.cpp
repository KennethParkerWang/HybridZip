#include "r2/entropy/lstm_compress_backend.h"

#include <sstream>
#include <stdexcept>
#include <string>

#include "core/byte_history.h"
#include "core/cdf.h"
#include "core/profile.h"
#include "entropy/arithmetic_codec.h"
#include "lstm_compress_donor_port.h"
#include "predictors/online_lstm_predictor.h"

namespace hz::r2 {

namespace {

hz::OnlineLstmConfig legacy_config() {
    return hz::OnlineLstmConfig{
        hz::kAlphabet, 90U, 3U, 10U, 0.05, 2.0};
}

std::vector<std::uint8_t> decode_legacy(const ByteView payload,
                                        const std::size_t expected_size,
                                        const std::uint64_t model_seed) {
    const std::string bytes(reinterpret_cast<const char*>(payload.data()),
                            payload.size());
    std::istringstream encoded(bytes, std::ios::in | std::ios::binary);
    hz::OnlineLstmPredictor predictor(legacy_config());
    predictor.reset(model_seed);
    hz::ByteHistory history(hz::make_profile_v1().history_capacity);
    hz::ArithmeticDecoderStream coder(encoded);
    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    for (std::size_t index = 0; index < expected_size; ++index) {
        hz::ProbVector probabilities{};
        predictor.predict(history, probabilities);
        const std::uint8_t symbol = coder.read_symbol(
            hz::quantize_to_cdf(probabilities));
        output.push_back(symbol);
        predictor.update(symbol, history);
        history.push(symbol);
    }
    return output;
}

}  // namespace
std::size_t LstmCompressBackend::maximum_payload_size(
    const std::size_t input_size) {
    return lstm_compress_donor::maximum_payload_size(input_size);
}

std::vector<std::uint8_t> LstmCompressBackend::encode(
    const ByteView input) const {
    (void)model_seed_;
    return lstm_compress_donor::encode(input);
}

std::vector<std::uint8_t> LstmCompressBackend::decode(
    const ByteView payload, const std::size_t expected_size) const {
    (void)model_seed_;
    if (payload.size() >= 4U && payload[0] == 'H' && payload[1] == 'L' &&
        payload[2] == 'C' && payload[3] == '1') {
        return lstm_compress_donor::decode(payload, expected_size);
    }
    return decode_legacy(payload, expected_size, model_seed_);
}

}  // namespace hz::r2
