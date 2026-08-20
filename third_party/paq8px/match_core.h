#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace hz::paq8px {

enum class MatchMode : std::uint8_t {
    normal,
    delta,
    pre_recovery,
    recovery,
};

struct MatchSnapshot {
    std::uint32_t index = 0;
    std::uint32_t strength = 0;
    std::uint32_t contiguous_length = 0;
    std::uint32_t backup_strength = 0;
    std::uint32_t backup_index = 0;
    std::uint8_t next_byte = 0;
    std::uint8_t registration_order = 0;
    MatchMode mode = MatchMode::normal;
    std::uint64_t priority = 0;
};

class MatchCore {
public:
    static constexpr std::uint8_t kDefaultHashBits = 20;
    static constexpr std::uint8_t kMinimumHashBits = 1;
    static constexpr std::uint8_t kMaximumHashBits = 26;
    static constexpr std::size_t kPositionsPerBucket = 3;
    static constexpr std::size_t kMaximumCandidates = 4;
    static constexpr std::uint32_t kMaximumStrength = 65535;
    static constexpr std::uint32_t kRecoveryThreshold = 3;
    static constexpr std::array<std::uint8_t, 3> kLookupOrders{{9, 7, 5}};

    explicit MatchCore(std::uint8_t hash_bits = kDefaultHashBits);
    ~MatchCore();

    MatchCore(const MatchCore&) = delete;
    MatchCore& operator=(const MatchCore&) = delete;
    MatchCore(MatchCore&&) noexcept;
    MatchCore& operator=(MatchCore&&) noexcept;

    void reset();

    // Advance exactly one byte boundary. `position` is the number of bytes in
    // the available prefix and must equal the previous position plus one.
    void advance(const std::uint8_t* history,
                 std::size_t history_size,
                 std::uint32_t position);

    std::vector<MatchSnapshot> active_candidates() const;
    std::vector<MatchSnapshot> predictive_candidates() const;

    std::uint8_t hash_bits() const noexcept { return hash_bits_; }
    std::size_t bucket_count() const noexcept { return bucket_count_; }
    std::size_t allocation_bytes() const noexcept { return allocation_bytes_; }
    std::uint32_t position() const noexcept { return position_; }

    std::uint64_t order_hash(std::uint8_t order) const;
    std::uint32_t order_bucket(std::uint8_t order) const;
    std::array<std::uint32_t, kPositionsPerBucket> bucket_positions(
        std::uint32_t bucket) const;

    static std::size_t allocation_bytes_for_hash_bits(std::uint8_t hash_bits);

private:
    struct Bucket {
        std::array<std::uint32_t, kPositionsPerBucket> match_positions{};

        void add(std::uint32_t position) noexcept;
    };
    static_assert(sizeof(Bucket) == 12,
                  "PAQ8px match buckets must occupy exactly 12 bytes");

    struct CandidateState {
        std::uint32_t length = 0;
        std::uint32_t index = 0;
        std::uint32_t length_backup = 0;
        std::uint32_t index_backup = 0;
        std::uint32_t contiguous_length = 0;
        std::uint8_t expected_byte = 0;
        std::uint8_t registration_order = 0;
        bool delta = false;

        bool no_match() const noexcept;
        bool pre_recovery() const noexcept;
        bool recovery() const noexcept;
        MatchMode mode() const noexcept;
        std::uint64_t priority() const noexcept;
    };

    static constexpr std::uint64_t kPhi64 =
        UINT64_C(0x9E3779B97F4A7C15);
    static constexpr std::uint64_t kGenericBlockTypeCount = 30;

    void update_hashes(std::uint8_t byte) noexcept;
    void update_candidate(CandidateState& candidate,
                          const std::uint8_t* history,
                          std::uint32_t position);
    bool is_match(const std::uint8_t* history,
                  std::uint32_t position,
                  std::uint32_t match_position,
                  std::uint8_t length) const noexcept;
    void add_candidates(const Bucket& bucket,
                        std::uint8_t order,
                        const std::uint8_t* history,
                        std::uint32_t position);
    void process_order(std::uint8_t order,
                       const std::uint8_t* history,
                       std::uint32_t position);
    MatchSnapshot snapshot(const CandidateState& candidate) const noexcept;
    std::uint32_t finalize(std::uint64_t hash) const noexcept;

    std::uint8_t hash_bits_ = 0;
    std::size_t bucket_count_ = 0;
    std::size_t allocation_bytes_ = 0;
    std::unique_ptr<Bucket[]> table_;
    std::array<CandidateState, kMaximumCandidates> candidates_{};
    std::size_t candidate_count_ = 0;
    std::array<std::uint64_t, 15> order_hashes_{};
    std::uint32_t position_ = 0;
};

static_assert(sizeof(std::array<std::uint32_t, 3>) == 12,
              "PAQ8px match bucket positions must occupy exactly 12 bytes");

}  // namespace hz::paq8px
