#include "entropy/cdf_frequency_table.h"

#include <stdexcept>

namespace hz {

CdfFrequencyTable::CdfFrequencyTable(const Cdf& cdf) noexcept : cdf_(cdf) {}

std::uint32_t CdfFrequencyTable::getSymbolLimit() const {
    return static_cast<std::uint32_t>(kAlphabet);
}

std::uint32_t CdfFrequencyTable::get(const std::uint32_t symbol) const {
    check_symbol(symbol);
    return cdf_.v[symbol + 1] - cdf_.v[symbol];
}

void CdfFrequencyTable::set(std::uint32_t, std::uint32_t) {
    throw std::logic_error("CDF frequency table is immutable");
}

void CdfFrequencyTable::increment(std::uint32_t) {
    throw std::logic_error("CDF frequency table is immutable");
}

std::uint32_t CdfFrequencyTable::getTotal() const {
    return cdf_.v.back();
}

std::uint32_t CdfFrequencyTable::getLow(const std::uint32_t symbol) const {
    check_symbol(symbol);
    return cdf_.v[symbol];
}

std::uint32_t CdfFrequencyTable::getHigh(const std::uint32_t symbol) const {
    check_symbol(symbol);
    return cdf_.v[symbol + 1];
}

void CdfFrequencyTable::check_symbol(const std::uint32_t symbol) const {
    if (symbol >= kAlphabet) {
        throw std::out_of_range("Arithmetic symbol is outside byte alphabet");
    }
}

}  // namespace hz
