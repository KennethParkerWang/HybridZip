#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "core/byte_history.h"
#include "core/profile.h"
#include "predictors/match_predictor.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void observe(hz::MatchPredictor& predictor,
             hz::ByteHistory& history,
             const std::uint8_t actual) {
    hz::ProbVector ignored{};
    predictor.predict(history, ignored);
    predictor.update(actual, history);
    history.push(actual);
}

void feed(hz::MatchPredictor& predictor,
          hz::ByteHistory& history,
          const std::string_view bytes) {
    for (const unsigned char byte : bytes) {
        observe(predictor, history, byte);
    }
}

void test_match_diagnostics_lifecycle() {
    const hz::Profile profile = hz::make_profile_v1();
    hz::MatchPredictor predictor(profile);
    hz::ByteHistory history(1024);

    require(predictor.last_diagnostics().status ==
                hz::MatchDiagnosticStatus::not_predicted,
            "Match diagnostics did not reset");

    hz::ProbVector probability{};
    predictor.predict(history, probability);
    const hz::MatchDiagnostics cold = predictor.last_diagnostics();
    require(cold.status == hz::MatchDiagnosticStatus::insufficient_history &&
                cold.history_position == 0 && cold.candidate_count == 0 &&
                !cold.prediction_active,
            "Cold Match diagnostics are wrong");

    predictor.update('A', history);
    history.push('A');
    feed(predictor, history, "abcdefghXBabcdefgh");

    predictor.predict(history, probability);
    const hz::MatchDiagnostics first = predictor.last_diagnostics();
    require(first.status == hz::MatchDiagnosticStatus::prediction_active &&
                first.prediction_active && first.candidate_count == 1,
            "Active Match candidate was not diagnosed");
    require(first.candidate_symbol == 'X' &&
                first.best_match_length >= profile.match_context_bytes,
            "Match candidate identity or length is wrong");
    require(first.confidence_bucket == first.best_match_length &&
                first.confidence_hits == 0 &&
                first.confidence_trials == 0,
            "Initial Match confidence diagnostics are wrong");
    require(std::abs(first.candidate_probability - 0.5) < 1e-12,
            "Initial Match probability diagnostics are wrong");

    predictor.update('X', history);
    history.push('X');
    feed(predictor, history, "Cabcdefgh");
    predictor.predict(history, probability);
    const hz::MatchDiagnostics learned = predictor.last_diagnostics();
    require(learned.status == hz::MatchDiagnosticStatus::prediction_active &&
                learned.candidate_symbol == 'X' &&
                learned.confidence_hits == 1 &&
                learned.confidence_trials == 1,
            "Learned Match confidence diagnostics are wrong");
    require(learned.candidate_probability > first.candidate_probability,
            "Match diagnostics did not expose learned confidence");

    predictor.reset(profile.model_seed);
    require(predictor.last_diagnostics().status ==
                hz::MatchDiagnosticStatus::not_predicted,
            "Reset retained stale Match diagnostics");
}

}  // namespace

int main() {
    try {
        test_match_diagnostics_lifecycle();
        std::cout << "match_diagnostics_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "match_diagnostics_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
