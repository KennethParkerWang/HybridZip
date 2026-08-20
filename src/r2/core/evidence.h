#pragma once

#include <array>
#include <cstdint>
#include <variant>
#include <vector>

namespace hz::r2 {

struct SymbolPosterior256 {
    std::array<std::uint32_t, 256> frequency{};
    std::uint32_t total = 0;
};

struct BitPosterior {
    std::uint32_t p1 = 0;
    std::uint32_t scale = 0;
};

struct MatchHypothesis {
    std::uint64_t distance = 0;
    std::uint32_t length = 0;
    std::uint8_t next_byte = 0;
    std::uint32_t confidence = 0;
};

struct MatchEvidence {
    std::vector<MatchHypothesis> candidates;
};

struct ResidualEvidence {
    std::int64_t prediction = 0;
    std::uint32_t scale_hint = 0;
};

using ExpertEvidence = std::variant<SymbolPosterior256,
                                    BitPosterior,
                                    MatchEvidence,
                                    ResidualEvidence>;

struct ExpertContext {
    std::uint64_t absolute_position = 0;
    std::uint32_t block_type = 0;
    std::uint32_t byte_in_block = 0;
};

}  // namespace hz::r2

