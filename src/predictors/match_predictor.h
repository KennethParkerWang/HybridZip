#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "predictors/predictor.h"

namespace hz {

struct Profile;

enum class MatchDiagnosticStatus : std::uint8_t {
    not_predicted,
    insufficient_history,
    no_candidate,
    candidate_outside_history,
    below_minimum_length,
    prediction_active,
};

struct MatchDiagnostics {
    MatchDiagnosticStatus status = MatchDiagnosticStatus::not_predicted;
    std::uint64_t history_position = 0;
    std::uint64_t candidate_position = 0;
    std::size_t candidate_count = 0;
    std::size_t best_match_length = 0;
    std::size_t confidence_bucket = 0;
    std::uint64_t confidence_hits = 0;
    std::uint64_t confidence_trials = 0;
    std::uint8_t candidate_symbol = 0;
    double candidate_probability = 0.0;
    bool prediction_active = false;
};

class MatchPredictor final : public Predictor {
public:
    explicit MatchPredictor(const Profile& profile);

    void reset(std::uint64_t seed) override;
    void predict(const ByteHistory& history, ProbVector& out) override;
    void update(std::uint8_t actual, const ByteHistory& history) override;

    const MatchDiagnostics& last_diagnostics() const noexcept;

private:
    struct Confidence {
        std::uint64_t hits = 0;
        std::uint64_t trials = 0;
    };

    std::size_t slot_for_context(const ByteHistory& history) const;
    std::size_t measure_match(const ByteHistory& history,
                              std::uint64_t candidate_position) const;
    void clear_pending() noexcept;

    std::size_t context_bytes_;
    std::size_t min_match_length_;
    std::size_t confidence_bucket_count_;
    std::size_t slot_mask_;

    std::vector<std::uint64_t> positions_;
    std::vector<Confidence> confidence_;

    bool pending_valid_ = false;
    std::uint8_t pending_candidate_ = 0;
    std::size_t pending_bucket_ = 0;
    std::uint64_t pending_history_position_ = 0;
    MatchDiagnostics last_diagnostics_{};
};

}  // namespace hz
