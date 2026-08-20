#include "entropy/arithmetic_codec.h"

#include <stdexcept>

#include "entropy/cdf_frequency_table.h"

namespace hz {

ArithmeticEncoderStream::ArithmeticEncoderStream(std::ostream& output)
    : bit_output_(output), encoder_(kCoderStateBits, bit_output_) {}

void ArithmeticEncoderStream::write_symbol(const Cdf& cdf,
                                           const std::uint8_t symbol) {
    if (finished_) {
        throw std::logic_error("Cannot write after arithmetic coder finish");
    }
    const CdfFrequencyTable frequencies(cdf);
    encoder_.write(frequencies, symbol);
}

void ArithmeticEncoderStream::finish() {
    if (!finished_) {
        encoder_.finish();
        bit_output_.finish();
        finished_ = true;
    }
}

ArithmeticDecoderStream::ArithmeticDecoderStream(std::istream& input)
    : bit_input_(input), decoder_(kCoderStateBits, bit_input_) {}

std::uint8_t ArithmeticDecoderStream::read_symbol(const Cdf& cdf) {
    const CdfFrequencyTable frequencies(cdf);
    const std::uint32_t symbol = decoder_.read(frequencies);
    if (symbol >= kAlphabet) {
        throw std::runtime_error("Arithmetic decoder returned a non-byte symbol");
    }
    return static_cast<std::uint8_t>(symbol);
}

}  // namespace hz
