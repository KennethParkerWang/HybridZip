#include "codec/model_pipeline.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

#include "core/cdf.h"
#include "core/probability.h"
#include "predictors/match_predictor.h"
#include "predictors/ngram_predictor.h"
#include "predictors/online_lstm_predictor.h"
#include "predictors/ppmd_predictor.h"

namespace hz {

ModelPipeline::ModelPipeline(const Profile& profile)
    : profile_(profile),
      history_(profile.history_capacity),
      mixer_(profile.expert_count, profile.mixer_eta),
      expert_outputs_(profile.expert_count) {
    if (profile_.id != 1U || profile_.expert_count != 4U ||
        profile_.cdf_bits != kCdfBits ||
        profile_.coder_state_bits != kCoderStateBits) {
        throw std::invalid_argument("ModelPipeline requires PROFILE_V1");
    }

    predictors_.reserve(profile_.expert_count);
    predictors_.push_back(std::make_unique<NGramPredictor>(profile_));
    auto ppmd_predictor = std::make_unique<PpmdPredictor>(profile_);
    ppmd_predictor_ = ppmd_predictor.get();
    predictors_.push_back(std::move(ppmd_predictor));
    auto match_predictor = std::make_unique<MatchPredictor>(profile_);
    match_predictor_ = match_predictor.get();
    predictors_.push_back(std::move(match_predictor));
    predictors_.push_back(std::make_unique<OnlineLstmPredictor>(profile_));
    reset(profile_.model_seed);
}

void ModelPipeline::reset(const std::uint64_t seed) {
    history_.reset();
    mixer_.reset();
    for (const std::unique_ptr<Predictor>& predictor : predictors_) {
        predictor->reset(seed);
    }
    for (ProbVector& output : expert_outputs_) {
        set_uniform_probability(output);
    }
    set_uniform_probability(mixed_);
    cdf_ = quantize_to_cdf(mixed_);
    prediction_ready_ = false;
    observed_bytes_ = 0;
    cumulative_log_loss_bits_.fill(0.0);
    cumulative_mixer_weights_.fill(0.0);
}

const Cdf& ModelPipeline::predict_cdf() {
    if (prediction_ready_) {
        throw std::logic_error("ModelPipeline prediction was not observed");
    }
    for (std::size_t expert = 0; expert < predictors_.size(); ++expert) {
        predictors_[expert]->predict(history_, expert_outputs_[expert]);
        normalize_probability(expert_outputs_[expert]);
    }
    mixer_.mix(expert_outputs_, mixed_);
    normalize_probability(mixed_);
    cdf_ = quantize_to_cdf(mixed_);
    prediction_ready_ = true;
    return cdf_;
}

PipelinePredictionView ModelPipeline::current_prediction() const {
    if (!prediction_ready_) {
        throw std::logic_error("ModelPipeline has no current prediction");
    }
    return PipelinePredictionView{
        observed_bytes_, expert_outputs_, mixed_, cdf_, mixer_.weights(),
        match_predictor_->last_diagnostics(),
        ppmd_predictor_->current_context_depth()};
}

void ModelPipeline::observe(const std::uint8_t actual) {
    if (!prediction_ready_) {
        throw std::logic_error("ModelPipeline observation needs a prediction");
    }

    const std::vector<double>& weights = mixer_.weights();
    if (evidence_sink_ != nullptr) {
        const PipelineByteEvidence evidence{
            observed_bytes_, actual, expert_outputs_, mixed_, cdf_, weights,
            match_predictor_->last_diagnostics(),
            ppmd_predictor_->current_context_depth()};
        evidence_sink_->on_byte(evidence);
    }
    for (std::size_t expert = 0; expert < predictors_.size(); ++expert) {
        cumulative_log_loss_bits_[expert] +=
            -std::log2(std::max(expert_outputs_[expert][actual], kProbFloor));
        cumulative_mixer_weights_[expert] += weights[expert];
    }

    mixer_.update(actual, expert_outputs_);
    for (const std::unique_ptr<Predictor>& predictor : predictors_) {
        predictor->update(actual, history_);
    }
    history_.push(actual);
    ++observed_bytes_;
    prediction_ready_ = false;
}

void ModelPipeline::set_evidence_sink(PipelineEvidenceSink* const sink) noexcept {
    evidence_sink_ = sink;
}

PipelineDiagnostics ModelPipeline::diagnostics() const {
    PipelineDiagnostics result{};
    result.observed_bytes = observed_bytes_;
    const std::vector<double>& final_weights = mixer_.weights();
    for (std::size_t expert = 0; expert < result.final_mixer_weights.size();
         ++expert) {
        result.final_mixer_weights[expert] = final_weights[expert];
        if (observed_bytes_ != 0) {
            result.average_log_loss_bits[expert] =
                cumulative_log_loss_bits_[expert] / observed_bytes_;
            result.average_mixer_weights[expert] =
                cumulative_mixer_weights_[expert] / observed_bytes_;
        }
    }
    return result;
}

}  // namespace hz
