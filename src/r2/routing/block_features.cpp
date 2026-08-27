#include "r2/routing/block_features.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace hz::r2 {
namespace {

constexpr std::size_t kHistogramSize = 256U;
constexpr std::size_t kCoarseHistogramSize = 16U;
constexpr std::size_t kLzHashSlots = 4096U;
constexpr std::size_t kLzSampleStride = 8U;
constexpr std::size_t kLzMatchLimit = 32U;
constexpr std::uint32_t kNoPosition =
    std::numeric_limits<std::uint32_t>::max();

// floor(log2(1 + bucket / 16)) in Q12. The table is fixed so entropy does not
// depend on a runtime math library or floating-point mode.
constexpr std::array<std::uint16_t, 16> kLog2MantissaQ12{{
    0U, 358U, 696U, 1015U, 1318U, 1606U, 1881U, 2145U,
    2396U, 2637U, 2869U, 3092U, 3307U, 3515U, 3715U, 3908U
}};

constexpr std::size_t feature_index(BlockFeatureId id) noexcept {
    return static_cast<std::size_t>(id);
}

std::uint32_t q12_fraction(std::size_t count, std::size_t total) noexcept {
    return total == 0U ? 0U : static_cast<std::uint32_t>(
        (count * static_cast<std::size_t>(kBlockFeatureQ12One)) / total);
}

unsigned floor_log2(std::uint32_t value) noexcept {
#if defined(_MSC_VER)
    unsigned long bit = 0;
    _BitScanReverse(&bit, value);
    return static_cast<unsigned>(bit);
#elif defined(__GNUC__) || defined(__clang__)
    return 31U - static_cast<unsigned>(__builtin_clz(value));
#else
    unsigned result = 0U;
    while (value > 1U) {
        value >>= 1U;
        ++result;
    }
    return result;
#endif
}

std::uint32_t log2_q12(std::uint32_t value) noexcept {
    if (value <= 1U) {
        return 0U;
    }
    const unsigned whole = floor_log2(value);
    const std::uint32_t base = 1U << whole;
    const std::uint32_t bucket = ((value - base) * 16U) / base;
    return whole * kBlockFeatureQ12One +
        kLog2MantissaQ12[bucket < kLog2MantissaQ12.size() ? bucket : 15U];
}

template <std::size_t N>
std::uint32_t entropy_q12(const std::array<std::uint32_t, N>& histogram,
                          std::size_t total) noexcept {
    if (total == 0U) {
        return 0U;
    }
    const std::uint32_t total_log = log2_q12(static_cast<std::uint32_t>(total));
    std::uint64_t weighted_log = 0U;
    for (const std::uint32_t count : histogram) {
        if (count != 0U) {
            weighted_log += static_cast<std::uint64_t>(count) *
                log2_q12(count);
        }
    }
    const std::uint64_t total_q12 =
        static_cast<std::uint64_t>(total) * total_log;
    return static_cast<std::uint32_t>((total_q12 - weighted_log) / total);
}

bool is_printable(std::uint8_t value) noexcept {
    return (value >= 0x20U && value <= 0x7EU) || value == '\n' ||
        value == '\r' || value == '\t';
}

bool is_whitespace(std::uint8_t value) noexcept {
    return value == ' ' || value == '\n' || value == '\r' ||
        value == '\t' || value == '\f' || value == '\v';
}

bool is_markup(std::uint8_t value) noexcept {
    return value == '<' || value == '>' || value == '{' || value == '}' ||
        value == '[' || value == ']' || value == '(' || value == ')' ||
        value == ':' || value == ',' || value == '"' || value == '/';
}

bool is_source_punctuation(std::uint8_t value) noexcept {
    return value == ';' || value == '=' || value == '+' || value == '-' ||
        value == '*' || value == '&' || value == '|' || value == '!' ||
        value == '#' || value == '\\' || value == '_' || value == '~';
}

std::uint32_t block_size_bucket(std::size_t size) noexcept {
    if (size == 32U * 1024U) return 32U;
    if (size == 64U * 1024U) return 64U;
    if (size == 128U * 1024U) return 128U;
    return 0U;
}

std::uint32_t read_u32_le(ByteView input, std::size_t offset) noexcept {
    return static_cast<std::uint32_t>(input[offset]) |
        (static_cast<std::uint32_t>(input[offset + 1U]) << 8U) |
        (static_cast<std::uint32_t>(input[offset + 2U]) << 16U) |
        (static_cast<std::uint32_t>(input[offset + 3U]) << 24U);
}

bool begins_with(ByteView input,
                 std::initializer_list<std::uint8_t> magic) noexcept {
    if (input.size() < magic.size()) return false;
    std::size_t index = 0U;
    for (const std::uint8_t expected : magic) {
        if (input[index++] != expected) return false;
    }
    return true;
}

struct MagicFlags {
    bool known = false;
    bool compressed = false;
};

MagicFlags magic_flags(ByteView input) noexcept {
    const bool png = begins_with(input, {0x89U, 'P', 'N', 'G', 0x0DU, 0x0AU,
                                         0x1AU, 0x0AU});
    const bool jpeg = begins_with(input, {0xFFU, 0xD8U, 0xFFU});
    const bool gif = begins_with(input, {'G', 'I', 'F', '8', '7', 'a'}) ||
        begins_with(input, {'G', 'I', 'F', '8', '9', 'a'});
    const bool elf = begins_with(input, {0x7FU, 'E', 'L', 'F'});
    const bool pdf = begins_with(input, {'%', 'P', 'D', 'F', '-'});
    const bool zip = begins_with(input, {'P', 'K', 0x03U, 0x04U}) ||
        begins_with(input, {'P', 'K', 0x05U, 0x06U}) ||
        begins_with(input, {'P', 'K', 0x07U, 0x08U});
    const bool gzip = begins_with(input, {0x1FU, 0x8BU});
    const bool bzip2 = begins_with(input, {'B', 'Z', 'h'});
    const bool xz = begins_with(input, {0xFDU, '7', 'z', 'X', 'Z', 0x00U});
    const bool seven_zip = begins_with(
        input, {'7', 'z', 0xBCU, 0xAFU, 0x27U, 0x1CU});
    const bool zstd = begins_with(input, {0x28U, 0xB5U, 0x2FU, 0xFDU});
    const bool lz4 = begins_with(input, {0x04U, 0x22U, 0x4DU, 0x18U});
    const bool rar = begins_with(input, {'R', 'a', 'r', '!', 0x1AU, 0x07U});
    const bool compressed = png || jpeg || gif || zip || gzip || bzip2 || xz ||
        seven_zip || zstd || lz4 || rar;
    return {png || jpeg || gif || elf || pdf || zip || gzip || bzip2 || xz ||
                seven_zip || zstd || lz4 || rar,
            compressed};
}

struct Utf8State {
    bool valid = true;
    unsigned remaining = 0U;
    std::uint32_t codepoint = 0U;
    std::uint32_t minimum = 0U;

