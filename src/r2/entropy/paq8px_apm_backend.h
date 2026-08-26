#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

// PAQ8px APM1 calibration over the existing decoder-synchronised V1 model.
class Paq8pxApmBackend final : public IBlockEntropyBackend {
public:
    static constexpr std::uint32_t kProbabilityScale = 1U << 24U;

    explicit Paq8pxApmBackend(std::uint64_t model_seed)
        : model_seed_(model_seed) {}

    const char* name() const noexcept override { return "paq8px-apm1"; }
    EntropyKind kind() const noexcept override {
        return EntropyKind::Paq8pxApm;
    }
    std::vector<std::uint8_t> encode(ByteView input) const override;
    std::vector<std::uint8_t> decode(ByteView payload,
                                     std::size_t expected_size) const override;

    static std::size_t maximum_payload_size(std::size_t input_size);

private:
    std::uint64_t model_seed_;
};

}  // namespace hz::r2
