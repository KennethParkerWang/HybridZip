#include "match_core.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace hz::paq8px {

namespace {

void saturating_increment(std::uint32_t& value,
                          const std::uint32_t maximum) noexcept {
    if (value < maximum) {
        ++value;
    }
}

}  // namespace

void MatchCore::Bucket::add(const std::uint32_t position) noexcept {
    for (std::size_t i = match_positions.size() - 1; i != 0; --i) {
        match_positions[i] = match_positions[i - 1];
    }
    match_positions[0] = position;
}

bool MatchCore::CandidateState::no_match() const noexcept {
    return length == 0 && !delta && length_backup == 0;
}

bool MatchCore::CandidateState::pre_recovery() const noexcept {
    return length == 0 && !delta && length_backup != 0;
}

bool MatchCore::CandidateState::recovery() const noexcept {
    return length != 0 && length_backup != 0;
}

MatchMode MatchCore::CandidateState::mode() const noexcept {
    if (delta) {
        return MatchMode::delta;
    }
    if (pre_recovery()) {
        return MatchMode::pre_recovery;
    }
    if (recovery()) {
        return MatchMode::recovery;
    }
    return MatchMode::normal;
}

std::uint64_t MatchCore::CandidateState::priority() const noexcept {
    return (static_cast<std::uint64_t>(length != 0) << 49U) |
           (static_cast<std::uint64_t>(delta) << 48U) |
           (static_cast<std::uint64_t>(delta ? length_backup : length)
            << 32U) |
           index;
}

std::size_t MatchCore::allocation_bytes_for_hash_bits(
    const std::uint8_t hash_bits) {
    if (hash_bits < kMinimumHashBits || hash_bits > kMaximumHashBits) {
        throw std::invalid_argument(
            "PAQ8px MatchCore hash bits must be in [1, 26]");
    }

    constexpr std::size_t bucket_bytes =
        sizeof(std::array<std::uint32_t, kPositionsPerBucket>);
    const std::size_t buckets = std::size_t{1} << hash_bits;
    if (buckets > std::numeric_limits<std::size_t>::max() / bucket_bytes) {
        throw std::length_error("PAQ8px MatchCore table size overflows size_t");
    }
    return buckets * bucket_bytes;
}

MatchCore::MatchCore(const std::uint8_t hash_bits)
    : hash_bits_(hash_bits),
      bucket_count_(allocation_bytes_for_hash_bits(hash_bits) /
                    sizeof(Bucket)),
      allocation_bytes_(bucket_count_ * sizeof(Bucket)),
      table_(std::make_unique<Bucket[]>(bucket_count_)) {
    reset();
}

MatchCore::~MatchCore() = default;
MatchCore::MatchCore(MatchCore&&) noexcept = default;
MatchCore& MatchCore::operator=(MatchCore&&) noexcept = default;

void MatchCore::reset() {
    std::fill_n(table_.get(), bucket_count_, Bucket{});
    candidates_ = {};
    candidate_count_ = 0;
    order_hashes_ = {};
    position_ = 0;

    // PAQ8px calls NormalModel::updateHashes() before MatchModel on the first
    // bit. Its initial c1 is zero, so prime the recurrence with that byte.
    update_hashes(0);
}

void MatchCore::update_hashes(const std::uint8_t byte) noexcept {
    const std::uint64_t base_context =
        static_cast<std::uint64_t>(byte) * kGenericBlockTypeCount;
    for (std::size_t order = order_hashes_.size() - 1; order != 0;
         --order) {
        order_hashes_[order] =
            (order_hashes_[order - 1] + base_context + order) * kPhi64;
    }
}

std::uint32_t MatchCore::finalize(const std::uint64_t hash) const noexcept {
    return static_cast<std::uint32_t>(hash >> (64U - hash_bits_));
}

std::uint64_t MatchCore::order_hash(const std::uint8_t order) const {
    if (order >= order_hashes_.size()) {
        throw std::out_of_range("PAQ8px MatchCore order hash is out of range");
    }
    return order_hashes_[order];
}

std::uint32_t MatchCore::order_bucket(const std::uint8_t order) const {
    return finalize(order_hash(order));
}

std::array<std::uint32_t, MatchCore::kPositionsPerBucket>
MatchCore::bucket_positions(const std::uint32_t bucket) const {
    if (bucket >= bucket_count_) {
        throw std::out_of_range("PAQ8px MatchCore bucket is out of range");
    }
    return table_[bucket].match_positions;
}

bool MatchCore::is_match(const std::uint8_t* const history,
                         const std::uint32_t position,
                         const std::uint32_t match_position,
                         const std::uint8_t length) const noexcept {
    if (position < length || match_position < length ||
        match_position >= position) {
        return false;
    }
    for (std::uint32_t offset = 1; offset <= length; ++offset) {
        if (history[position - offset] !=
            history[match_position - offset]) {
            return false;
        }
    }
    return true;
}

