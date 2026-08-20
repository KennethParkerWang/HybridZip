#include "predictors/ngram_predictor.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

#include "core/byte_history.h"
#include "core/profile.h"

namespace hz {
namespace {

constexpr std::size_t kMaximumPackedOrder = 4;
constexpr std::size_t kBaseMemoryCharge = 4U * 1024U;
constexpr std::size_t kContextMemoryCharge = 96U;
constexpr std::size_t kContinuationMemoryCharge = 16U;

}  // namespace

NGramPredictor::NGramPredictor(const Profile& profile)
    : max_order_(profile.ngram_max_order),
      memory_limit_bytes_(profile.ngram_memory_bytes),
      lambda_bias_(profile.ngram_lambda_bias) {
    if (max_order_ > kMaximumPackedOrder || memory_limit_bytes_ == 0 ||
        !std::isfinite(lambda_bias_) || lambda_bias_ <= 0.0) {
        throw std::invalid_argument("Invalid NGram predictor configuration");
    }
    reset(profile.model_seed);
}

void NGramPredictor::reset(const std::uint64_t seed) {
    (void)seed;
    unigram_counts_.fill(0);
    unigram_total_ = 0;

    ContextMap empty;
    empty.max_load_factor(contexts_.max_load_factor());
    contexts_.swap(empty);

    accounted_memory_bytes_ = kBaseMemoryCharge;
    context_creation_frozen_ =
        accounted_memory_bytes_ >= memory_limit_bytes_;
}

void NGramPredictor::predict(const ByteHistory& history, ProbVector& out) {
    const double unigram_denominator =
        static_cast<double>(unigram_total_) + static_cast<double>(kAlphabet);
    for (std::size_t symbol = 0; symbol < kAlphabet; ++symbol) {
        out[symbol] =
            (static_cast<double>(unigram_counts_[symbol]) + 1.0) /
            unigram_denominator;
    }

    const std::size_t available_order =
        std::min(max_order_, history.size());
    for (std::size_t order = 1; order <= available_order; ++order) {
        const auto found = contexts_.find(make_context_key(history, order));
        if (found == contexts_.end() || found->second.total == 0) {
            continue;
        }

        const ContextCounts& context = found->second;
        const double total = static_cast<double>(context.total);
        const double lambda = total / (total + lambda_bias_);
        const double backoff_weight = 1.0 - lambda;
        for (double& probability : out) {
            probability *= backoff_weight;
        }
        for (const Continuation& continuation : context.continuations) {
            out[continuation.symbol] +=
                lambda * static_cast<double>(continuation.count) / total;
        }
    }
}

void NGramPredictor::update(const std::uint8_t actual,
                            const ByteHistory& history) {
    increment_unigram(actual);

    const std::size_t available_order =
        std::min(max_order_, history.size());
    for (std::size_t order = 1; order <= available_order; ++order) {
        const std::uint64_t key = make_context_key(history, order);
        const auto found = contexts_.find(key);
        if (found != contexts_.end()) {
            increment_context(found->second, actual);
            continue;
        }

        if (!can_create_context()) {
            context_creation_frozen_ = true;
            continue;
        }

        ContextCounts context;
        context.total = 1;
        context.continuations.push_back(Continuation{actual, 1});

        const auto inserted = contexts_.emplace(key, std::move(context));
        if (inserted.second) {
            account_context_insertion();
        }
    }
}

std::uint64_t NGramPredictor::make_context_key(const ByteHistory& history,
                                               const std::size_t order) {
    std::uint64_t packed = 0;
    for (std::size_t distance = order; distance > 0; --distance) {
        packed = (packed << 8U) | history.back(distance);
    }
    return (static_cast<std::uint64_t>(order) << 32U) | packed;
}

void NGramPredictor::increment_unigram(const std::uint8_t symbol) {
    if (unigram_total_ == std::numeric_limits<std::uint64_t>::max()) {
        unigram_total_ = 0;
        for (std::uint64_t& count : unigram_counts_) {
            count = count / 2U + count % 2U;
            unigram_total_ += count;
        }
    }
    ++unigram_counts_[symbol];
    ++unigram_total_;
}

void NGramPredictor::increment_context(ContextCounts& context,
                                       const std::uint8_t symbol) {
    if (context.total == std::numeric_limits<std::uint64_t>::max()) {
        context.total = 0;
        for (Continuation& continuation : context.continuations) {
            continuation.count =
                continuation.count / 2U + continuation.count % 2U;
            context.total += continuation.count;
        }
    }

    for (Continuation& continuation : context.continuations) {
        if (continuation.symbol == symbol) {
            ++continuation.count;
            ++context.total;
            return;
        }
    }

    context.continuations.push_back(Continuation{symbol, 1});
    account_continuation_insertion();
    ++context.total;
}

bool NGramPredictor::can_create_context() const noexcept {
    constexpr std::size_t minimum_insertion_bytes =
        kContextMemoryCharge + kContinuationMemoryCharge;
    return !context_creation_frozen_ &&
           accounted_memory_bytes_ < memory_limit_bytes_ &&
           memory_limit_bytes_ - accounted_memory_bytes_ >=
               minimum_insertion_bytes;
}

void NGramPredictor::account_context_insertion() noexcept {
    accounted_memory_bytes_ +=
        kContextMemoryCharge + kContinuationMemoryCharge;
    if (accounted_memory_bytes_ >= memory_limit_bytes_) {
        context_creation_frozen_ = true;
    }
}

void NGramPredictor::account_continuation_insertion() noexcept {
    if (accounted_memory_bytes_ >
        std::numeric_limits<std::size_t>::max() -
            kContinuationMemoryCharge) {
        accounted_memory_bytes_ = std::numeric_limits<std::size_t>::max();
    } else {
        accounted_memory_bytes_ += kContinuationMemoryCharge;
    }
    if (accounted_memory_bytes_ >= memory_limit_bytes_) {
        context_creation_frozen_ = true;
    }
}

}  // namespace hz
