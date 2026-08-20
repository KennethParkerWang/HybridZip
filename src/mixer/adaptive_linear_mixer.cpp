#include "mixer/adaptive_linear_mixer.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "core/probability.h"

namespace hz {

AdaptiveLinearMixer::AdaptiveLinearMixer(const std::size_t experts,
                                         const double eta)
    : eta_(eta), log_weights_(experts), weights_(experts) {
    if (experts == 0 || !std::isfinite(eta) || eta <= 0.0) {
        throw std::invalid_argument("Invalid adaptive mixer configuration");
    }
    reset();
}

void AdaptiveLinearMixer::reset() {
    const double weight = 1.0 / static_cast<double>(weights_.size());
    const double log_weight = std::log(weight);
    std::fill(weights_.begin(), weights_.end(), weight);
    std::fill(log_weights_.begin(), log_weights_.end(), log_weight);
}

void AdaptiveLinearMixer::mix(const std::vector<ProbVector>& input,
                              ProbVector& output) const {
    validate_input(input);
    output.fill(0.0);
    for (std::size_t expert = 0; expert < input.size(); ++expert) {
        for (std::size_t symbol = 0; symbol < kAlphabet; ++symbol) {
            output[symbol] += weights_[expert] * input[expert][symbol];
        }
    }
    normalize_probability(output);
}

void AdaptiveLinearMixer::update(const std::uint8_t actual,
                                 const std::vector<ProbVector>& input) {
    validate_input(input);
    for (std::size_t expert = 0; expert < input.size(); ++expert) {
        const double likelihood =
            std::max(input[expert][actual], kProbFloor);
        log_weights_[expert] += eta_ * std::log(likelihood);
    }

    const double maximum =
        *std::max_element(log_weights_.begin(), log_weights_.end());
    double sum = 0.0;
    for (std::size_t expert = 0; expert < log_weights_.size(); ++expert) {
        weights_[expert] = std::exp(log_weights_[expert] - maximum);
        sum += weights_[expert];
    }
    for (std::size_t expert = 0; expert < log_weights_.size(); ++expert) {
        weights_[expert] /= sum;
        log_weights_[expert] = std::log(weights_[expert]);
    }
}

const std::vector<double>& AdaptiveLinearMixer::weights() const noexcept {
    return weights_;
}

void AdaptiveLinearMixer::validate_input(
    const std::vector<ProbVector>& input) const {
    if (input.size() != weights_.size()) {
        throw std::invalid_argument("Mixer expert count mismatch");
    }
}

}  // namespace hz