void MatchCore::add_candidates(const Bucket& bucket,
                               const std::uint8_t order,
                               const std::uint8_t* const history,
                               const std::uint32_t position) {
    for (const std::uint32_t match_position : bucket.match_positions) {
        if (candidate_count_ == kMaximumCandidates || match_position == 0) {
            break;
        }
        if (!is_match(history, position, match_position, order)) {
            continue;
        }

        bool duplicate = false;
        for (std::size_t i = 0; i < candidate_count_; ++i) {
            if (candidates_[i].index == match_position) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) {
            continue;
        }

        CandidateState& candidate = candidates_[candidate_count_++];
        candidate = {};
        candidate.length = static_cast<std::uint32_t>(order) - 5U + 1U;
        candidate.index = match_position;
        candidate.contiguous_length = order;
        candidate.registration_order = order;
    }
}

void MatchCore::process_order(const std::uint8_t order,
                              const std::uint8_t* const history,
                              const std::uint32_t position) {
    Bucket& bucket = table_[finalize(order_hashes_[order])];
    if (candidate_count_ < kMaximumCandidates) {
        add_candidates(bucket, order, history, position);
    }
    bucket.add(position);
}

void MatchCore::update_candidate(CandidateState& candidate,
                                 const std::uint8_t* const history,
                                 const std::uint32_t position) {
    const std::uint8_t actual = history[position - 1U];

    if (candidate.length != 0 && actual != candidate.expected_byte) {
        if (candidate.recovery()) {
            // A second mismatch while recovering is fatal in MatchInfo::update.
            candidate.length_backup = 0;
            candidate.index_backup = 0;
        } else {
            candidate.length_backup = candidate.length;
            candidate.index_backup = candidate.index;
            candidate.delta = true;
        }
        candidate.length = 0;
        candidate.contiguous_length = 0;
    }

    // This block is the byte-boundary equivalent of donor bpos == 0. A first
    // mismatch spent the remaining bits in delta mode, then becomes
    // pre-recovery for one complete byte. Therefore a freshly set delta flag
    // deliberately prevents recovery at this boundary.
    if (candidate.pre_recovery()) {
        ++candidate.index_backup;
        saturating_increment(candidate.length_backup, kMaximumStrength);
        if (candidate.index_backup < position &&
            history[candidate.index_backup] == actual) {
            candidate.length = candidate.length_backup;
            candidate.index = candidate.index_backup;
            candidate.contiguous_length = 0;
        } else {
            candidate.length_backup = 0;
            candidate.index_backup = 0;
        }
    }

    if (candidate.length != 0) {
        ++candidate.index;
        saturating_increment(candidate.length, kMaximumStrength);
        saturating_increment(candidate.contiguous_length,
                             std::numeric_limits<std::uint32_t>::max());
        if (candidate.recovery() &&
            candidate.length - candidate.length_backup >=
                kRecoveryThreshold) {
            candidate.length_backup = 0;
            candidate.index_backup = 0;
        }
    }

    candidate.delta = false;
}

void MatchCore::advance(const std::uint8_t* const history,
                        const std::size_t history_size,
                        const std::uint32_t position) {
    if (position == 0 || position != position_ + 1U) {
        throw std::invalid_argument(
            "PAQ8px MatchCore advance must move exactly one byte forward");
    }
    if (history == nullptr || history_size < position) {
        throw std::out_of_range(
            "PAQ8px MatchCore history does not contain the requested prefix");
    }

    for (std::size_t i = 0; i < candidate_count_;) {
        update_candidate(candidates_[i], history, position);
        if (candidates_[i].no_match()) {
            for (std::size_t j = i + 1; j < candidate_count_; ++j) {
                candidates_[j - 1] = candidates_[j];
            }
            --candidate_count_;
            candidates_[candidate_count_] = {};
        } else {
            ++i;
        }
    }

    update_hashes(history[position - 1U]);
    for (const std::uint8_t order : MatchCore::kLookupOrders) {
        process_order(order, history, position);
    }

    for (std::size_t i = 0; i < candidate_count_; ++i) {
        if (candidates_[i].index >= position) {
            throw std::logic_error(
                "PAQ8px MatchCore candidate index is not in the prefix");
        }
        candidates_[i].expected_byte = history[candidates_[i].index];
    }
    position_ = position;
}

MatchSnapshot MatchCore::snapshot(
    const CandidateState& candidate) const noexcept {
    MatchSnapshot result{};
    result.index = candidate.index;
    result.strength = candidate.length;
    result.contiguous_length = candidate.contiguous_length;
    result.backup_strength = candidate.length_backup;
    result.backup_index = candidate.index_backup;
    result.next_byte = candidate.expected_byte;
    result.registration_order = candidate.registration_order;
    result.mode = candidate.mode();
    result.priority = candidate.priority();
    return result;
}

std::vector<MatchSnapshot> MatchCore::active_candidates() const {
    std::vector<MatchSnapshot> result;
    result.reserve(candidate_count_);
    for (std::size_t i = 0; i < candidate_count_; ++i) {
        result.push_back(snapshot(candidates_[i]));
    }
    return result;
}

std::vector<MatchSnapshot> MatchCore::predictive_candidates() const {
    std::vector<MatchSnapshot> result;
    result.reserve(candidate_count_);
    for (std::size_t i = 0; i < candidate_count_; ++i) {
        if (candidates_[i].length != 0) {
            result.push_back(snapshot(candidates_[i]));
        }
    }
    return result;
}

}  // namespace hz::paq8px
