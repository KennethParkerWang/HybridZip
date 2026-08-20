#pragma once

#include <cstdint>
#include <iosfwd>

#include "ArithmeticCoder.hpp"
#include "BitIoStream.hpp"
#include "core/types.h"

namespace hz {

class ArithmeticEncoderStream {
public:
    explicit ArithmeticEncoderStream(std::ostream& output);
    ~ArithmeticEncoderStream() = default;

    ArithmeticEncoderStream(const ArithmeticEncoderStream&) = delete;
    ArithmeticEncoderStream& operator=(const ArithmeticEncoderStream&) = delete;

    void write_symbol(const Cdf& cdf, std::uint8_t symbol);
    void finish();

private:
    BitOutputStream bit_output_;
    ArithmeticEncoder encoder_;
    bool finished_ = false;
};

class ArithmeticDecoderStream {
public:
    explicit ArithmeticDecoderStream(std::istream& input);

    ArithmeticDecoderStream(const ArithmeticDecoderStream&) = delete;
    ArithmeticDecoderStream& operator=(const ArithmeticDecoderStream&) = delete;

    std::uint8_t read_symbol(const Cdf& cdf);

private:
    BitInputStream bit_input_;
    ArithmeticDecoder decoder_;
};

}  // namespace hz
