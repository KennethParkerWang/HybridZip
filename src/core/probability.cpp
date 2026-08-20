#include "core/probability.h"

#include <cmath>

namespace hz {

void set_uniform_probability(ProbVector& probability) {
    probability.fill(1.0 / static_cast<double>(kAlphabet));
}

void normalize_probability(ProbVector& probability) {
    double sum = 0.0;
    for (double& value : probability) {
        if (!std::isfinite(value)) {
            value = 0.0;
        }
        value = value < kProbFloor ? kProbFloor : value;
        sum += value;
    }

    if (!std::isfinite(sum) || sum <= 0.0) {
        set_uniform_probability(probability);
        return;
    }

    for (double& value : probability) {
        value /= sum;
    }
}

}  // namespace hz
