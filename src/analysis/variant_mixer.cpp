#include "analysis/variant_mixer.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

#include "core/probability.h"

namespace hz {
namespace {

ExpertMask valid_mask_for(const std::size_t expert_count) {
    if (expert_count == 0 ||
        expert_count > std::numeric_limits<ExpertMask>::digits) {
        throw std::invalid_argument("Variant mixer expert count is invalid");
    }
    if (expert_count == std::numeric_limits<ExpertMask>::digits) {
        return std::numeric_limits<ExpertMask>::max();
    }
    return (ExpertMask{1} << expert_count) - ExpertMask{1};
}

void validate_active_mask(const std::size_t expert_count,
                          const ExpertMask active_mask) {
    const ExpertMask valid_mask = valid_mask_for(expert_count);
    if (active_mask == 0 || (active_mask & ~valid_mask) != 0) {
        throw std::invalid_argument("Variant mixer active mask is invalid");
    }
}

bool is_active(const ExpertMask mask, const std::size_t expert) noexcept {
    return (mask & (ExpertMask{1} << expert)) != 0;
}

void set_equal_active_weights(const ExpertMask active_mask,
                              std::vector<double>& weights) {
    std::size_t active_count = 0;
    for (std::size_t expert = 0; expert < weights.size(); ++expert) {
        if (is_active(active_mask, expert)) {
            ++active_count;
        }
    }
    const double equal_weight = 1.0 / static_cast<double>(active_count);
    for (std::size_t expert = 0; expert < weights.size(); ++expert) {
        weights[expert] =
            is_active(active_mask, expert) ? equal_weight : 0.0;
    }
}

void mix_with_weights(const std::vector<ProbVector>& input,
                      const std::vector<double>& weights,
                      ProbVector& output) {
    if (input.size() != weights.size()) {
        throw std::invalid_argument("Variant mixer expert count mismatch");
    }
    output.fill(0.0);
    for (std::size_t expert = 0; expert < input.size(); ++expert) {
        if (weights[expert] == 0.0) {
            continue;
        }
        for (std::size_t symbol = 0; symbol < kAlphabet; ++symbol) {
            output[symbol] += weights[expert] * input[expert][symbol];
        }
    }
    normalize_probability(output);
}

double finite_likelihood(const double probability) noexcept {
    if (!std::isfinite(probability) || probability < kProbFloor) {
        return kProbFloor;
    }
    return probability;
}

}  // namespace

double discount_from_half_life(const double half_life_bytes) {
    if (!std::isfinite(half_life_bytes) || half_life_bytes <= 0.0) {
        throw std::invalid_argument("Hedge half-life must be positive");
    }
    return std::exp(std::log(0.5) / half_life_bytes);
}

ActiveEqualMixer::ActiveEqualMixer(const std::size_t expert_count,
                                   const ExpertMask active_mask)
    : active_mask_(active_mask), weights_(expert_count) {
    validate_active_mask(expert_count, active_mask_);
    set_equal_active_weights(active_mask_, weights_);
}

void ActiveEqualMixer::mix(const std::vector<ProbVector>& input,
                           ProbVector& output) const {
    mix_with_weights(input, weights_, output);
}

const std::vector<double>& ActiveEqualMixer::weights() const noexcept {
    return weights_;
}

ExpertMask ActiveEqualMixer::active_mask() const noexcept {
    return active_mask_;
}

DiscountedHedgeMixer::DiscountedHedgeMixer(
    const std::size_t expert_count,
    const ExpertMask active_mask,
    const HedgeScaleConfig config)
    : active_mask_(active_mask),
      config_(config),
      cumulative_losses_nats_(expert_count),
      weights_(expert_count) {
    validate_active_mask(expert_count, active_mask_);
    if (!std::isfinite(config_.eta) || config_.eta <= 0.0 ||
        !std::isfinite(config_.discount) || config_.discount <= 0.0 ||
        config_.discount > 1.0) {
        throw std::invalid_argument("Discounted Hedge configuration is invalid");
    }
    reset();
}

void DiscountedHedgeMixer::reset() {
    std::fill(cumulative_losses_nats_.begin(),
              cumulative_losses_nats_.end(), 0.0);
    set_equal_active_weights(active_mask_, weights_);
}

void DiscountedHedgeMixer::mix(const std::vector<ProbVector>& input,
                               ProbVector& output) const {
    mix_with_weights(input, weights_, output);
}

void DiscountedHedgeMixer::update(
    const std::uint8_t actual,
    const std::vector<ProbVector>& input) {
    if (input.size() != weights_.size()) {
        throw std::invalid_argument("Variant mixer expert count mismatch");
    }
    for (std::size_t expert = 0; expert < input.size(); ++expert) {
        if (!is_active(active_mask_, expert)) {
            continue;
        }
        const double loss = -std::log(finite_likelihood(input[expert][actual]));
        cumulative_losses_nats_[expert] =
            config_.discount * cumulative_losses_nats_[expert] + loss;
    }
    recompute_weights();
}

const std::vector<double>& DiscountedHedgeMixer::weights() const noexcept {
    return weights_;
}

const std::vector<double>&
DiscountedHedgeMixer::cumulative_losses_nats() const noexcept {
    return cumulative_losses_nats_;
}

ExpertMask DiscountedHedgeMixer::active_mask() const noexcept {
    return active_mask_;
}

HedgeScaleConfig DiscountedHedgeMixer::config() const noexcept {
    return config_;
}

void DiscountedHedgeMixer::recompute_weights() {
    double minimum_loss = std::numeric_limits<double>::infinity();
    for (std::size_t expert = 0; expert < weights_.size(); ++expert) {
        if (is_active(active_mask_, expert)) {
            minimum_loss =
                std::min(minimum_loss, cumulative_losses_nats_[expert]);
        }
    }

    double total = 0.0;
    for (std::size_t expert = 0; expert < weights_.size(); ++expert) {
        if (is_active(active_mask_, expert)) {
            weights_[expert] = std::exp(
                -config_.eta *
                (cumulative_losses_nats_[expert] - minimum_loss));
            total += weights_[expert];
        } else {
            weights_[expert] = 0.0;
        }
    }
    for (double& weight : weights_) {
        weight /= total;
    }
}

MultiTimescaleHedgeMixer::MultiTimescaleHedgeMixer(
    const std::size_t expert_count,
    const ExpertMask active_mask,
    std::vector<HedgeScaleConfig> scales)
    : active_mask_(active_mask), weights_(expert_count) {
    validate_active_mask(expert_count, active_mask_);
    if (scales.empty()) {
        throw std::invalid_argument("Multi-timescale Hedge needs a scale");
    }
    scales_.reserve(scales.size());
    for (const HedgeScaleConfig config : scales) {
        scales_.emplace_back(expert_count, active_mask_, config);
    }
    recompute_combined_weights();
}

void MultiTimescaleHedgeMixer::reset() {
    for (DiscountedHedgeMixer& scale : scales_) {
        scale.reset();
    }
    recompute_combined_weights();
}

void MultiTimescaleHedgeMixer::mix(
    const std::vector<ProbVector>& input,
    ProbVector& output) const {
    mix_with_weights(input, weights_, output);
}

void MultiTimescaleHedgeMixer::update(
    const std::uint8_t actual,
    const std::vector<ProbVector>& input) {
    for (DiscountedHedgeMixer& scale : scales_) {
        scale.update(actual, input);
    }
    recompute_combined_weights();
}

const std::vector<double>& MultiTimescaleHedgeMixer::weights() const noexcept {
    return weights_;
}

const std::vector<DiscountedHedgeMixer>&
MultiTimescaleHedgeMixer::scales() const noexcept {
    return scales_;
}

ExpertMask MultiTimescaleHedgeMixer::active_mask() const noexcept {
    return active_mask_;
}

void MultiTimescaleHedgeMixer::recompute_combined_weights() {
    std::fill(weights_.begin(), weights_.end(), 0.0);
    const double scale_weight = 1.0 / static_cast<double>(scales_.size());
    for (const DiscountedHedgeMixer& scale : scales_) {
        const std::vector<double>& scale_weights = scale.weights();
        for (std::size_t expert = 0; expert < weights_.size(); ++expert) {
            weights_[expert] += scale_weight * scale_weights[expert];
        }
    }
}

}  // namespace hz
