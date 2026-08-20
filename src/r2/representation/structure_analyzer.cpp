#include "r2/representation/structure_analyzer.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace hz::r2 {
namespace {

constexpr std::size_t kWindowSize = 4;
constexpr std::size_t kMatchLimit = 258;
constexpr std::size_t kHashSlots = 4096;
constexpr std::size_t kMaximumMatchProbes = 512;

std::size_t hash_window(const ByteView input, const std::size_t position) {
    std::uint32_t value = 0;
    for (std::size_t offset = 0; offset < kWindowSize; ++offset) {
        value = (value * 257U) + input[position + offset];
    }
    return value & (kHashSlots - 1U);
}

double delta_similarity(const ByteView input, const std::size_t width) {
    if (input.size() <= width) {
        return 0.0;
    }
    std::size_t equal_bytes = 0;
    for (std::size_t index = width; index < input.size(); ++index) {
        equal_bytes += input[index] == input[index - width] ? 1U : 0U;
    }
    return static_cast<double>(equal_bytes) /
           static_cast<double>(input.size() - width);
}

}  // namespace

StructureFeatures StructureAnalyzer::analyze(const ByteView input) const {
    StructureFeatures features{};
    if (input.empty()) {
        return features;
    }

    std::array<std::size_t, 256> histogram{};
    std::size_t printable = 0;
    std::size_t zero = 0;
    std::size_t x86_branches = 0;
    for (std::size_t index = 0; index < input.size(); ++index) {
        const std::uint8_t value = input[index];
        ++histogram[value];
        printable += (value >= 0x20U && value <= 0x7EU) ||
                     value == '\n' || value == '\r' || value == '\t';
        zero += value == 0;
        if ((value == 0xE8U || value == 0xE9U) &&
            index + 4 < input.size()) {
            ++x86_branches;
        }
    }

    const double size = static_cast<double>(input.size());
    for (const std::size_t count : histogram) {
        if (count == 0) {
            continue;
        }
        const double probability = static_cast<double>(count) / size;
        features.entropy_bits -= probability * std::log2(probability);
    }
    features.printable_fraction = static_cast<double>(printable) / size;
    features.zero_fraction = static_cast<double>(zero) / size;
    features.x86_branch_fraction = static_cast<double>(x86_branches) / size;
    features.delta_similarity_1 = delta_similarity(input, 1);
    features.delta_similarity_2 = delta_similarity(input, 2);
    features.delta_similarity_4 = delta_similarity(input, 4);
    features.delta_similarity_8 = delta_similarity(input, 8);

    if (input.size() < kWindowSize * 2) {
        return features;
    }

    std::array<std::size_t, kHashSlots> latest{};
    latest.fill(std::numeric_limits<std::size_t>::max());
    const std::size_t window_count = input.size() - kWindowSize + 1;
    const std::size_t probe_stride =
        window_count > kMaximumMatchProbes
            ? window_count / kMaximumMatchProbes
            : 1;
    std::size_t probes = 0;
    std::size_t matched_probes = 0;
    for (std::size_t position = 0; position + kWindowSize <= input.size();
         ++position) {
        const std::size_t slot = hash_window(input, position);
        const std::size_t previous = latest[slot];
        latest[slot] = position;
        if (position % probe_stride != 0) {
            continue;
        }
        ++probes;
        if (previous == std::numeric_limits<std::size_t>::max() ||
            previous >= position || position - previous > 65536U) {
            continue;
        }

        std::size_t length = 0;
        const std::size_t available = input.size() - position;
        const std::size_t limit = available < kMatchLimit ? available : kMatchLimit;
        while (length < limit && input[previous + length] == input[position + length]) {
            ++length;
        }
        if (length >= kWindowSize) {
            features.longest_match = static_cast<std::uint32_t>(
                length > features.longest_match ? length : features.longest_match);
            ++matched_probes;
        }
    }
    features.repeated_window_fraction = static_cast<double>(matched_probes) /
        static_cast<double>(probes);
    return features;
}

}  // namespace hz::r2
