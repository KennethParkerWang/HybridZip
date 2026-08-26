#include "r2/entropy/lmic_arithmetic_backend.h"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>

#include "core/types.h"
#include "r2/entropy/bgpt_shared_prior_data.h"

namespace hz::r2 {
namespace {

constexpr std::uint64_t kBase = 2U;
constexpr std::uint64_t kPrecision = 32U;
constexpr std::uint64_t kBaseToPm1 = std::uint64_t{1} << 31U;
constexpr std::uint64_t kBaseToPm2 = std::uint64_t{1} << 30U;
constexpr std::uint64_t kInitialHigh = (std::uint64_t{1} << 32U) - 1U;
constexpr std::array<std::uint8_t, 4> kPayloadMagic{{'H', 'L', 'M', '1'}};

class BitWriter {
public:
    void write(const std::uint8_t bit) { bits_.push_back(bit != 0U); }

    std::vector<std::uint8_t> finish(std::uint8_t& padded_bits) const {
        padded_bits = static_cast<std::uint8_t>((8U - bits_.size() % 8U) % 8U);
        const std::size_t total_bits = bits_.size() + padded_bits;
        std::vector<std::uint8_t> bytes((total_bits + 7U) / 8U, 0U);
        for (std::size_t index = 0; index < bits_.size(); ++index) {
            const std::size_t bit_index = padded_bits + index;
            bytes[bit_index / 8U] = static_cast<std::uint8_t>(
                bytes[bit_index / 8U] |
                (static_cast<std::uint8_t>(bits_[index]) <<
                 (7U - bit_index % 8U)));
        }
        return bytes;
    }

private:
    std::vector<bool> bits_;
};

class BitReader {
public:
    BitReader(ByteView bytes, const std::uint8_t padded_bits)
        : bytes_(bytes), bit_index_(padded_bits) {
        if (padded_bits > 7U ||
            (bytes.size() == 0U && padded_bits != 0U) ||
            (bytes.size() != 0U && padded_bits >= bytes.size() * 8U)) {
            throw std::runtime_error("LMIC bitstream padding is malformed");
        }
    }

    std::uint8_t read() {
        if (bit_index_ >= bytes_.size() * 8U) {
            return 1U;  // LMIC decoder padding uses base-1 digits.
        }
        const std::uint8_t bit = static_cast<std::uint8_t>(
            (bytes_[bit_index_ / 8U] >> (7U - bit_index_ % 8U)) & 1U);
        ++bit_index_;
        return bit;
    }

private:
    ByteView bytes_;
    std::size_t bit_index_;
};

using Intervals = std::array<std::uint64_t, 257U>;

Intervals intervals_for_context(const std::size_t context,
                                const std::uint64_t width) {
    if (context >= kBgptSharedPriorContextCount || width == 0U) {
        throw std::runtime_error("LMIC frozen-prior context is invalid");
    }
    const std::uint16_t* const frequencies =
        bgpt_shared_prior_frequencies() + context * hz::kAlphabet;
    Intervals qcpdf{};
    std::uint64_t cumulative = 0U;
    qcpdf[0] = 0U;
    for (std::size_t symbol = 0; symbol < hz::kAlphabet; ++symbol) {
        const std::uint64_t frequency = frequencies[symbol];
        if (frequency == 0U) {
            throw std::runtime_error(
                "LMIC frozen prior contains a zero probability");
        }
        cumulative += frequency;
        // LMIC's numpy implementation floors the cumulative probability after
        // scaling it to the current inclusive arithmetic interval width.
        qcpdf[symbol + 1U] =
            (cumulative * width) / kBgptSharedPriorFrequencyTotal;
        if (qcpdf[symbol + 1U] <= qcpdf[symbol]) {
            throw std::runtime_error(
                "LMIC probability lost a quantised bin at context " +
                std::to_string(context) + ", symbol " +
                std::to_string(symbol) + ", width " +
                std::to_string(width) + ", frequency " +
                std::to_string(frequency));
        }
    }
    if (qcpdf.back() > width) {
        throw std::runtime_error("LMIC probability sum exceeds interval");
    }
    return qcpdf;
}

std::uint64_t shift_left(const std::uint64_t value) {
    return (value % kBaseToPm1) * kBase;
}

std::uint64_t shift_left_keeping_msd(const std::uint64_t value) {
    // Keep the most-significant base digit (the top 31-bit prefix) in place
    // and shift the remaining precision-2 digits left by one base digit.
    // This mirrors the donor's `x - (x % base_to_pm1)` expression.
    return value - (value % kBaseToPm1) + (value % kBaseToPm2) * kBase;
}

class LmicEncoder {
public:
    explicit LmicEncoder(BitWriter& output) : output_(output) {}

