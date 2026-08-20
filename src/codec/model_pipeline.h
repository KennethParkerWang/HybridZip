#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "analysis/pipeline_evidence.h"
#include "core/byte_history.h"
#include "core/profile.h"
#include "core/types.h"
#include "mixer/adaptive_linear_mixer.h"
#include "predictors/predictor.h"

namespace hz {

class PpmdPredictor;

struct PipelineDiagnostics {
    std::uint64_t observed_bytes = 0;
    std::array<double, 4> average_log_loss_bits{};
    std::array<double, 4> average_mixer_weights{};
    std::array<double, 4> final_mixer_weights{};
};

class ModelPipeline {
public:
    explicit ModelPipeline(const Profile& profile);

    void reset(std::uint64_t seed);
    const Cdf& predict_cdf();
    PipelinePredictionView current_prediction() const;
    void observe(std::uint8_t actual);

    void set_evidence_sink(PipelineEvidenceSink* sink) noexcept;

    PipelineDiagnostics diagnostics() const;

private:
    Profile profile_;
    ByteHistory history_;
    std::vector<std::unique_ptr<Predictor>> predictors_;
    PpmdPredictor* ppmd_predictor_ = nullptr;
    MatchPredictor* match_predictor_ = nullptr;
    AdaptiveLinearMixer mixer_;
    std::vector<ProbVector> expert_outputs_;
    ProbVector mixed_{};
    Cdf cdf_{};
    bool prediction_ready_ = false;
    PipelineEvidenceSink* evidence_sink_ = nullptr;

    std::uint64_t observed_bytes_ = 0;
    std::array<double, 4> cumulative_log_loss_bits_{};
    std::array<double, 4> cumulative_mixer_weights_{};
};

}  // namespace hz
