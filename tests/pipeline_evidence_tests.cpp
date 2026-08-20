#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "analysis/pipeline_evidence.h"
#include "codec/model_pipeline.h"
#include "core/profile.h"
#include "entropy/arithmetic_codec.h"

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
void require_logic_error(Function function, const char* message) {
    try {
        function();
    } catch (const std::logic_error&) {
        return;
    }
    throw std::runtime_error(message);
}

struct CapturedEvidence {
    std::uint64_t position = 0;
    std::uint8_t actual = 0;
    std::array<double, 4> expert_actual_probabilities{};
    std::array<double, 4> weights{};
    double mixed_actual_probability = 0.0;
    std::uint32_t coding_frequency = 0;
    hz::MatchDiagnostics match{};
};

class RecordingSink final : public hz::PipelineEvidenceSink {
public:
    void on_byte(const hz::PipelineByteEvidence& evidence) override {
        require(evidence.expert_probabilities.size() == 4,
                "Evidence expert count is not four");
        require(evidence.mixer_weights.size() == 4,
                "Evidence mixer weight count is not four");

        CapturedEvidence captured{};
        captured.position = evidence.position;
        captured.actual = evidence.actual;
        for (std::size_t expert = 0; expert < 4; ++expert) {
            captured.expert_actual_probabilities[expert] =
                evidence.expert_probabilities[expert][evidence.actual];
            captured.weights[expert] = evidence.mixer_weights[expert];
        }
        captured.mixed_actual_probability =
            evidence.mixed_probability[evidence.actual];
        captured.coding_frequency =
            evidence.coding_cdf.v[evidence.actual + 1U] -
            evidence.coding_cdf.v[evidence.actual];
        captured.match = evidence.match;
        captured_.push_back(captured);
    }

    const std::vector<CapturedEvidence>& captured() const noexcept {
        return captured_;
    }

private:
    std::vector<CapturedEvidence> captured_;
};

void validate_distribution(const hz::ProbVector& probability,
                           const char* message) {
    double total = 0.0;
    for (const double value : probability) {
        require(std::isfinite(value) && value > 0.0, message);
        total += value;
    }
    require_close(total, 1.0, 1e-12, message);
}

void test_evidence_preserves_v1_cdf_and_payload() {
    const hz::Profile profile = hz::make_profile_v1();
    hz::ModelPipeline baseline(profile);
    hz::ModelPipeline instrumented(profile);
    RecordingSink sink;
    instrumented.set_evidence_sink(&sink);

    require_logic_error([&] { instrumented.current_prediction(); },
                        "Evidence was available before prediction");

    std::stringstream baseline_payload(
        std::ios::in | std::ios::out | std::ios::binary);
    std::stringstream instrumented_payload(
        std::ios::in | std::ios::out | std::ios::binary);
    hz::ArithmeticEncoderStream baseline_coder(baseline_payload);
    hz::ArithmeticEncoderStream instrumented_coder(instrumented_payload);

    constexpr std::string_view sequence =
        "AabcdefghXBabcdefghXCabcdefghX";
    std::uint64_t position = 0;
    for (const unsigned char actual : sequence) {
        const hz::Cdf& baseline_cdf = baseline.predict_cdf();
        const hz::Cdf& instrumented_cdf = instrumented.predict_cdf();
        require(baseline_cdf.v == instrumented_cdf.v,
                "Evidence changed the V1 CDF");

        const hz::PipelinePredictionView view =
            instrumented.current_prediction();
        require(view.position == position,
                "Prediction evidence position is wrong");
        require(view.expert_probabilities.size() == 4,
                "Prediction evidence expert count is wrong");
        require(view.mixer_weights.size() == 4,
                "Prediction evidence weight count is wrong");
        require(view.coding_cdf.v == instrumented_cdf.v,
                "Prediction evidence CDF is wrong");
        validate_distribution(view.mixed_probability,
                              "Mixed evidence probability is invalid");

        std::array<double, 4> expected_probabilities{};
        std::array<double, 4> expected_weights{};
        for (std::size_t expert = 0; expert < 4; ++expert) {
            validate_distribution(view.expert_probabilities[expert],
                                  "Expert evidence probability is invalid");
            expected_probabilities[expert] =
                view.expert_probabilities[expert][actual];
            expected_weights[expert] = view.mixer_weights[expert];
        }
        const double expected_mixed = view.mixed_probability[actual];
        const std::uint32_t expected_frequency =
            view.coding_cdf.v[actual + 1U] - view.coding_cdf.v[actual];
        const hz::MatchDiagnostics expected_match = view.match;

        baseline_coder.write_symbol(baseline_cdf, actual);
        instrumented_coder.write_symbol(instrumented_cdf, actual);
        baseline.observe(actual);
        instrumented.observe(actual);

        require(sink.captured().size() == position + 1U,
                "Evidence sink was not called exactly once");
        const CapturedEvidence& captured = sink.captured().back();
        require(captured.position == position && captured.actual == actual,
                "Observed evidence identity is wrong");
        for (std::size_t expert = 0; expert < 4; ++expert) {
            require_close(captured.expert_actual_probabilities[expert],
                          expected_probabilities[expert], 0.0,
                          "Observed expert probability changed");
            require_close(captured.weights[expert], expected_weights[expert],
                          0.0, "Evidence did not use pre-update weights");
        }
        require_close(captured.mixed_actual_probability, expected_mixed, 0.0,
                      "Observed mixed probability changed");
        require(captured.coding_frequency == expected_frequency,
                "Observed coding frequency changed");
        require(captured.match.status == expected_match.status &&
                    captured.match.best_match_length ==
                        expected_match.best_match_length,
                "Observed Match diagnostics changed");
        require_logic_error([&] { instrumented.current_prediction(); },
                            "Evidence survived past observation");
        ++position;
    }

    baseline_coder.finish();
    instrumented_coder.finish();
    require(baseline_payload.str() == instrumented_payload.str(),
            "Evidence changed the V1 arithmetic payload");

    const hz::PipelineDiagnostics baseline_diagnostics =
        baseline.diagnostics();
    const hz::PipelineDiagnostics instrumented_diagnostics =
        instrumented.diagnostics();
    require(baseline_diagnostics.observed_bytes ==
                instrumented_diagnostics.observed_bytes,
            "Evidence changed the diagnostic byte count");
    require(baseline_diagnostics.average_log_loss_bits ==
                instrumented_diagnostics.average_log_loss_bits &&
                baseline_diagnostics.average_mixer_weights ==
                    instrumented_diagnostics.average_mixer_weights &&
                baseline_diagnostics.final_mixer_weights ==
                    instrumented_diagnostics.final_mixer_weights,
            "Evidence changed aggregate V1 diagnostics");
}

}  // namespace

int main() {
    try {
        test_evidence_preserves_v1_cdf_and_payload();
        std::cout << "pipeline_evidence_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "pipeline_evidence_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
