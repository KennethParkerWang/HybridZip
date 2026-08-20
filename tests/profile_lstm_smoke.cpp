#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "core/byte_history.h"
#include "core/profile.h"
#include "predictors/online_lstm_predictor.h"

int main() {
    try {
        const auto start = std::chrono::steady_clock::now();
        const hz::Profile profile = hz::make_profile_v1();
        hz::OnlineLstmPredictor predictor(profile);
        hz::ByteHistory history(profile.history_capacity);
        const auto constructed = std::chrono::steady_clock::now();

        hz::ProbVector probability{};
        predictor.predict(history, probability);
        predictor.update(42, history);
        history.push(42);
        predictor.predict(history, probability);
        const auto updated = std::chrono::steady_clock::now();

        double sum = 0.0;
        for (const double value : probability) {
            if (!std::isfinite(value) || value <= 0.0) {
                throw std::runtime_error("PROFILE_V1 LSTM probability invalid");
            }
            sum += value;
        }
        if (std::abs(sum - 1.0) >= 1e-12) {
            throw std::runtime_error("PROFILE_V1 LSTM probability sum invalid");
        }

        const std::chrono::duration<double> construction = constructed - start;
        const std::chrono::duration<double> first_update = updated - constructed;
        std::cout << "construction_seconds\t" << construction.count() << '\n'
                  << "first_update_seconds\t" << first_update.count() << '\n'
                  << "status\tPASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "profile_lstm_smoke: FAIL: " << error.what() << '\n';
        return 1;
    }
}
