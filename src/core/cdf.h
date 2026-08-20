#pragma once

#include "core/types.h"

namespace hz {

Cdf quantize_to_cdf(const ProbVector& probability);
void validate_cdf(const Cdf& cdf);

}  // namespace hz