    void observe(std::uint8_t value) noexcept {
        if (!valid) return;
        if (remaining != 0U) {
            if ((value & 0xC0U) != 0x80U) {
                valid = false;
                return;
            }
            codepoint = (codepoint << 6U) | (value & 0x3FU);
            --remaining;
            if (remaining == 0U &&
                (codepoint < minimum || codepoint > 0x10FFFFU ||
                 (codepoint >= 0xD800U && codepoint <= 0xDFFFU))) {
                valid = false;
            }
            return;
        }
        if (value <= 0x7FU) return;
        if (value >= 0xC2U && value <= 0xDFU) {
            remaining = 1U;
            codepoint = value & 0x1FU;
            minimum = 0x80U;
        } else if (value >= 0xE0U && value <= 0xEFU) {
            remaining = 2U;
            codepoint = value & 0x0FU;
            minimum = 0x800U;
        } else if (value >= 0xF0U && value <= 0xF4U) {
            remaining = 3U;
            codepoint = value & 0x07U;
            minimum = 0x10000U;
        } else {
            valid = false;
        }
    }
};

bool small_delta_element(ByteView input, std::size_t current,
                         std::size_t width) noexcept {
    const std::size_t previous = current - width;
    for (std::size_t byte = 0U; byte < width; ++byte) {
        const std::uint8_t delta = static_cast<std::uint8_t>(
            input[current + byte] - input[previous + byte]);
        if (delta != 0U && delta != 1U && delta != 0xFFU) return false;
    }
    return true;
}

}  // namespace

