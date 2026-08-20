#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace hz::cmix {

// Standalone byte-level adapter around the PPMd model shipped with cmix v21.
class PpmdCore {
public:
    static constexpr std::size_t kSymbolCount = 256;
    using WeightArray = std::array<std::uint32_t, kSymbolCount>;

    PpmdCore(int max_order, std::size_t memory_bytes);
    ~PpmdCore();

    PpmdCore(const PpmdCore&) = delete;
    PpmdCore& operator=(const PpmdCore&) = delete;
    PpmdCore(PpmdCore&&) noexcept;
    PpmdCore& operator=(PpmdCore&&) noexcept;

    void reset();
    void predict(WeightArray& out);
    void observe(std::uint8_t symbol);
    std::size_t context_depth() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace hz::cmix
