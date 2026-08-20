#pragma once

#include <cstdint>
#include <iosfwd>

#include "ArithmeticCoder.hpp"
#include "BitIoStream.hpp"

namespace hz::r2 {

class BinaryArithmeticEncoderStream {
public:
    explicit BinaryArithmeticEncoderStream(std::ostream& output);

    BinaryArithmeticEncoderStream(const BinaryArithmeticEncoderStream&) = delete;
    BinaryArithmeticEncoderStream& operator=(
        const BinaryArithmeticEncoderStream&) = delete;

    void write_bit(std::uint32_t p1,
                   std::uint32_t scale,
                   std::uint8_t bit);
    void finish();

private:
    BitOutputStream bit_output_;
    ArithmeticEncoder encoder_;
    bool finished_ = false;
};

class BinaryArithmeticDecoderStream {
public:
    explicit BinaryArithmeticDecoderStream(std::istream& input);

    BinaryArithmeticDecoderStream(const BinaryArithmeticDecoderStream&) = delete;
    BinaryArithmeticDecoderStream& operator=(
        const BinaryArithmeticDecoderStream&) = delete;

    std::uint8_t read_bit(std::uint32_t p1, std::uint32_t scale);

private:
    BitInputStream bit_input_;
    ArithmeticDecoder decoder_;
};

}  // namespace hz::r2