BlockClass BlockFeaturesV1::classify() const noexcept {
    const std::int32_t printable = (*this)[BlockFeatureId::PrintableFractionQ12];
    const std::int32_t x86 = (*this)[BlockFeatureId::X86RelativeTargetDensityQ12];
    if (x86 >= 82 && printable < 2867) return BlockClass::X86;
    const std::int32_t whitespace = (*this)[BlockFeatureId::WhitespaceFractionQ12];
    const std::int32_t markup = (*this)[BlockFeatureId::MarkupFractionQ12];
    if (printable >= 2867 && (whitespace >= 41 || markup >= 82)) {
        return BlockClass::Text;
    }
    const std::int32_t periodicity = (*this)[BlockFeatureId::BestPeriodicityQ12];
    const std::int32_t zero = (*this)[BlockFeatureId::ZeroFractionQ12];
    const std::int32_t small_delta4 = (*this)[BlockFeatureId::SmallDeltaWidth4Q12];
    const std::int32_t small_delta8 = (*this)[BlockFeatureId::SmallDeltaWidth8Q12];
    if (periodicity >= 492 || zero >= 328 ||
        (small_delta4 > small_delta8 ? small_delta4 : small_delta8) >= 2048) {
        return BlockClass::Numeric;
    }
    return BlockClass::Generic;
}

