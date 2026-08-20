#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/types.h"
#include "predictors/match_predictor.h"

namespace hz {

// Views are valid only until the matching ModelPipeline::observe() returns.
struct PipelinePredictionView {
    std::uint64_t position;
    const std::vector<ProbVector>& expert_probabilities;
    const ProbVector& mixed_probability;
    const Cdf& coding_cdf;
    const std::vector<double>& mixer_weights;
    MatchDiagnostics match;
    std::size_t ppmd_context_depth;
};

struct PipelineByteEvidence {
    std::uint64_t position;
    std::uint8_t actual;
    const std::vector<ProbVector>& expert_probabilities;
    const ProbVector& mixed_probability;
    const Cdf& coding_cdf;
    const std::vector<double>& mixer_weights;
    MatchDiagnostics match;
    std::size_t ppmd_context_depth;
};

class PipelineEvidenceSink {
public:
    virtual ~PipelineEvidenceSink() = default;
    virtual void on_byte(const PipelineByteEvidence& evidence) = 0;
};

}  // namespace hz
