#include "r2/entropy/neural_lstm_backend.h"

#include <sstream>
#include <string>

#include "core/byte_history.h"
#include "core/cdf.h"
#include "core/profile.h"
#include "entropy/arithmetic_codec.h"
#include "predictors/online_lstm_predictor.h"

namespace hz::r2 {
namespace {

std::vector<std::uint8_t> as_bytes(const std::string& value) {
    return {reinterpret_cast<const std::uint8_t*>(value.data()),
            reinterpret_cast<const std::uint8_t*>(value.data()) + value.size()};
}

}  // namespace

std::vector<std::uint8_t> NeuralLstmBackend::encode(const ByteView input) const {
    const hz::Profile profile = hz::make_profile_v1();
    hz::OnlineLstmPredictor predictor(profile);
    hz::ByteHistory history(profile.history_capacity);
    predictor.reset(model_seed_);

    std::ostringstream encoded(std::ios::out | std::ios::binary);
    {
        hz::ArithmeticEncoderStream coder(encoded);
        for (std::size_t index = 0; index < input.size(); ++index) {
            hz::ProbVector probabilities{};
            predictor.predict(history, probabilities);
            const hz::Cdf cdf = hz::quantize_to_cdf(probabilities);
            coder.write_symbol(cdf, input[index]);
            predictor.update(input[index], history);
            history.push(input[index]);
        }
        coder.finish();
    }
    return as_bytes(encoded.str());
}

std::vector<std::uint8_t> NeuralLstmBackend::decode(
    const ByteView payload, const std::size_t expected_size) const {
    const std::string bytes(reinterpret_cast<const char*>(payload.data()),
                            payload.size());
    std::istringstream encoded(bytes, std::ios::in | std::ios::binary);
    const hz::Profile profile = hz::make_profile_v1();
    hz::OnlineLstmPredictor predictor(profile);
    hz::ByteHistory history(profile.history_capacity);
    predictor.reset(model_seed_);
    hz::ArithmeticDecoderStream coder(encoded);

    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    for (std::size_t index = 0; index < expected_size; ++index) {
        hz::ProbVector probabilities{};
        predictor.predict(history, probabilities);
        const hz::Cdf cdf = hz::quantize_to_cdf(probabilities);
        const std::uint8_t symbol = coder.read_symbol(cdf);
        output.push_back(symbol);
        predictor.update(symbol, history);
        history.push(symbol);
    }
    return output;
}

}  // namespace hz::r2
