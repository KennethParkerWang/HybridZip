#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/core/byte_view.h"

namespace hz::r2 {

enum class LzParseStability : std::uint8_t {
    Production,
    CandidateInstrumentationOnly,
};

struct LzParseSequence {
    std::size_t sequence_index = 0;
    std::size_t block_index = 0;

    // The record consumes [cursor_begin, literal_end) as literals and then
    // [literal_end, match_end) as a match copied from match_source_begin.
    std::size_t cursor_begin = 0;
    std::size_t literal_end = 0;
    std::size_t match_source_begin = 0;
    std::size_t match_end = 0;

    std::uint32_t literal_length = 0;
    std::uint32_t match_length = 0;
    std::uint32_t distance = 0;
    std::uint32_t offset = 0;
    std::uint32_t rep = 0;
    bool block_delimiter = false;
};

inline bool operator==(const LzParseSequence& left,
                       const LzParseSequence& right) noexcept {
    return left.sequence_index == right.sequence_index &&
           left.block_index == right.block_index &&
           left.cursor_begin == right.cursor_begin &&
           left.literal_end == right.literal_end &&
           left.match_source_begin == right.match_source_begin &&
           left.match_end == right.match_end &&
           left.literal_length == right.literal_length &&
           left.match_length == right.match_length &&
           left.distance == right.distance && left.offset == right.offset &&
           left.rep == right.rep &&
           left.block_delimiter == right.block_delimiter;
}

struct LzParseResult {
    std::size_t input_size = 0;
    std::size_t consumed_size = 0;
    std::size_t sequence_bound = 0;
    std::size_t raw_sequence_count = 0;
    std::size_t block_count = 0;
    std::size_t literal_bytes = 0;
    std::size_t match_bytes = 0;
    bool block_delimiters_preserved = false;
    std::vector<LzParseSequence> sequences;
};

inline bool operator==(const LzParseResult& left,
                       const LzParseResult& right) {
    return left.input_size == right.input_size &&
           left.consumed_size == right.consumed_size &&
           left.sequence_bound == right.sequence_bound &&
           left.raw_sequence_count == right.raw_sequence_count &&
           left.block_count == right.block_count &&
           left.literal_bytes == right.literal_bytes &&
           left.match_bytes == right.match_bytes &&
           left.block_delimiters_preserved ==
               right.block_delimiters_preserved &&
           left.sequences == right.sequences;
}

class ILzParseService {
public:
    virtual ~ILzParseService() = default;

    virtual const char* name() const noexcept = 0;
    virtual LzParseStability stability() const noexcept = 0;
    virtual const char* stability_notice() const noexcept = 0;
    virtual LzParseResult parse(ByteView input) const = 0;
};

}  // namespace hz::r2
