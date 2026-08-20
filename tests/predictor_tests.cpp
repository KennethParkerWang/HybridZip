#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "core/byte_history.h"
#include "core/profile.h"
#include "predictors/match_predictor.h"
#include "predictors/ngram_predictor.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void observe(hz::Predictor& predictor,
             hz::ByteHistory& history,
             const std::uint8_t actual) {
    hz::ProbVector ignored{};
    predictor.predict(history, ignored);
    predictor.update(actual, history);
    history.push(actual);
}

void test_ngram_freeze_and_existing_update() {
    hz::Profile profile = hz::make_profile_v1();
    profile.ngram_max_order = 1;
    profile.ngram_memory_bytes = 4096U + 96U + 16U;
    hz::NGramPredictor predictor(profile);
    hz::ByteHistory history(64);

    observe(predictor, history, 'A');
    observe(predictor, history, 'B');  // Creates the only allowed context A.
    observe(predictor, history, 'C');  // Context B cannot be created.
    observe(predictor, history, 'A');

    hz::ProbVector before{};
    predictor.predict(history, before);
    predictor.update('B', history);  // Existing context A must still update.
    history.push('B');
    observe(predictor, history, 'A');

    hz::ProbVector after{};
    predictor.predict(history, after);
    require(after['B'] > before['B'],
            "NGram frozen context did not continue learning");

    double sum = 0.0;
    for (const double value : after) {
        sum += value;
    }
    require(std::abs(sum - 1.0) < 1e-12,
            "NGram distribution is not normalized");
}

void feed(hz::MatchPredictor& predictor,
          hz::ByteHistory& history,
          const std::string_view bytes) {
    for (const unsigned char byte : bytes) {
        observe(predictor, history, byte);
    }
}

void test_match_learning_and_reset() {
    hz::Profile profile = hz::make_profile_v1();
    hz::MatchPredictor predictor(profile);
    hz::ByteHistory history(1024);

    hz::ProbVector initial{};
    predictor.predict(history, initial);
    require(std::abs(initial[0] - 1.0 / hz::kAlphabet) < 1e-15,
            "Match cold prediction is not uniform");

    feed(predictor, history, "AabcdefghXBabcdefgh");
    hz::ProbVector first_match{};
    predictor.predict(history, first_match);
    require(std::abs(first_match['X'] - 0.5) < 1e-12,
            "Match first candidate probability is not Laplace-smoothed");
    predictor.update('X', history);
    history.push('X');

    feed(predictor, history, "Cabcdefgh");
    hz::ProbVector learned{};
    predictor.predict(history, learned);
    require(learned['X'] > first_match['X'],
            "Match confidence did not learn a successful candidate");

    predictor.reset(profile.model_seed);
    history.reset();
    hz::ProbVector reset{};
    predictor.predict(history, reset);
    require(std::abs(reset['X'] - 1.0 / hz::kAlphabet) < 1e-15,
            "Match reset did not clear learned state");
}

}  // namespace

int main() {
    try {
        test_ngram_freeze_and_existing_update();
        test_match_learning_and_reset();
        std::cout << "predictor_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "predictor_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
