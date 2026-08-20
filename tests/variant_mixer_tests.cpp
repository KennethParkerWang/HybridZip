#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "analysis/variant_mixer.h"
#include "core/probability.h"
#include "mixer/adaptive_linear_mixer.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void require_close(const double left,
                   const double right,
                   const double tolerance,
                   const char* message) {
    if (std::abs(left - right) > tolerance) {
        throw std::runtime_error(message);
    }
}

template <typename Function>
void require_invalid_argument(Function function, const char* message) {
    try {
        function();
    } catch (const std::invalid_argument&) {
        return;
    }
    throw std::runtime_error(message);
}

hz::ProbVector biased_probability(const std::uint8_t symbol,
                                  const double probability) {
    hz::ProbVector result{};
    const double remainder =
        (1.0 - probability) / static_cast<double>(hz::kAlphabet - 1U);
    result.fill(remainder);
    result[symbol] = probability;
    hz::normalize_probability(result);
    return result;
}

std::vector<hz::ProbVector> four_experts() {
    return std::vector<hz::ProbVector>{
        biased_probability(42, 0.90), biased_probability(42, 0.10),
        biased_probability(7, 0.80), biased_probability(99, 0.70)};
}

void require_normalized(const hz::ProbVector& probability,
                        const char* message) {
    double total = 0.0;
    for (const double value : probability) {
        require(std::isfinite(value) && value > 0.0, message);
        total += value;
    }
    require_close(total, 1.0, 1e-12, message);
}

void test_active_equal_mixer() {
    const std::vector<hz::ProbVector> experts = four_experts();
    const hz::ExpertMask mask = (hz::ExpertMask{1} << 0U) |
                                (hz::ExpertMask{1} << 2U);
    const hz::ActiveEqualMixer pair(4, mask);
    require_close(pair.weights()[0], 0.5, 0.0,
                  "Equal pair weight is wrong");
    require_close(pair.weights()[1], 0.0, 0.0,
                  "Inactive equal weight is not zero");
    require_close(pair.weights()[2], 0.5, 0.0,
                  "Equal pair weight is wrong");

    hz::ProbVector mixed{};
    pair.mix(experts, mixed);
    require_normalized(mixed, "Equal pair mixture is invalid");
    for (std::size_t symbol = 0; symbol < hz::kAlphabet; ++symbol) {
        require_close(mixed[symbol],
                      0.5 * (experts[0][symbol] + experts[2][symbol]),
                      1e-14, "Equal pair mixture is wrong");
    }

    const hz::ActiveEqualMixer only(4, hz::ExpertMask{1} << 1U);
    only.mix(experts, mixed);
    for (std::size_t symbol = 0; symbol < hz::kAlphabet; ++symbol) {
        require_close(mixed[symbol], experts[1][symbol], 1e-14,
                      "Only-expert mixture changed its posterior");
    }

    require_invalid_argument([] { hz::ActiveEqualMixer invalid(4, 0); },
                             "Empty active mask was accepted");
    require_invalid_argument(
        [] { hz::ActiveEqualMixer invalid(4, hz::ExpertMask{1} << 4U); },
        "Out-of-range active mask was accepted");
}

void test_discounted_hedge_and_v1_equivalence() {
    std::vector<hz::ProbVector> experts{
        biased_probability(42, 0.90), biased_probability(42, 0.10)};
    hz::AdaptiveLinearMixer v1(2, 0.5);
    hz::DiscountedHedgeMixer hedge(
        2, 0x3U, hz::HedgeScaleConfig{0.5, 1.0});

    for (std::size_t step = 0; step < 20; ++step) {
        for (std::size_t expert = 0; expert < 2; ++expert) {
            require_close(hedge.weights()[expert], v1.weights()[expert],
                          1e-12,
                          "Undiscounted Hedge diverged from V1 update");
        }
        v1.update(42, experts);
        hedge.update(42, experts);
    }
    require(hedge.weights()[0] > hedge.weights()[1],
            "Hedge did not reward the accurate expert");

    hz::DiscountedHedgeMixer recent(
        2, 0x3U, hz::HedgeScaleConfig{0.5, 0.5});
    for (std::size_t step = 0; step < 16; ++step) {
        recent.update(42, experts);
    }
    require(recent.weights()[0] > recent.weights()[1],
            "Discounted Hedge did not learn the first regime");

    experts[0] = biased_probability(7, 0.10);
    experts[1] = biased_probability(7, 0.90);
    for (std::size_t step = 0; step < 8; ++step) {
        recent.update(7, experts);
    }
    require(recent.weights()[1] > recent.weights()[0],
            "Discounted Hedge did not recover in a new regime");

    require_invalid_argument(
        [] {
            hz::DiscountedHedgeMixer invalid(
                2, 0x3U, hz::HedgeScaleConfig{0.5, 0.0});
        },
        "Zero Hedge discount was accepted");
    require_close(hz::discount_from_half_life(16.0),
                  std::pow(0.5, 1.0 / 16.0), 1e-15,
                  "Hedge half-life conversion is wrong");
}

void test_multi_timescale_hedge() {
    hz::MultiTimescaleHedgeMixer mixer(
        2, 0x3U,
        std::vector<hz::HedgeScaleConfig>{
            hz::HedgeScaleConfig{0.5, 1.0},
            hz::HedgeScaleConfig{0.5, 0.5}});
    std::vector<hz::ProbVector> first_regime{
        biased_probability(42, 0.99), biased_probability(42, 0.01)};
    for (std::size_t step = 0; step < 16; ++step) {
        mixer.update(42, first_regime);
    }

    std::vector<hz::ProbVector> second_regime{
        biased_probability(7, 0.01), biased_probability(7, 0.99)};
    for (std::size_t step = 0; step < 4; ++step) {
        mixer.update(7, second_regime);
    }

    require(mixer.scales().size() == 2,
            "Multi-timescale Hedge scale count is wrong");
    require(mixer.scales()[0].weights()[0] >
                mixer.scales()[0].weights()[1],
            "Long-term Hedge forgot its accumulated regime too quickly");
    require(mixer.scales()[1].weights()[1] >
                mixer.scales()[1].weights()[0],
            "Recent Hedge did not switch regimes");
    for (std::size_t expert = 0; expert < 2; ++expert) {
        const double expected =
            0.5 * (mixer.scales()[0].weights()[expert] +
                   mixer.scales()[1].weights()[expert]);
        require_close(mixer.weights()[expert], expected, 1e-15,
                      "Multi-timescale weights are not the scale mean");
    }

    hz::ProbVector mixed{};
    mixer.mix(second_regime, mixed);
    require_normalized(mixed, "Multi-timescale mixture is invalid");
    mixer.reset();
    require_close(mixer.weights()[0], 0.5, 0.0,
                  "Multi-timescale reset weight is wrong");
    require_close(mixer.weights()[1], 0.5, 0.0,
                  "Multi-timescale reset weight is wrong");
}

}  // namespace

int main() {
    try {
        test_active_equal_mixer();
        test_discounted_hedge_and_v1_equivalence();
        test_multi_timescale_hedge();
        std::cout << "variant_mixer_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "variant_mixer_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
