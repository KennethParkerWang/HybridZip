#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/types.h"

namespace hz {

class AdaptiveLinearMixer {
public:
    AdaptiveLinearMixer(std::size_t experts, double eta);

    void reset();
    void mix(const std::vector<ProbVector>& input, ProbVector& output) const;
    void update(std::uint8_t actual, const std::vector<ProbVector>& input);

    const std::vector<double>& weights() const noexcept;

private:
    void validate_input(const std::vector<ProbVector>& input) const;

    double eta_;
    std::vector<double> log_weights_;
    std::vector<double> weights_;
};

}  // namespace hz
