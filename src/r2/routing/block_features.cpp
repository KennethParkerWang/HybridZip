#include "r2/routing/block_features.h"

#include <array>
#include <cstddef>

namespace hz::r2 {
namespace {

std::uint16_t per_mille(const std::size_t count, const std::size_t total) {
    return total == 0 ? 0 : static_cast<std::uint16_t>(
        (count * 1000U) / total);
}

bool is_printable(const std::uint8_t value) noexcept {
    return (value >= 0x20U && value <= 0x7EU) || value == '\n' ||
        value == '\r' || value == '\t';
}

bool is_whitespace(const std::uint8_t value) noexcept {
    return value == ' ' || value == '\n' || value == '\r' ||
        value == '\t' || value == '\f' || value == '\v';
}

bool is_markup(const std::uint8_t value) noexcept {
    return value == '<' || value == '>' || value == '{' || value == '}' ||
        value == '[' || value == ']' || value == '(' || value == ')' ||
        value == ';' || value == '=' || value == ':' || value == '/' ||
        value == '\\' || value == '#' || value == '*';
}

std::uint16_t equal_at(const ByteView input, const std::size_t lag) {
    if (input.size() <= lag) {
        return 0;
    }
    std::size_t equal = 0;
    for (std::size_t index = lag; index < input.size(); ++index) {
        equal += input[index] == input[index - lag] ? 1U : 0U;
    }
    return per_mille(equal, input.size() - lag);
}

}  // namespace

BlockClass BlockFeaturesV1::classify() const noexcept {
    if (x86_branch_per_mille >= 20U && printable_per_mille < 700U) {
        return BlockClass::X86;
    }
    if (printable_per_mille >= 700U &&
        (whitespace_per_mille >= 10U || markup_per_mille >= 20U)) {
        return BlockClass::Text;
    }
    const std::uint16_t max_equal =
        equal_lag1_per_mille > equal_lag2_per_mille
            ? (equal_lag1_per_mille > equal_lag4_per_mille
                   ? equal_lag1_per_mille
                   : equal_lag4_per_mille)
            : (equal_lag2_per_mille > equal_lag4_per_mille
                   ? equal_lag2_per_mille
                   : equal_lag4_per_mille);
    const std::uint16_t max_equal_with_lag8 =
        max_equal > equal_lag8_per_mille ? max_equal : equal_lag8_per_mille;
    if (max_equal_with_lag8 >= 120U || zero_per_mille >= 80U) {
        return BlockClass::Numeric;
    }
    return BlockClass::Generic;
}

BlockFeaturesV1 extract_block_features(const ByteView input) noexcept {
    BlockFeaturesV1 features{};
    features.byte_count = static_cast<std::uint32_t>(input.size());
    if (input.empty()) {
        return features;
    }

    std::array<bool, 256> present{};
    std::size_t printable = 0;
    std::size_t whitespace = 0;
    std::size_t markup = 0;
    std::size_t zero = 0;
    std::size_t branches = 0;
    for (std::size_t index = 0; index < input.size(); ++index) {
        const std::uint8_t value = input[index];
        present[value] = true;
        printable += is_printable(value) ? 1U : 0U;
        whitespace += is_whitespace(value) ? 1U : 0U;
        markup += is_markup(value) ? 1U : 0U;
        zero += value == 0 ? 1U : 0U;
        branches += (value == 0xE8U || value == 0xE9U) &&
            index + 4U < input.size() ? 1U : 0U;
    }
    std::size_t unique = 0;
    for (const bool value : present) {
        unique += value ? 1U : 0U;
    }

    features.printable_per_mille = per_mille(printable, input.size());
    features.whitespace_per_mille = per_mille(whitespace, input.size());
    features.markup_per_mille = per_mille(markup, input.size());
    features.zero_per_mille = per_mille(zero, input.size());
    features.x86_branch_per_mille = per_mille(branches, input.size());
    features.unique_bytes = static_cast<std::uint16_t>(unique);
    features.equal_lag1_per_mille = equal_at(input, 1U);
    features.equal_lag2_per_mille = equal_at(input, 2U);
    features.equal_lag4_per_mille = equal_at(input, 4U);
    features.equal_lag8_per_mille = equal_at(input, 8U);
    return features;
}

}  // namespace hz::r2
