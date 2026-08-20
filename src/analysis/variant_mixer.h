#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/types.h"

namespace hz {

using ExpertMask = std::uint64_t;

struct HedgeScaleConfig {
    double eta = 0.5;
    double discount = 1.0;
};

double discount_from_half_life(double half_life_bytes);

class ActiveEqualMixer {
public:
    ActiveEqualMixer(std::size_t expert_count, ExpertMask active_mask);

    void mix(const std::vector<ProbVector>& input, ProbVector& output) const;
    const std::vector<double>& weights() const noexcept;
    ExpertMask active_mask() const noexcept;

private:
    ExpertMask active_mask_;
    std::vector<double> weights_;
};

class DiscountedHedgeMixer {
public:
    DiscountedHedgeMixer(std::size_t expert_count,
                         ExpertMask active_mask,
                         HedgeScaleConfig config);

    void reset();
    void mix(const std::vector<ProbVector>& input, ProbVector& output) const;
    void update(std::uint8_t actual, const std::vector<ProbVector>& input);

    const std::vector<double>& weights() const noexcept;
    const std::vector<double>& cumulative_losses_nats() const noexcept;
    ExpertMask active_mask() const noexcept;
    HedgeScaleConfig config() const noexcept;

private:
    void recompute_weights();

    ExpertMask active_mask_;
    HedgeScaleConfig config_;
    std::vector<double> cumulative_losses_nats_;
    std::vector<double> weights_;
};

class MultiTimescaleHedgeMixer {
public:
    MultiTimescaleHedgeMixer(std::size_t expert_count,
                             ExpertMask active_mask,
                             std::vector<HedgeScaleConfig> scales);

    void reset();
    void mix(const std::vector<ProbVector>& input, ProbVector& output) const;
    void update(std::uint8_t actual, const std::vector<ProbVector>& input);

    const std::vector<double>& weights() const noexcept;
    const std::vector<DiscountedHedgeMixer>& scales() const noexcept;
    ExpertMask active_mask() const noexcept;

private:
    void recompute_combined_weights();

    ExpertMask active_mask_;
    std::vector<DiscountedHedgeMixer> scales_;
    std::vector<double> weights_;
};

}  // namespace hz
