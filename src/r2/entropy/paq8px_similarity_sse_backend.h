#pragma once

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

// Complete PAQ8px SimilarityModelPair followed by the donor SSE calibrator.
class Paq8pxSimilaritySseBackend final : public IBlockEntropyBackend {
public:
    const char* name() const noexcept override {
        return "paq8px-similarity-sse";
    }
    EntropyKind kind() const noexcept override {
        return EntropyKind::Paq8pxSimilaritySse;
    }

    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(
        ByteView payload, std::size_t expected_size) const override;

    static std::size_t maximum_payload_size(std::size_t input_size);
};

}  // namespace hz::r2
