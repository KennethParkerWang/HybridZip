#include "match_core.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace hz::cmix {
namespace {

void validate_config(const MatchConfig& config) {
    if (config.history_size == 0) {
        throw std::invalid_argument("cmix Match history size must be positive");
    }
    if (config.map_entries == 0) {
        throw std::invalid_argument("cmix Match map size must be positive");
    }
    if (config.history_size >
        static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
        throw std::invalid_argument(
            "cmix Match history exceeds the donor's 32-bit map positions");
    }
    if (config.hash_bits == 0 || config.hash_bits > 31) {
        throw std::invalid_argument("cmix Match hash bits must be in [1, 31]");
    }
    if (config.context_order != 0 &&
        config.context_order > 63U / config.hash_bits) {
        throw std::invalid_argument(
            "cmix Match context width must be less than 64 bits");
    }
    if (config.limit <= 0) {
        throw std::invalid_argument("cmix Match learning limit must be positive");
    }
    if (!std::isfinite(config.delta) || config.delta <= 0.0F) {
        throw std::invalid_argument(
            "cmix Match learning delta must be finite and positive");
    }
    const float denominator =
        static_cast<float>(config.limit) + config.delta;
    if (!std::isfinite(denominator)) {
        throw std::invalid_argument(
            "cmix Match learning denominator must be finite");
    }
    if (config.map_entries >
        std::numeric_limits<std::size_t>::max() / sizeof(std::uint32_t)) {
        throw std::length_error("cmix Match map allocation overflows size_t");
    }
    const std::size_t map_bytes =
        config.map_entries * sizeof(std::uint32_t);
    if (config.history_size >
        std::numeric_limits<std::size_t>::max() - map_bytes) {
        throw std::length_error("cmix Match allocation overflows size_t");
    }
}

}  // namespace

MatchCore::MatchCore(MatchConfig config) : config_(config) {
    validate_config(config_);

    history_ = std::make_unique<std::uint8_t[]>(config_.history_size);
    map_ = std::make_unique<std::uint32_t[]>(config_.map_entries);

    context_base_ = std::uint64_t{1} << config_.hash_bits;
    const std::uint32_t context_width =
        config_.context_order * config_.hash_bits;
    context_modulus_ = context_width == 0
                           ? std::uint64_t{1}
                           : (std::uint64_t{1} << context_width);
    const float denominator =
        static_cast<float>(config_.limit) + config_.delta;
    steady_divisor_ = static_cast<float>(1.0 / denominator);
    reset();
}

float MatchCore::predict() const noexcept {
    const float confidence = predictions_[match_length_];
    return (current_byte_ & bit_mask_) != 0 ? confidence
                                            : 1.0F - confidence;
}

void MatchCore::observe(const std::uint8_t bit) {
    if (bit > 1) {
        throw std::invalid_argument("cmix Match accepts only bits 0 and 1");
    }

    const std::size_t bucket = match_length_;
    const bool expected_bit = (current_byte_ & bit_mask_) != 0;
    const bool matched = bit == static_cast<std::uint8_t>(expected_bit);
    last_observation_matched_ = matched;
    bit_mask_ = static_cast<std::uint8_t>(bit_mask_ / 2U);

    float divisor = steady_divisor_;
    if (counts_[bucket] < config_.limit) {
        ++counts_[bucket];
        const float denominator =
            static_cast<float>(counts_[bucket]) + config_.delta;
        divisor = static_cast<float>(1.0 / denominator);
    }
    predictions_[bucket] +=
        (static_cast<int>(matched) - predictions_[bucket]) * divisor;

    if (matched) {
        if (match_length_ < 255) {
            ++match_length_;
        }
    } else {
        match_length_ = 0;
    }

    const bool completes_byte = bit_context_ >= 128U;
    std::size_t history_write_index = 0;
    if (completes_byte) {
        map_[current_map_index()] =
            static_cast<std::uint32_t>(history_position_);
        history_write_index = static_cast<std::size_t>(
            history_position_ % config_.history_size);
        ++history_position_;
    }

    bit_context_ += bit_context_ + bit;
    if (!completes_byte) {
        return;
    }

    bit_context_ -= 256U;
    last_completed_byte_ = static_cast<std::uint8_t>(bit_context_);
    has_completed_byte_ = true;
    history_[history_write_index] = last_completed_byte_;
    byte_context_ =
        (byte_context_ * context_base_ + last_completed_byte_) %
        context_modulus_;
    byte_update();
    bit_context_ = 1;
}

