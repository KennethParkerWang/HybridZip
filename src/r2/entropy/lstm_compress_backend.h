#pragma once

#include <cstdint>
#include <vector>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

// The standalone lstm-compress donor uses a smaller 90-cell, 3-layer,
// horizon-10 online model.  The archive carries the model identity through
// the block mode and the model seed through the HZ02 archive header.
class LstmCompressBackend final : public IBlockEntropyBackend {
public:
    explicit LstmCompressBackend(std::uint64_t model_seed)
        : model_seed_(model_seed) {}

    const char* name() const noexcept override { return "lstm-compress-v1"; }
    EntropyKind kind() const noexcept override {
        return EntropyKind::SymbolArithmetic;
    }
    static std::size_t maximum_payload_size(std::size_t input_size);
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;

private:
    std::uint64_t model_seed_;
};

}  // namespace hz::r2
