#include "apm1.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace hz::r2::paq8px {
namespace {

int squash_uncached(const int distance) {
    if (distance < -2047) {
        return 1;
    }
    if (distance > 2047) {
        return 4095;
    }
    float probability =
        1.0f / (1.0f + std::exp(-static_cast<float>(distance) / 256.0f));
    probability *= 4096.0f;
    return std::clamp(static_cast<int>(std::round(probability)), 1, 4095);
}

}  // namespace

Apm1::Apm1(const std::uint32_t context_count, const std::uint32_t rate)
    : context_count_(context_count), rate_(rate),
      table_(static_cast<std::size_t>(context_count) * 33U) {
    if (context_count_ == 0U || rate_ == 0U || rate_ >= 32U) {
        throw std::invalid_argument("invalid PAQ8px APM1 dimensions");
    }
    // This is the donor's initialization: context 0 is the probability curve,
    // all other contexts start with the same curve.
    for (std::uint32_t context = 0; context < context_count_; ++context) {
        for (std::uint32_t point = 0; point < 33U; ++point) {
            if (context == 0U) {
                table_[point] = static_cast<std::uint16_t>(
                    squash((static_cast<int>(point) - 16) * 128) * 16);
            } else {
                table_[static_cast<std::size_t>(context) * 33U + point] =
                    table_[point];
            }
        }
    }
}

int Apm1::squash(const int distance) {
    return squash_uncached(distance);
}

int Apm1::stretch(const std::uint32_t probability) {
    if (probability > 4095U) {
        throw std::invalid_argument("PAQ8px APM1 probability out of range");
    }
    const float p = static_cast<float>(std::max(1U, probability)) / 4096.0f;
    return std::clamp(static_cast<int>(std::round(
                           std::log(p / (1.0f - p)) * 256.0f)),
                      -2047, 2047);
}

std::uint32_t Apm1::predict(const std::uint32_t probability,
                            const std::uint32_t context) {
    if (probability >= 4096U || context >= context_count_) {
        throw std::invalid_argument("invalid PAQ8px APM1 prediction query");
    }
    const int stretched = stretch(probability);
    const int weight = stretched & 127;
    const std::size_t base =
        static_cast<std::size_t>(context) * 33U +
        static_cast<std::size_t>((stretched + 2048) >> 7);
    index_ = static_cast<std::uint32_t>(base);
    const std::uint32_t result =
        (static_cast<std::uint32_t>(table_[base]) * (128U - weight) +
         static_cast<std::uint32_t>(table_[base + 1U]) * weight) >> 11U;
    return std::clamp(result, 1U, 4095U);
}

void Apm1::update(const std::uint8_t bit) {
    if (bit > 1U) {
        throw std::invalid_argument("PAQ8px APM1 accepts only binary updates");
    }
    const std::int64_t target =
        (static_cast<std::int64_t>(bit) << 16U) +
        (static_cast<std::int64_t>(bit) << rate_) - bit - bit;
    for (const std::size_t offset : {static_cast<std::size_t>(index_),
                                     static_cast<std::size_t>(index_) + 1U}) {
        const std::int64_t current = table_[offset];
        const std::int64_t next = current + ((target - current) >> rate_);
        table_[offset] = static_cast<std::uint16_t>(std::clamp<std::int64_t>(
            next, 0, std::numeric_limits<std::uint16_t>::max()));
    }
}

}  // namespace hz::r2::paq8px