    std::uint64_t width() const noexcept { return high_ - low_ + 1U; }

    void encode(const Intervals& qcpdf, const std::size_t symbol) {
        if (symbol >= hz::kAlphabet) {
            throw std::runtime_error("LMIC symbol is out of range");
        }
        const std::uint64_t low_pre_split = low_;
        low_ = low_pre_split + qcpdf[symbol];
        high_ = low_pre_split + qcpdf[symbol + 1U] - 1U;
        if (low_ > high_) {
            throw std::runtime_error(
                "LMIC interval inverted at symbol " + std::to_string(symbol) +
                ", low " + std::to_string(low_) + ", high " +
                std::to_string(high_) + ", base low " +
                std::to_string(low_pre_split) + ", qlow " +
                std::to_string(qcpdf[symbol]) + ", qhigh " +
                std::to_string(qcpdf[symbol + 1U]));
        }
        normalize_matching(low_pre_split);
        normalize_carry();
    }

    void terminate() {
        output_.write(static_cast<std::uint8_t>(low_ / kBaseToPm1));
        for (std::size_t index = 0; index < carry_digits_; ++index) {
            output_.write(1U);
        }
    }

private:
    void normalize_matching(const std::uint64_t low_pre_split) {
        while (low_ / kBaseToPm1 == high_ / kBaseToPm1) {
            const std::uint64_t low_msd = low_ / kBaseToPm1;
            output_.write(static_cast<std::uint8_t>(low_msd));
            const std::uint64_t carry_digit =
                (1U + low_msd - low_pre_split / kBaseToPm1) % kBase;
            while (carry_digits_ != 0U) {
                output_.write(static_cast<std::uint8_t>(carry_digit));
                --carry_digits_;
            }
            low_ = shift_left(low_);
            high_ = shift_left(high_) + kBase - 1U;
        }
    }

    void normalize_carry() {
        while (low_ / kBaseToPm2 + 1U == high_ / kBaseToPm2) {
            ++carry_digits_;
            low_ = shift_left_keeping_msd(low_);
            high_ = shift_left_keeping_msd(high_) + kBase - 1U;
        }
    }

    BitWriter& output_;
    std::uint64_t low_ = 0U;
    std::uint64_t high_ = kInitialHigh;
    std::size_t carry_digits_ = 0U;
};

class LmicDecoder {
public:
    explicit LmicDecoder(BitReader& input) : input_(input) {
        for (std::size_t index = 0; index < kPrecision; ++index) {
            code_ = code_ * kBase + input_.read();
        }
    }

    std::uint64_t width() const noexcept { return high_ - low_ + 1U; }

