#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "core/byte_history.h"
#include "core/profile.h"
#include "predictors/ppmd_predictor.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void validate(const hz::ProbVector& probability) {
    double sum = 0.0;
    for (const double value : probability) {
        require(std::isfinite(value) && value > 0.0,
                "PPMd probability is invalid");
        sum += value;
    }
    require(std::abs(sum - 1.0) < 1e-12,
            "PPMd probability sum is not one");
}

}  // namespace

int main() {
    try {
        const hz::Profile profile = hz::make_profile_v1();
        hz::PpmdPredictor first(profile);
        hz::PpmdPredictor second(profile);
        hz::ByteHistory first_history(1024);
        hz::ByteHistory second_history(1024);

        hz::ProbVector initial{};
        first.predict(first_history, initial);
        validate(initial);
        hz::ProbVector repeated{};
        first.predict(first_history, repeated);
        require(initial == repeated, "PPMd prediction query changed state");

        constexpr std::string_view sequence =
            "abracadabra abracadabra abracadabra ";
        for (int repeat = 0; repeat < 12; ++repeat) {
            for (const unsigned char actual : sequence) {
                hz::ProbVector left{};
                hz::ProbVector right{};
                first.predict(first_history, left);
                second.predict(second_history, right);
                require(left == right, "PPMd instances diverged");
                first.update(actual, first_history);
                second.update(actual, second_history);
                first_history.push(actual);
                second_history.push(actual);
            }
        }

        hz::ProbVector learned{};
        first.predict(first_history, learned);
        validate(learned);
        require(learned['a'] != initial['a'], "PPMd did not learn the sequence");

        first.reset(profile.model_seed);
        first_history.reset();
        hz::ProbVector reset{};
        first.predict(first_history, reset);
        require(reset == initial, "PPMd reset did not reproduce initial state");

        std::cout << "ppmd_predictor_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "ppmd_predictor_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
