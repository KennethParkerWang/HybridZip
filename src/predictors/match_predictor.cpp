#include "predictors/match_predictor.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

#include "core/byte_history.h"
#include "core/probability.h"
#include "core/profile.h"

namespace hz {
namespace {

constexpr std::uint64_t kEmptyPosition =
    std::numeric_limits<std::uint64_t>::max();
constexpr std::uint64_t kFnvOffsetBasis = 14695981039346656037ULL;
constexpr std::uint64_t kFnvPrime = 1099511628211ULL;

std::uint64_t avalanche(std::uint64_t value) noexcept {
    value ^= value >> 30U;
    value *= 0xBF58476D1CE4E5B9ULL;
    value ^= value >> 27U;
    value *= 0x94D049BB133111EBULL;
    value ^= value >> 31U;
    return value;
}

}  // namespace

MatchPredictor::MatchPredictor(const Profile& profile)
    : context_bytes_(profile.match_context_bytes),
      min_match_length_(profile.match_min_length),
      confidence_bucket_count_(profile.match_confidence_buckets),
      slot_mask_(0) {
    constexpr std::size_t size_bits =
        std::numeric_limits<std::size_t>::digits;
    if (context_bytes_ == 0 || min_match_length_ == 0 ||
        confidence_bucket_count_ <= min_match_length_ ||
        confidence_bucket_count_ > kAlphabet ||
        profile.match_hash_bits == 0 || profile.match_hash_bits >= size_bits) {
        throw std::invalid_argument("Invalid Match predictor configuration");
    }

    const std::size_t slot_count =
        std::size_t{1} << profile.match_hash_bits;
    slot_mask_ = slot_count - 1U;
    positions_.resize(slot_count);
    confidence_.resize(confidence_bucket_count_);
    reset(profile.model_seed);
}

void MatchPredictor::reset(const std::uint64_t seed) {
    (void)seed;
    std::fill(positions_.begin(), positions_.end(), kEmptyPosition);
    std::fill(confidence_.begin(), confidence_.end(), Confidence{});
    clear_pending();
    last_diagnostics_ = MatchDiagnostics{};
}

void MatchPredictor::predict(const ByteHistory& history, ProbVector& out) {
    clear_pending();
    last_diagnostics_ = MatchDiagnostics{};
    last_diagnostics_.history_position = history.position();
    if (history.size() < context_bytes_) {
        last_diagnostics_.status =
            MatchDiagnosticStatus::insufficient_history;
        set_uniform_probability(out);
        return;
    }

    const std::uint64_t candidate_position =
        positions_[slot_for_context(history)];
    if (candidate_position == kEmptyPosition) {
        last_diagnostics_.status = MatchDiagnosticStatus::no_candidate;
        set_uniform_probability(out);
        return;
    }
    last_diagnostics_.candidate_position = candidate_position;
    if (candidate_position >= history.position() ||
        !history.contains(candidate_position)) {
        last_diagnostics_.status =
            MatchDiagnosticStatus::candidate_outside_history;
        set_uniform_probability(out);
        return;
    }

    last_diagnostics_.candidate_count = 1;
    last_diagnostics_.candidate_symbol =
        history.at_absolute(candidate_position);

    const std::size_t match_length =
        measure_match(history, candidate_position);
    last_diagnostics_.best_match_length = match_length;
    if (match_length < min_match_length_) {
        last_diagnostics_.status =
            MatchDiagnosticStatus::below_minimum_length;
        set_uniform_probability(out);
        return;
    }

    const std::size_t bucket =
        std::min(match_length, confidence_bucket_count_ - 1U);
    const std::uint8_t candidate =
        history.at_absolute(candidate_position);
    const Confidence& stats = confidence_[bucket];
    const double q =
        (static_cast<double>(stats.hits) + 1.0) /
        (static_cast<double>(stats.trials) + 2.0);
    const double remainder =
        (1.0 - q) / static_cast<double>(kAlphabet - 1U);
    out.fill(remainder);
    out[candidate] = q;

    last_diagnostics_.status = MatchDiagnosticStatus::prediction_active;
    last_diagnostics_.confidence_bucket = bucket;
    last_diagnostics_.confidence_hits = stats.hits;
    last_diagnostics_.confidence_trials = stats.trials;
    last_diagnostics_.candidate_probability = q;
    last_diagnostics_.prediction_active = true;

    pending_valid_ = true;
    pending_candidate_ = candidate;
    pending_bucket_ = bucket;
    pending_history_position_ = history.position();
}

void MatchPredictor::update(const std::uint8_t actual,
                            const ByteHistory& history) {
    if (pending_valid_ &&
        pending_history_position_ == history.position()) {
        Confidence& stats = confidence_[pending_bucket_];
        if (stats.trials == std::numeric_limits<std::uint64_t>::max()) {
            stats.hits = stats.hits / 2U + stats.hits % 2U;
            stats.trials = stats.trials / 2U + stats.trials % 2U;
        }
        ++stats.trials;
        if (actual == pending_candidate_) {
            ++stats.hits;
        }
    }

    if (history.size() >= context_bytes_) {
        positions_[slot_for_context(history)] = history.position();
    }
    clear_pending();
}

const MatchDiagnostics& MatchPredictor::last_diagnostics() const noexcept {
    return last_diagnostics_;
}

std::size_t MatchPredictor::slot_for_context(
    const ByteHistory& history) const {
    std::uint64_t hash = kFnvOffsetBasis;
    for (std::size_t distance = context_bytes_; distance > 0; --distance) {
        hash ^= history.back(distance);
        hash *= kFnvPrime;
    }
    return static_cast<std::size_t>(avalanche(hash)) & slot_mask_;
}

std::size_t MatchPredictor::measure_match(
    const ByteHistory& history,
    const std::uint64_t candidate_position) const {
    const std::uint64_t current_position = history.position();
    const std::size_t maximum_length = confidence_bucket_count_ - 1U;
    std::size_t length = 0;
    while (length < maximum_length &&
           candidate_position > static_cast<std::uint64_t>(length) &&
           current_position > static_cast<std::uint64_t>(length)) {
        const std::uint64_t candidate_byte_position =
            candidate_position - 1U - length;
        const std::uint64_t current_byte_position =
            current_position - 1U - length;
        if (!history.contains(candidate_byte_position) ||
            !history.contains(current_byte_position) ||
            history.at_absolute(candidate_byte_position) !=
                history.at_absolute(current_byte_position)) {
            break;
        }
        ++length;
    }
    return length;
}

void MatchPredictor::clear_pending() noexcept {
    pending_valid_ = false;
    pending_candidate_ = 0;
    pending_bucket_ = 0;
    pending_history_position_ = 0;
}

}  // namespace hz
