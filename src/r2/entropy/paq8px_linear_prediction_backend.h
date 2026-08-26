#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

// Complete PAQ8px LinearPredictionModel adapted to HZ02's bit stream.
class Paq8pxLinearPredictionBackend final : public IBlockEntropyBackend {
public:
    const char* name() const noexcept override {
        return "paq8px-linear-prediction";
    }
    EntropyKind kind() const noexcept override {
        return EntropyKind::Paq8pxLinearPrediction;
    }

    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;

    static std::size_t maximum_payload_size(std::size_t input_size);
};

}  // namespace hz::r2
