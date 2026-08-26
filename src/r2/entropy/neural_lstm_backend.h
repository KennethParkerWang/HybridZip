#pragma once

#include <cstdint>
#include <vector>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

constexpr std::uint32_t kSharedNeuralModelId = 0x31564C53U; // "SLV1"
constexpr std::uint64_t kSharedNeuralModelSeed = 0x4D4F44454C5631A5ULL;

// Self-contained neural runtime: the fixed Online LSTM configuration and
// model seed are part of the HZ02 neural block contract.
class NeuralLstmBackend final : public IBlockEntropyBackend {
public:
    explicit NeuralLstmBackend(std::uint64_t model_seed)
        : model_seed_(model_seed) {}

    const char* name() const noexcept override { return "neural-lstm-v1"; }
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
