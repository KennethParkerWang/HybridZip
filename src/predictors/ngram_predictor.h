#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "predictors/predictor.h"

namespace hz {

struct Profile;

class NGramPredictor final : public Predictor {
public:
    explicit NGramPredictor(const Profile& profile);

    void reset(std::uint64_t seed) override;
    void predict(const ByteHistory& history, ProbVector& out) override;
    void update(std::uint8_t actual, const ByteHistory& history) override;

private:
    struct Continuation {
        std::uint8_t symbol = 0;
        std::uint64_t count = 0;
    };

    struct ContextCounts {
        std::uint64_t total = 0;
        std::vector<Continuation> continuations;
    };

    using ContextMap = std::unordered_map<std::uint64_t, ContextCounts>;

    static std::uint64_t make_context_key(const ByteHistory& history,
                                          std::size_t order);

    void increment_unigram(std::uint8_t symbol);
    void increment_context(ContextCounts& context, std::uint8_t symbol);
    bool can_create_context() const noexcept;
    void account_context_insertion() noexcept;
    void account_continuation_insertion() noexcept;

    std::size_t max_order_;
    std::size_t memory_limit_bytes_;
    double lambda_bias_;

    std::array<std::uint64_t, kAlphabet> unigram_counts_{};
    std::uint64_t unigram_total_ = 0;
    ContextMap contexts_;
    std::size_t accounted_memory_bytes_ = 0;
    bool context_creation_frozen_ = false;
};

}  // namespace hz
