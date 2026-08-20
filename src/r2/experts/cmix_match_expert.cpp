#include "r2/experts/cmix_match_expert.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "match_core.h"

namespace hz::r2 {
namespace {

std::uint32_t to_q24(const float probability) noexcept {
    const double scaled =
        static_cast<double>(probability) * CmixMatchExpert::kProbabilityScale;
    const double rounded = std::floor(scaled + 0.5);
    return static_cast<std::uint32_t>(std::clamp(
        rounded, 1.0,
        static_cast<double>(CmixMatchExpert::kProbabilityScale - 1U)));
}

}  // namespace

CmixMatchExpert::CmixMatchExpert()
    : CmixMatchExpert(cmix::MatchConfig{}) {}

CmixMatchExpert::CmixMatchExpert(const cmix::MatchConfig& config)
    : core_(std::make_unique<cmix::MatchCore>(config)) {}

CmixMatchExpert::~CmixMatchExpert() = default;

const char* CmixMatchExpert::name() const noexcept {
    return "cmix-match";
}

bool CmixMatchExpert::same_context(const ExpertContext& left,
                                   const ExpertContext& right) noexcept {
    return left.absolute_position == right.absolute_position &&
           left.block_type == right.block_type &&
           left.byte_in_block == right.byte_in_block;
}

ExpertEvidence CmixMatchExpert::predict(const ExpertContext& context) {
    if (prediction_pending_ &&
        !same_context(context, predicted_context_)) {
        throw std::logic_error(
            "cmix Match expert requires observe before advancing context");
    }
    predicted_context_ = context;
    prediction_pending_ = true;
    return BitPosterior{to_q24(core_->predict()), kProbabilityScale};
}

void CmixMatchExpert::observe(const std::uint8_t actual,
                              const ExpertContext& context) {
    if (actual > 1) {
        throw std::invalid_argument("cmix Match expert accepts only bits 0 and 1");
    }
    if (!prediction_pending_) {
        throw std::logic_error(
            "cmix Match expert observe requires a preceding prediction");
    }
    if (!same_context(context, predicted_context_)) {
        throw std::invalid_argument(
            "cmix Match expert observe context differs from predict");
    }
    core_->observe(actual);
    prediction_pending_ = false;
}

void CmixMatchExpert::reset_block(const ExpertContext& context) {
    // cmix Match has no random state; block metadata and seeds are irrelevant.
    (void)context;
    core_->reset();
    prediction_pending_ = false;
    predicted_context_ = {};
}

bool CmixMatchExpert::prediction_pending() const noexcept {
    return prediction_pending_;
}

const cmix::MatchCore& CmixMatchExpert::core() const noexcept {
    return *core_;
}

}  // namespace hz::r2
