#pragma once

#include <cstddef>
#include <cstdint>

#include "core/types.h"
#include "r2/core/evidence.h"
#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

struct MatchBitProbability {
    std::uint32_t p1 = 0;
    bool active = false;
};

class DonorMatchPredictiveBackend final : public IBlockEntropyBackend {
public:
    static constexpr std::uint32_t kProbabilityScale = 1U << 24U;
    static constexpr std::uint32_t kMinimumBitFrequency =
        kProbabilityScale / 16U;

    explicit DonorMatchPredictiveBackend(std::uint64_t model_seed)
        : model_seed_(model_seed) {}

    const char* name() const noexcept override {
        return "donor-match-predictive";
    }
    EntropyKind kind() const noexcept override {
        return EntropyKind::SymbolArithmetic;
    }

    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(
        ByteView payload,
        std::size_t expected_size) const override;

    static std::uint32_t conditional_v1_p1(
        const Cdf& cdf,
        std::uint8_t prefix_length,
        std::uint8_t prefix_value);
    static MatchBitProbability conditional_match_p1(
        const MatchEvidence& evidence,
        std::uint8_t prefix_length,
        std::uint8_t prefix_value);
    static std::uint32_t fuse_p1(
        std::uint32_t v1_p1,
        std::uint32_t cmix_p1,
        MatchBitProbability paq);

    static std::size_t maximum_payload_size(std::size_t input_size);

private:
    std::uint64_t model_seed_;
};

}  // namespace hz::r2
