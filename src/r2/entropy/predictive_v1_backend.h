#pragma once

#include <cstdint>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

class PredictiveV1Backend final : public IBlockEntropyBackend {
public:
    explicit PredictiveV1Backend(std::uint64_t model_seed)
        : model_seed_(model_seed) {}

    const char* name() const noexcept override { return "predictive-v1"; }
    EntropyKind kind() const noexcept override {
        return EntropyKind::SymbolArithmetic;
    }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;

private:
    std::uint64_t model_seed_;
};

}  // namespace hz::r2

