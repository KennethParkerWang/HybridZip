#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace hz::cmix {

struct MatchConfig {
    std::size_t history_size = 8U << 20U;
    std::uint32_t context_order = 2;
    std::uint32_t hash_bits = 8;
    std::size_t map_entries = 1U << 16U;
    int limit = 200;
    float delta = 0.5F;
};

struct MatchDiagnostics {
    std::uint64_t history_position = 0;
    std::uint64_t byte_context = 0;
    std::uint64_t current_match = 0;
    std::size_t history_write_index = 0;
    std::size_t map_index = 0;
    std::uint32_t bit_context = 1;
    std::uint8_t current_byte = 0;
    std::uint8_t bit_mask = 128;
    std::uint8_t match_length = 0;
    std::uint8_t match_context = 0;
    std::uint8_t last_completed_byte = 0;
    bool last_observation_matched = false;
    bool has_completed_byte = false;
};

class MatchCore {
public:
    static constexpr std::size_t kBucketCount = 256;

    explicit MatchCore(MatchConfig config = {});

    MatchCore(const MatchCore&) = delete;
    MatchCore& operator=(const MatchCore&) = delete;
    MatchCore(MatchCore&&) = delete;
    MatchCore& operator=(MatchCore&&) = delete;

    float predict() const noexcept;
    void observe(std::uint8_t bit);
    void reset() noexcept;

    const MatchConfig& config() const noexcept;
    MatchDiagnostics diagnostics() const noexcept;

    float bucket_probability(std::size_t bucket) const;
    int bucket_count(std::size_t bucket) const;
    std::uint32_t map_entry(std::size_t index) const;
    std::uint8_t history_byte(std::size_t index) const;

    std::size_t allocation_bytes() const noexcept;
    static std::size_t required_allocation_bytes(const MatchConfig& config);

private:
    void byte_update() noexcept;
    std::size_t current_map_index() const noexcept;

    MatchConfig config_;
    std::unique_ptr<std::uint8_t[]> history_;
    std::unique_ptr<std::uint32_t[]> map_;
    std::array<float, kBucketCount> predictions_{};
    std::array<int, kBucketCount> counts_{};

    std::uint64_t context_base_ = 0;
    std::uint64_t context_modulus_ = 0;
    std::uint64_t history_position_ = 0;
    std::uint64_t byte_context_ = 0;
    std::uint64_t current_match_ = 0;
    std::uint32_t bit_context_ = 1;
    float steady_divisor_ = 0.0F;
    std::uint8_t current_byte_ = 0;
    std::uint8_t bit_mask_ = 128;
    std::uint8_t match_length_ = 0;
    std::uint8_t match_context_ = 0;
    std::uint8_t last_completed_byte_ = 0;
    bool last_observation_matched_ = false;
    bool has_completed_byte_ = false;
};

}  // namespace hz::cmix