BlockFeaturesV1 extract_block_features(ByteView input) noexcept {
    BlockFeaturesV1 features{};
    const bool saturated = input.size() >
        static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max());
    features.byte_count = saturated ? std::numeric_limits<std::uint32_t>::max()
                                    : static_cast<std::uint32_t>(input.size());
    if (input.empty()) return features;

    std::array<std::uint32_t, kHistogramSize> histogram{};
    std::array<std::uint32_t, kCoarseHistogramSize> coarse_histogram{};
    std::array<std::size_t, 4> delta_small{};
    std::array<std::size_t, 4> delta_total{};
    std::array<std::size_t, 4> periodic_equal{};
    constexpr std::array<std::size_t, 4> kDeltaWidths{{1U, 2U, 4U, 8U}};
    constexpr std::array<std::size_t, 4> kPeriodWidths{{2U, 4U, 8U, 16U}};
    std::array<std::uint32_t, kLzHashSlots> last_position{};
    last_position.fill(kNoPosition);
    const MagicFlags magic = magic_flags(input);
    Utf8State utf8{};

    std::size_t printable = 0U;
    std::size_t whitespace = 0U;
    std::size_t markup = 0U;
    std::size_t source_punctuation = 0U;
    std::size_t newline = 0U;
    std::size_t digit = 0U;
    std::size_t zero = 0U;
    std::size_t ff = 0U;
    std::size_t high_bit = 0U;
    std::size_t low_byte = 0U;
    std::size_t equal_adjacent = 0U;
    std::size_t long_run_coverage = 0U;
    std::size_t longest_run = 1U;
    std::size_t run_start = 0U;
    std::size_t match_coverage = 0U;
    std::size_t match_length_sum = 0U;
    std::size_t match_count = 0U;
    std::size_t maximum_match = 0U;
    std::size_t x86_relative_targets = 0U;

    // This is the sole sequential input traversal. LZ probes make bounded
    // look-ahead comparisons: one probe per eight bytes, capped at 32 bytes.
    for (std::size_t index = 0U; index < input.size(); ++index) {
        const std::uint8_t value = input[index];
        ++histogram[value];
        ++coarse_histogram[value >> 4U];
        printable += is_printable(value) ? 1U : 0U;
        whitespace += is_whitespace(value) ? 1U : 0U;
        markup += is_markup(value) ? 1U : 0U;
        source_punctuation += is_source_punctuation(value) ? 1U : 0U;
        newline += value == '\n' ? 1U : 0U;
        digit += value >= '0' && value <= '9' ? 1U : 0U;
        zero += value == 0U ? 1U : 0U;
        ff += value == 0xFFU ? 1U : 0U;
        high_bit += (value & 0x80U) != 0U ? 1U : 0U;
        low_byte += value < 32U ? 1U : 0U;
        utf8.observe(value);

        if (index != 0U) equal_adjacent += input[index] == input[index - 1U];
        if (index + 1U == input.size() || input[index + 1U] != value) {
            const std::size_t run_length = index - run_start + 1U;
            if (run_length > longest_run) longest_run = run_length;
            if (run_length >= 4U) long_run_coverage += run_length;
            run_start = index + 1U;
        }

        for (std::size_t width_index = 0U;
             width_index < kDeltaWidths.size(); ++width_index) {
            const std::size_t width = kDeltaWidths[width_index];
            if (index >= width && index % width == 0U) {
                ++delta_total[width_index];
                delta_small[width_index] +=
                    small_delta_element(input, index, width) ? 1U : 0U;
            }
        }
        for (std::size_t width_index = 0U;
             width_index < kPeriodWidths.size(); ++width_index) {
            const std::size_t width = kPeriodWidths[width_index];
            if (index >= width) {
                periodic_equal[width_index] +=
                    input[index] == input[index - width] ? 1U : 0U;
            }
        }

        if (index + 5U <= input.size() &&
            (value == 0xE8U || value == 0xE9U)) {
            const std::int32_t relative = static_cast<std::int32_t>(
                read_u32_le(input, index + 1U));
            const std::int64_t target = static_cast<std::int64_t>(index + 5U) +
                static_cast<std::int64_t>(relative);
            x86_relative_targets += target >= 0 &&
                target < static_cast<std::int64_t>(input.size()) ? 1U : 0U;
        }

        if (index % kLzSampleStride == 0U && index + 4U <= input.size()) {
            const std::uint32_t sequence = read_u32_le(input, index);
            const std::size_t slot =
                (static_cast<std::size_t>(sequence) * 2654435761U) &
                (kLzHashSlots - 1U);
            const std::uint32_t previous = last_position[slot];
            last_position[slot] = static_cast<std::uint32_t>(index);
            if (previous != kNoPosition && previous < index) {
                const std::size_t limit = input.size() - index < kLzMatchLimit
                    ? input.size() - index : kLzMatchLimit;
                std::size_t matched = 0U;
                while (matched < limit && input[index + matched] ==
                       input[static_cast<std::size_t>(previous) + matched]) {
                    ++matched;
                }
                if (matched >= 4U) {
                    match_coverage += matched;
                    match_length_sum += matched;
                    ++match_count;
                    if (matched > maximum_match) maximum_match = matched;
                }
            }
        }
    }

    std::uint32_t maximum_frequency = 0U;
    for (std::uint32_t count : histogram) {
        if (count > maximum_frequency) maximum_frequency = count;
    }
    if (match_coverage > input.size()) match_coverage = input.size();

    auto& values = features.values;
    values[feature_index(BlockFeatureId::BlockSizeBucket)] =
        static_cast<std::int32_t>(block_size_bucket(input.size()));
    values[feature_index(BlockFeatureId::ByteEntropyQ12)] =
        static_cast<std::int32_t>(entropy_q12(histogram, input.size()));
    values[feature_index(BlockFeatureId::CoarseEntropyQ12)] =
        static_cast<std::int32_t>(entropy_q12(coarse_histogram, input.size()));
    values[feature_index(BlockFeatureId::MaximumByteFrequencyQ12)] =
        static_cast<std::int32_t>(q12_fraction(maximum_frequency, input.size()));
    values[feature_index(BlockFeatureId::ZeroFractionQ12)] =
        static_cast<std::int32_t>(q12_fraction(zero, input.size()));
    values[feature_index(BlockFeatureId::FfFractionQ12)] =
        static_cast<std::int32_t>(q12_fraction(ff, input.size()));
    values[feature_index(BlockFeatureId::PrintableFractionQ12)] =
        static_cast<std::int32_t>(q12_fraction(printable, input.size()));
    values[feature_index(BlockFeatureId::HighBitFractionQ12)] =
        static_cast<std::int32_t>(q12_fraction(high_bit, input.size()));
    values[feature_index(BlockFeatureId::WhitespaceFractionQ12)] =
        static_cast<std::int32_t>(q12_fraction(whitespace, input.size()));
    values[feature_index(BlockFeatureId::NewlineFractionQ12)] =
        static_cast<std::int32_t>(q12_fraction(newline, input.size()));
    values[feature_index(BlockFeatureId::DigitFractionQ12)] =
        static_cast<std::int32_t>(q12_fraction(digit, input.size()));
    values[feature_index(BlockFeatureId::MarkupFractionQ12)] =
        static_cast<std::int32_t>(q12_fraction(markup, input.size()));
    values[feature_index(BlockFeatureId::SourcePunctuationFractionQ12)] =
        static_cast<std::int32_t>(q12_fraction(source_punctuation, input.size()));
    values[feature_index(BlockFeatureId::EqualAdjacentFractionQ12)] =
        static_cast<std::int32_t>(q12_fraction(
            equal_adjacent, input.size() > 1U ? input.size() - 1U : 0U));
    values[feature_index(BlockFeatureId::LongRunCoverageQ12)] =
        static_cast<std::int32_t>(q12_fraction(long_run_coverage, input.size()));
    const unsigned longest_log = longest_run > 1U
        ? floor_log2(static_cast<std::uint32_t>(longest_run)) : 0U;
    values[feature_index(BlockFeatureId::LongestRunLog2Q12)] =
        static_cast<std::int32_t>((longest_log > 15U ? 15U : longest_log) *
                                  kBlockFeatureQ12One);
    values[feature_index(BlockFeatureId::SmallDeltaWidth1Q12)] =
        static_cast<std::int32_t>(q12_fraction(delta_small[0], delta_total[0]));
    values[feature_index(BlockFeatureId::SmallDeltaWidth2Q12)] =
        static_cast<std::int32_t>(q12_fraction(delta_small[1], delta_total[1]));
    values[feature_index(BlockFeatureId::SmallDeltaWidth4Q12)] =
        static_cast<std::int32_t>(q12_fraction(delta_small[2], delta_total[2]));
    values[feature_index(BlockFeatureId::SmallDeltaWidth8Q12)] =
        static_cast<std::int32_t>(q12_fraction(delta_small[3], delta_total[3]));
    values[feature_index(BlockFeatureId::LowByteConcentrationQ12)] =
        static_cast<std::int32_t>(q12_fraction(low_byte, input.size()));

    std::uint32_t best_periodicity = 0U;
    std::uint32_t best_width = 0U;
    for (std::size_t width_index = 0U;
         width_index < kPeriodWidths.size(); ++width_index) {
        const std::size_t width = kPeriodWidths[width_index];
        const std::uint32_t score = q12_fraction(periodic_equal[width_index],
            input.size() > width ? input.size() - width : 0U);
        if (score > best_periodicity) {
            best_periodicity = score;
            best_width = static_cast<std::uint32_t>(width);
        }
    }
    values[feature_index(BlockFeatureId::BestPeriodicityQ12)] =
        static_cast<std::int32_t>(best_periodicity);
    values[feature_index(BlockFeatureId::BestPeriodWidth)] =
        static_cast<std::int32_t>(best_width);
    values[feature_index(BlockFeatureId::SampledLzCoverageQ12)] =
        static_cast<std::int32_t>(q12_fraction(match_coverage, input.size()));
    values[feature_index(BlockFeatureId::SampledLzMeanLengthQ12)] =
        match_count == 0U ? 0 : static_cast<std::int32_t>(
            (match_length_sum * static_cast<std::size_t>(kBlockFeatureQ12One)) /
            match_count);
    values[feature_index(BlockFeatureId::SampledLzMaximumLength)] =
        static_cast<std::int32_t>(maximum_match);
    values[feature_index(BlockFeatureId::X86RelativeTargetDensityQ12)] =
        static_cast<std::int32_t>(q12_fraction(x86_relative_targets, input.size()));

    const bool valid_utf8 = utf8.valid && utf8.remaining == 0U;
    std::uint32_t flags = 0U;
    flags |= valid_utf8 ? kBlockFeatureUtf8 : 0U;
    flags |= magic.known ? kBlockFeatureKnownMagic : 0U;
    flags |= magic.compressed ? kBlockFeatureCompressedMagic : 0U;
    flags |= saturated ? kBlockFeatureSaturated : 0U;
    flags |= (block_size_bucket(input.size()) == 0U && !magic.known &&
              !valid_utf8) ? kBlockFeatureOutOfDistribution : 0U;
    values[feature_index(BlockFeatureId::PackedFlags)] =
        static_cast<std::int32_t>(flags);
    return features;
}

}  // namespace hz::r2
