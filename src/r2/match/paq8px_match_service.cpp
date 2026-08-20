#include "r2/match/paq8px_match_service.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

#include "../../../third_party/paq8px/match_core.h"

namespace hz::r2 {

Paq8pxMatchService::Paq8pxMatchService(const std::uint8_t hash_bits)
    : core_(std::make_unique<paq8px::MatchCore>(hash_bits)) {
    reset(0);
}

Paq8pxMatchService::~Paq8pxMatchService() = default;
Paq8pxMatchService::Paq8pxMatchService(Paq8pxMatchService&&) noexcept =
    default;
Paq8pxMatchService& Paq8pxMatchService::operator=(
    Paq8pxMatchService&&) noexcept = default;

const char* Paq8pxMatchService::name() const noexcept {
    return "paq8px-match";
}

void Paq8pxMatchService::reset(const std::uint64_t seed) {
    (void)seed;
    core_->reset();
    position_ = 0;
    cached_candidates_.clear();
}

std::size_t Paq8pxMatchService::allocation_bytes() const noexcept {
    return core_->allocation_bytes();
}

const paq8px::MatchCore& Paq8pxMatchService::core() const noexcept {
    return *core_;
}

std::vector<MatchCandidate> Paq8pxMatchService::truncate_cache(
    const std::size_t maximum_candidates) const {
    const std::size_t count =
        std::min({maximum_candidates,
                  paq8px::MatchCore::kMaximumCandidates,
                  cached_candidates_.size()});
    std::vector<MatchCandidate> result = cached_candidates_;
    result.resize(count);
    return result;
}

std::vector<MatchCandidate> Paq8pxMatchService::find(
    const ByteView history,
    const std::size_t position,
    const std::size_t maximum_candidates) {
    if (position > history.size()) {
        throw std::out_of_range(
            "PAQ8px match position is outside the supplied history");
    }
    if (position > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error(
            "PAQ8px match position exceeds the 32-bit block limit");
    }
    if (position < position_) {
        throw std::invalid_argument(
            "PAQ8px match service does not accept backward positions");
    }
    if (position != 0 && history.data() == nullptr) {
        throw std::invalid_argument(
            "PAQ8px match history has a null data pointer");
    }
    if (position == position_) {
        return truncate_cache(maximum_candidates);
    }

    while (position_ < position) {
        const std::uint32_t next_position =
            static_cast<std::uint32_t>(position_ + 1U);
        core_->advance(history.data(), history.size(), next_position);
        ++position_;
    }

    std::vector<paq8px::MatchSnapshot> snapshots =
        core_->predictive_candidates();
    std::stable_sort(
        snapshots.begin(), snapshots.end(),
        [](const paq8px::MatchSnapshot& left,
           const paq8px::MatchSnapshot& right) {
            return left.priority > right.priority;
        });

    cached_candidates_.clear();
    cached_candidates_.reserve(snapshots.size());
    for (const paq8px::MatchSnapshot& snapshot : snapshots) {
        if (snapshot.index >= position) {
            throw std::logic_error(
                "PAQ8px match candidate does not precede its position");
        }
        MatchCandidate candidate{};
        candidate.distance = position - snapshot.index;
        candidate.length = snapshot.contiguous_length;
        candidate.next_byte = snapshot.next_byte;
        candidate.confidence = snapshot.strength;
        candidate.estimated_parse_cost =
            std::numeric_limits<std::uint32_t>::max();
        cached_candidates_.push_back(candidate);
    }
    return truncate_cache(maximum_candidates);
}

}  // namespace hz::r2
