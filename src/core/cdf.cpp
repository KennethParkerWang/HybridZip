#include "core/cdf.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <stdexcept>

#include "core/probability.h"

namespace hz {
namespace {

struct FractionalCount {
    std::size_t symbol = 0;
    double fraction = 0.0;
};

}  // namespace

Cdf quantize_to_cdf(const ProbVector& probability) {
    ProbVector normalized = probability;
    normalize_probability(normalized);

    constexpr std::uint32_t remainder_total =
        kCdfTotal - static_cast<std::uint32_t>(kAlphabet);

    std::array<std::uint32_t, kAlphabet> frequencies{};
    std::array<FractionalCount, kAlphabet> fractional{};
    std::uint64_t allocated = 0;

    for (std::size_t symbol = 0; symbol < kAlphabet; ++symbol) {
        const double exact = normalized[symbol] * remainder_total;
        const auto base = static_cast<std::uint32_t>(std::floor(exact));
        frequencies[symbol] = 1U + base;
        fractional[symbol] = FractionalCount{symbol, exact - base};
        allocated += frequencies[symbol];
    }

    if (allocated > kCdfTotal) {
        throw std::runtime_error("CDF quantization allocated too many counts");
    }

    std::sort(fractional.begin(), fractional.end(),
              [](const FractionalCount& left, const FractionalCount& right) {
                  if (left.fraction == right.fraction) {
                      return left.symbol < right.symbol;
                  }
                  return left.fraction > right.fraction;
              });

    const auto missing = static_cast<std::size_t>(kCdfTotal - allocated);
    if (missing > kAlphabet) {
        throw std::runtime_error("CDF normalization left too many counts");
    }
    for (std::size_t index = 0; index < missing; ++index) {
        ++frequencies[fractional[index].symbol];
    }

    Cdf cdf{};
    for (std::size_t symbol = 0; symbol < kAlphabet; ++symbol) {
        cdf.v[symbol + 1] = cdf.v[symbol] + frequencies[symbol];
    }
    validate_cdf(cdf);
    return cdf;
}

void validate_cdf(const Cdf& cdf) {
    if (cdf.v.front() != 0U || cdf.v.back() != kCdfTotal) {
        throw std::invalid_argument("CDF endpoints do not match PROFILE_V1");
    }
    for (std::size_t index = 0; index < kAlphabet; ++index) {
        if (cdf.v[index + 1] <= cdf.v[index]) {
            throw std::invalid_argument("CDF contains a zero frequency");
        }
    }
}

}  // namespace hz
