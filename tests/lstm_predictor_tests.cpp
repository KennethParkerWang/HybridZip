#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "core/byte_history.h"
#include "predictors/online_lstm_predictor.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

}  // namespace

int main() {
    try {
        const hz::OnlineLstmConfig config{hz::kAlphabet, 4, 2, 2, 0.03, 10.0};
        hz::OnlineLstmPredictor first(config);
        hz::OnlineLstmPredictor second(config);
        hz::ByteHistory first_history(32);
        hz::ByteHistory second_history(32);
        first.reset(123456789);
        second.reset(123456789);

        for (const std::uint8_t actual : {0U, 1U, 42U, 255U, 42U}) {
            hz::ProbVector left{};
            hz::ProbVector right{};
            first.predict(first_history, left);
            second.predict(second_history, right);
            require(left == right, "seeded LSTM predictions diverged");
            double sum = 0.0;
            for (const double value : left) {
                require(std::isfinite(value) && value > 0.0,
                        "LSTM probability is invalid");
                sum += value;
            }
            require(std::abs(sum - 1.0) < 1e-12,
                    "LSTM probability sum is not one");
            first.update(actual, first_history);
            second.update(actual, second_history);
            first_history.push(actual);
            second_history.push(actual);
        }

        std::cout << "lstm_predictor_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "lstm_predictor_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