    std::size_t decode(const Intervals& qcpdf) {
        if (code_ < low_ || code_ > high_) {
            throw std::runtime_error("LMIC arithmetic code is outside interval");
        }
        // `qcpdf` is relative to the current low.  The donor searches an
        // absolute CDF (`low + qcpdf`), so subtract low before searching.
        const std::uint64_t relative_code = code_ - low_;
        const auto symbol_it = std::upper_bound(
            qcpdf.begin(), qcpdf.end(), relative_code);
        if (symbol_it == qcpdf.begin() || symbol_it == qcpdf.end()) {
            throw std::runtime_error("LMIC arithmetic code is outside CDF");
        }
        const std::size_t symbol =
            static_cast<std::size_t>(symbol_it - qcpdf.begin() - 1);
        const std::uint64_t low_pre_split = low_;
        low_ = low_pre_split + qcpdf[symbol];
        high_ = low_pre_split + qcpdf[symbol + 1U] - 1U;
        normalize_matching();
        normalize_carry();
        return symbol;
    }

private:
    void normalize_matching() {
        while (low_ / kBaseToPm1 == high_ / kBaseToPm1) {
            code_ = shift_left(code_) + input_.read();
            low_ = shift_left(low_);
            high_ = shift_left(high_) + kBase - 1U;
        }
    }

    void normalize_carry() {
        while (low_ / kBaseToPm2 + 1U == high_ / kBaseToPm2) {
            code_ = shift_left_keeping_msd(code_) + input_.read();
            low_ = shift_left_keeping_msd(low_);
            high_ = shift_left_keeping_msd(high_) + kBase - 1U;
        }
    }

    BitReader& input_;
    std::uint64_t low_ = 0U;
    std::uint64_t high_ = kInitialHigh;
    std::uint64_t code_ = 0U;
};

}  // namespace

std::vector<std::uint8_t> LmicArithmeticBackend::encode(
    const ByteView input) const {
    BitWriter bits;
    LmicEncoder coder(bits);
    std::size_t context = kBgptSharedPriorStartContext;
    for (std::size_t index = 0; index < input.size(); ++index) {
        const Intervals qcpdf = intervals_for_context(context, coder.width());
        coder.encode(qcpdf, input[index]);
        context = input[index];
    }
    coder.terminate();

    std::uint8_t padded_bits = 0U;
    const std::vector<std::uint8_t> stream = bits.finish(padded_bits);
    std::vector<std::uint8_t> payload;
    payload.reserve(kLmicArithmeticPayloadHeaderSize + stream.size());
    payload.insert(payload.end(), kPayloadMagic.begin(), kPayloadMagic.end());
    payload.push_back(1U);  // framing version
    payload.push_back(padded_bits);
    payload.push_back(2U);  // base-2
    payload.push_back(32U); // precision-32
    payload.insert(payload.end(), stream.begin(), stream.end());
    return payload;
}

std::vector<std::uint8_t> LmicArithmeticBackend::decode(
    const ByteView payload, const std::size_t expected_size) const {
    if (payload.size() < kLmicArithmeticPayloadHeaderSize ||
        !std::equal(kPayloadMagic.begin(), kPayloadMagic.end(), payload.data()) ||
        payload[4] != 1U || payload[6] != 2U || payload[7] != 32U) {
        throw std::runtime_error("LMIC arithmetic payload header is invalid");
    }
    const std::uint8_t padded_bits = payload[5];
    const ByteView stream(payload.data() + kLmicArithmeticPayloadHeaderSize,
                          payload.size() - kLmicArithmeticPayloadHeaderSize);
    BitReader bits(stream, padded_bits);
    LmicDecoder coder(bits);
    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    std::size_t context = kBgptSharedPriorStartContext;
    for (std::size_t index = 0; index < expected_size; ++index) {
        const Intervals qcpdf = intervals_for_context(context, coder.width());
        const std::size_t symbol = coder.decode(qcpdf);
        output.push_back(static_cast<std::uint8_t>(symbol));
        context = symbol;
    }
    return output;
}

std::size_t LmicArithmeticBackend::maximum_payload_size(
    const std::size_t input_size) {
    if (input_size > (std::numeric_limits<std::size_t>::max() - 64U) / 16U) {
        throw std::runtime_error("LMIC payload bound overflow");
    }
    return input_size * 16U + 64U;
}

}  // namespace hz::r2
