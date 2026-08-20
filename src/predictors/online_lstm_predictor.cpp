#include "predictors/online_lstm_predictor.h"

#include <limits>
#include <stdexcept>
#include <utility>
#include <valarray>

#include "core/byte_history.h"
#include "core/probability.h"
#include "lstm.h"

namespace hz {

OnlineLstmPredictor::OnlineLstmPredictor(const Profile& profile)
    : OnlineLstmPredictor(OnlineLstmConfig{
          profile.lstm_output_size,
          profile.lstm_cells,
          profile.lstm_layers,
          profile.lstm_horizon,
          profile.lstm_learning_rate,
          profile.lstm_gradient_clip}) {}

OnlineLstmPredictor::OnlineLstmPredictor(OnlineLstmConfig config)
    : config_(std::move(config)) {
    if (config_.output_size != kAlphabet || config_.cells == 0 ||
        config_.layers == 0 || config_.horizon == 0 ||
        config_.cells > std::numeric_limits<unsigned int>::max() ||
        config_.layers > std::numeric_limits<unsigned int>::max() ||
        config_.horizon > static_cast<std::size_t>(
                              std::numeric_limits<int>::max())) {
        throw std::invalid_argument("Invalid Online LSTM configuration");
    }
    reset(kDefaultModelSeed);
}

OnlineLstmPredictor::~OnlineLstmPredictor() = default;

void OnlineLstmPredictor::reset(const std::uint64_t seed) {
    model_ = std::make_unique<Lstm>(
        0U,
        static_cast<unsigned int>(config_.output_size),
        static_cast<unsigned int>(config_.cells),
        static_cast<unsigned int>(config_.layers),
        static_cast<int>(config_.horizon),
        static_cast<float>(config_.learning_rate),
        static_cast<float>(config_.gradient_clip),
        seed);
    set_uniform_probability(cached_);
    prediction_ready_ = false;
}

void OnlineLstmPredictor::predict(const ByteHistory&, ProbVector& out) {
    out = cached_;
    prediction_ready_ = true;
}

void OnlineLstmPredictor::update(const std::uint8_t actual,
                                 const ByteHistory&) {
    if (!prediction_ready_) {
        throw std::logic_error("Online LSTM update requires a prior prediction");
    }
    const std::valarray<float>& next = model_->Perceive(actual);
    if (next.size() != kAlphabet) {
        throw std::runtime_error("Online LSTM returned a non-byte distribution");
    }
    for (std::size_t symbol = 0; symbol < kAlphabet; ++symbol) {
        cached_[symbol] = static_cast<double>(next[symbol]);
    }
    normalize_probability(cached_);
    prediction_ready_ = false;
}

}  // namespace hz
