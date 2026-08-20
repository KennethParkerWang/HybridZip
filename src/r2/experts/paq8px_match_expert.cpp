#include "r2/experts/paq8px_match_expert.h"

#include <limits>
#include <stdexcept>
#include <utility>

namespace hz::r2 {

Paq8pxMatchExpert::Paq8pxMatchExpert(const std::uint8_t hash_bits)
    : service_(hash_bits) {}

const char* Paq8pxMatchExpert::name() const noexcept {
    return "paq8px-match-expert";
}

bool Paq8pxMatchExpert::same_context(const ExpertContext& left,
                                     const ExpertContext& right) noexcept {
    return left.absolute_position == right.absolute_position &&
           left.block_type == right.block_type &&
           left.byte_in_block == right.byte_in_block;
}

ExpertEvidence Paq8pxMatchExpert::predict(const ExpertContext& context) {
    if (context.byte_in_block != history_.size()) {
        throw std::invalid_argument(
            "PAQ8px match expert context is not at the prefix boundary");
    }
    if (prediction_pending_ &&
        !same_context(context, predicted_context_)) {
        throw std::logic_error(
            "PAQ8px match expert requires observe before advancing context");
    }

    const std::vector<MatchCandidate> matches = service_.find(
        ByteView(history_), history_.size(),
        Paq8pxMatchService::kMaximumCandidates);
    MatchEvidence evidence{};
    evidence.candidates.reserve(matches.size());
    for (const MatchCandidate& match : matches) {
        MatchHypothesis hypothesis{};
        hypothesis.distance = match.distance;
        hypothesis.length = match.length;
        hypothesis.next_byte = match.next_byte;
        hypothesis.confidence = match.confidence;
        evidence.candidates.push_back(hypothesis);
    }

    predicted_context_ = context;
    prediction_pending_ = true;
    return ExpertEvidence{std::move(evidence)};
}

void Paq8pxMatchExpert::observe(const std::uint8_t actual,
                                const ExpertContext& context) {
    if (!prediction_pending_) {
        throw std::logic_error(
            "PAQ8px match expert observe requires a preceding predict");
    }
    if (!same_context(context, predicted_context_)) {
        throw std::invalid_argument(
            "PAQ8px match expert observe context differs from predict");
    }
    if (history_.size() ==
        std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error(
            "PAQ8px match expert block exceeds the 32-bit position limit");
    }
    history_.push_back(actual);
    prediction_pending_ = false;
}

void Paq8pxMatchExpert::reset_block(const ExpertContext& context) {
    history_.clear();
    service_.reset(context.absolute_position);
    prediction_pending_ = false;
    predicted_context_ = {};
}

}  // namespace hz::r2
