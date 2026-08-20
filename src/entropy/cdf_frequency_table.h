#pragma once

#include <cstdint>

#include "FrequencyTable.hpp"
#include "core/types.h"

namespace hz {

class CdfFrequencyTable final : public FrequencyTable {
public:
    explicit CdfFrequencyTable(const Cdf& cdf) noexcept;

    std::uint32_t getSymbolLimit() const override;
    std::uint32_t get(std::uint32_t symbol) const override;
    void set(std::uint32_t symbol, std::uint32_t frequency) override;
    void increment(std::uint32_t symbol) override;
    std::uint32_t getTotal() const override;
    std::uint32_t getLow(std::uint32_t symbol) const override;
    std::uint32_t getHigh(std::uint32_t symbol) const override;

private:
    void check_symbol(std::uint32_t symbol) const;

    const Cdf& cdf_;
};

}  // namespace hz
