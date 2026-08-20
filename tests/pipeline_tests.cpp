#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "codec/model_pipeline.h"
#include "core/profile.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
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

}  // namespace

int main() {
    try {
        const hz::Profile profile = hz::make_profile_v1();
        hz::ModelPipeline lifecycle(profile);
        lifecycle.reset(profile.model_seed);
        require_logic_error([&] { lifecycle.observe(0); },
                            "Pipeline accepted observation before prediction");
        lifecycle.predict_cdf();
        require_logic_error([&] { lifecycle.predict_cdf(); },
                            "Pipeline accepted an unobserved second prediction");

        hz::ModelPipeline first(profile);
        hz::ModelPipeline second(profile);
        first.reset(profile.model_seed);
        second.reset(profile.model_seed);
        constexpr std::string_view sequence = "abracadabra abracadabra";
        for (const unsigned char actual : sequence) {
            const hz::Cdf& left = first.predict_cdf();
            const hz::Cdf& right = second.predict_cdf();
            require(left.v == right.v, "Seeded pipelines produced different CDFs");
            first.observe(actual);
            second.observe(actual);
        }

        const hz::PipelineDiagnostics diagnostics = first.diagnostics();
        require(diagnostics.observed_bytes == sequence.size(),
                "Pipeline diagnostic byte count is wrong");
        double final_weight_sum = 0.0;
        for (std::size_t expert = 0;
             expert < diagnostics.final_mixer_weights.size(); ++expert) {
            require(std::isfinite(diagnostics.average_log_loss_bits[expert]) &&
                        diagnostics.average_log_loss_bits[expert] > 0.0,
                    "Expert log-loss diagnostic is invalid");
            require(std::isfinite(diagnostics.average_mixer_weights[expert]) &&
                        diagnostics.average_mixer_weights[expert] > 0.0,
                    "Expert average mixer weight is invalid");
            require(std::isfinite(diagnostics.final_mixer_weights[expert]) &&
                        diagnostics.final_mixer_weights[expert] > 0.0,
                    "Expert final mixer weight is invalid");
            final_weight_sum += diagnostics.final_mixer_weights[expert];
        }
        require(std::abs(final_weight_sum - 1.0) < 1e-12,
                "Final mixer weights do not sum to one");

        std::cout << "pipeline_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "pipeline_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