void MatchCore::reset() noexcept {
    std::fill_n(history_.get(), config_.history_size, std::uint8_t{0});
    std::fill_n(map_.get(), config_.map_entries, std::uint32_t{0});
    for (std::size_t i = 0; i < predictions_.size(); ++i) {
        predictions_[i] =
            static_cast<float>(0.5 + (static_cast<double>(i) + 0.5) / 512.0);
    }
    counts_.fill(0);

    history_position_ = 0;
    byte_context_ = 0;
    current_match_ = 0;
    bit_context_ = 1;
    current_byte_ = 0;
    bit_mask_ = 128;
    match_length_ = 0;
    match_context_ = 0;
    last_completed_byte_ = 0;
    last_observation_matched_ = false;
    has_completed_byte_ = false;
}

const MatchConfig& MatchCore::config() const noexcept {
    return config_;
}

MatchDiagnostics MatchCore::diagnostics() const noexcept {
    MatchDiagnostics result{};
    result.history_position = history_position_;
    result.byte_context = byte_context_;
    result.current_match = current_match_;
    result.history_write_index = static_cast<std::size_t>(
        history_position_ % config_.history_size);
    result.map_index = current_map_index();
    result.bit_context = bit_context_;
    result.current_byte = current_byte_;
    result.bit_mask = bit_mask_;
    result.match_length = match_length_;
    result.match_context = match_context_;
    result.last_completed_byte = last_completed_byte_;
    result.last_observation_matched = last_observation_matched_;
    result.has_completed_byte = has_completed_byte_;
    return result;
}

float MatchCore::bucket_probability(const std::size_t bucket) const {
    if (bucket >= predictions_.size()) {
        throw std::out_of_range("cmix Match probability bucket is out of range");
    }
    return predictions_[bucket];
}

int MatchCore::bucket_count(const std::size_t bucket) const {
    if (bucket >= counts_.size()) {
        throw std::out_of_range("cmix Match count bucket is out of range");
    }
    return counts_[bucket];
}

std::uint32_t MatchCore::map_entry(const std::size_t index) const {
    if (index >= config_.map_entries) {
        throw std::out_of_range("cmix Match map index is out of range");
    }
    return map_[index];
}

std::uint8_t MatchCore::history_byte(const std::size_t index) const {
    if (index >= config_.history_size) {
        throw std::out_of_range("cmix Match history index is out of range");
    }
    return history_[index];
}

std::size_t MatchCore::allocation_bytes() const noexcept {
    return config_.history_size +
           config_.map_entries * sizeof(std::uint32_t);
}

std::size_t MatchCore::required_allocation_bytes(const MatchConfig& config) {
    validate_config(config);
    return config.history_size +
           config.map_entries * sizeof(std::uint32_t);
}

void MatchCore::byte_update() noexcept {
    if (match_length_ < 8) {
        current_match_ = map_[current_map_index()];
    } else {
        ++current_match_;
    }
    current_match_ %= config_.history_size;
    current_byte_ = history_[static_cast<std::size_t>(current_match_)];
    bit_mask_ = 128;
    match_context_ = static_cast<std::uint8_t>(match_length_ / 32U);
}

std::size_t MatchCore::current_map_index() const noexcept {
    return static_cast<std::size_t>(byte_context_ % config_.map_entries);
}

}  // namespace hz::cmix
