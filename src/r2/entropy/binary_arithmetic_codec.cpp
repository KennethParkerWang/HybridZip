#include "r2/entropy/binary_arithmetic_codec.h"

#include <stdexcept>

#include "FrequencyTable.hpp"
#include "core/types.h"

namespace hz::r2 {
namespace {

class BinaryFrequencyTable final : public FrequencyTable {
public:
    BinaryFrequencyTable(const std::uint32_t p1,
                         const std::uint32_t scale)
        : p1_(p1), scale_(scale) {
        if (scale_ < 2U || p1_ == 0U || p1_ >= scale_) {
            throw std::invalid_argument(
                "Binary arithmetic probability must reserve both symbols");
        }
    }

    std::uint32_t getSymbolLimit() const override { return 2U; }

    std::uint32_t get(const std::uint32_t symbol) const override {
        validate_symbol(symbol);
        return symbol == 0U ? scale_ - p1_ : p1_;
    }

    void set(std::uint32_t, std::uint32_t) override {
        throw std::logic_error("Binary frequency table is immutable");
    }

    void increment(std::uint32_t) override {
        throw std::logic_error("Binary frequency table is immutable");
    }

    std::uint32_t getTotal() const override { return scale_; }

    std::uint32_t getLow(const std::uint32_t symbol) const override {
        validate_symbol(symbol);
        return symbol == 0U ? 0U : scale_ - p1_;
    }

    std::uint32_t getHigh(const std::uint32_t symbol) const override {
        validate_symbol(symbol);
        return symbol == 0U ? scale_ - p1_ : scale_;
    }

private:
    static void validate_symbol(const std::uint32_t symbol) {
        if (symbol >= 2U) {
            throw std::out_of_range(
                "Binary frequency table symbol is out of range");
        }
    }

    std::uint32_t p1_;
    std::uint32_t scale_;
};

}  // namespace

BinaryArithmeticEncoderStream::BinaryArithmeticEncoderStream(
    std::ostream& output)
    : bit_output_(output), encoder_(kCoderStateBits, bit_output_) {}

void BinaryArithmeticEncoderStream::write_bit(
    const std::uint32_t p1,
    const std::uint32_t scale,
    const std::uint8_t bit) {
    if (finished_) {
        throw std::logic_error(
            "Cannot write after binary arithmetic coder finish");
    }
    if (bit > 1U) {
        throw std::invalid_argument(
            "Binary arithmetic coder accepts only bits 0 and 1");
    }
    const BinaryFrequencyTable frequencies(p1, scale);
    encoder_.write(frequencies, bit);
}

void BinaryArithmeticEncoderStream::finish() {
    if (!finished_) {
        encoder_.finish();
        bit_output_.finish();
        finished_ = true;
    }
}

BinaryArithmeticDecoderStream::BinaryArithmeticDecoderStream(
    std::istream& input)
    : bit_input_(input), decoder_(kCoderStateBits, bit_input_) {}

std::uint8_t BinaryArithmeticDecoderStream::read_bit(
    const std::uint32_t p1,
    const std::uint32_t scale) {
    const BinaryFrequencyTable frequencies(p1, scale);
    const std::uint32_t bit = decoder_.read(frequencies);
    if (bit > 1U) {
        throw std::runtime_error(
            "Binary arithmetic decoder returned a non-bit symbol");
    }
    return static_cast<std::uint8_t>(bit);
}

}  // namespace hz::r2
