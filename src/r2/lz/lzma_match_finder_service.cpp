#include "r2/lz/lzma_match_finder_service.h"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <string>

extern "C" {
#include "Alloc.h"
#include "LzFind.h"
}

namespace hz::r2 {
namespace {

constexpr std::uint32_t kMinimumHistorySize = 4096;
constexpr std::uint32_t kMinimumMatchLength = 2;
constexpr std::uint32_t kMaximumLzmaMatchLength = 273;
constexpr std::uint32_t kHashBytes = 4;

class MatchFinder final {
public:
    MatchFinder(const ByteView input, const LzmaMatchFinderConfig& config) {
        MatchFinder_Construct(&finder_);
        finder_.btMode = 1;
        finder_.numHashBytes = kHashBytes;
        finder_.cutValue = config.cut_value;
        MatchFinder_SET_DIRECT_INPUT_BUF(&finder_, input.data(), input.size());
        if (MatchFinder_Create(&finder_, config.history_size, 0,
                               config.maximum_match_length, 0, &g_Alloc) == 0) {
            throw std::bad_alloc();
        }
        MatchFinder_CreateVTable(&finder_, &table_);
        table_.Init(&finder_);
    }

    ~MatchFinder() { MatchFinder_Free(&finder_, &g_Alloc); }

    MatchFinder(const MatchFinder&) = delete;
    MatchFinder& operator=(const MatchFinder&) = delete;

    UInt32 available() { return table_.GetNumAvailableBytes(&finder_); }

    UInt32* get_matches(UInt32* output) {
        return table_.GetMatches(&finder_, output);
    }

    void skip(const UInt32 count) { table_.Skip(&finder_, count); }

private:
    CMatchFinder finder_{};
    IMatchFinder2 table_{};
};

void validate_input(const ByteView input) {
    if (!input.empty() && input.data() == nullptr) {
        throw std::invalid_argument("LZMA match finder input has a null pointer");
    }
    if (input.size() > std::numeric_limits<UInt32>::max()) {
        throw std::length_error("LZMA match finder input exceeds donor limits");
    }
}

}  // namespace

LzmaMatchFinderService::LzmaMatchFinderService(
    const LzmaMatchFinderConfig config)
    : config_(config) {
    if (config_.history_size < kMinimumHistorySize ||
        config_.maximum_match_length < kMinimumMatchLength ||
        config_.maximum_match_length > kMaximumLzmaMatchLength ||
        config_.cut_value == 0) {
        throw std::invalid_argument("LZMA match finder configuration is invalid");
    }
}

const char* LzmaMatchFinderService::name() const noexcept {
    return "7zip-lzfind-binary-tree-greedy";
}

LzParseStability LzmaMatchFinderService::stability() const noexcept {
    return LzParseStability::Production;
}

const char* LzmaMatchFinderService::stability_notice() const noexcept {
    return "public-domain 7-Zip LzFind binary-tree candidates with a "
           "HybridZip deterministic greedy selection policy";
}

LzParseResult LzmaMatchFinderService::parse(const ByteView input) const {
    validate_input(input);
    LzParseResult result{};
    result.input_size = input.size();
    result.sequence_bound = input.size() + (input.empty() ? 0U : 1U);
    result.block_delimiters_preserved = true;
    if (input.empty()) {
        result.consumed_size = 0;
        return result;
    }

    MatchFinder finder(input, config_);
    std::array<UInt32, kMaximumLzmaMatchLength * 2U + 2U> matches{};
    std::size_t cursor = 0;
    while (finder.available() != 0U) {
        const UInt32 available = finder.available();
        UInt32* const end = finder.get_matches(matches.data());
        if (end < matches.data() || end > matches.data() + matches.size() ||
            ((end - matches.data()) % 2) != 0) {
            throw std::logic_error("7-Zip LzFind returned an invalid match list");
        }

        std::uint32_t length = 0;
        std::uint32_t distance = 0;
        for (UInt32* item = matches.data(); item != end; item += 2) {
            if (item[0] > length ||
                (item[0] == length && item[1] < distance)) {
                length = item[0];
                distance = item[1];
            }
        }

        LzParseSequence sequence{};
        sequence.sequence_index = result.sequences.size();
        sequence.block_index = 0;
        sequence.cursor_begin = cursor;
        if (length >= kMinimumMatchLength) {
            if (length > available || length > input.size() - cursor ||
                distance == std::numeric_limits<std::uint32_t>::max()) {
                throw std::logic_error("7-Zip LzFind returned an invalid match");
            }
            sequence.match_length = length;
            sequence.distance = distance + 1U;
            sequence.offset = sequence.distance;
            if (sequence.distance > cursor) {
                throw std::logic_error("7-Zip LzFind match exceeds history");
            }
            sequence.literal_end = cursor;
            sequence.match_source_begin = cursor - sequence.distance;
            sequence.match_end = cursor + length;
            for (std::size_t i = 0; i < length; ++i) {
                if (input[sequence.match_source_begin + i] != input[cursor + i]) {
                    throw std::logic_error("7-Zip LzFind match does not reconstruct input");
                }
            }
            if (length > 1U) {
                finder.skip(length - 1U);
            }
            result.match_bytes += length;
            cursor += length;
        } else {
            sequence.literal_length = 1;
            sequence.literal_end = cursor + 1U;
            sequence.match_source_begin = sequence.literal_end;
            sequence.match_end = sequence.literal_end;
            ++result.literal_bytes;
            ++cursor;
        }
        result.sequences.push_back(sequence);
    }

    if (cursor != input.size()) {
        throw std::logic_error("7-Zip LzFind parse did not consume the input");
    }
    LzParseSequence delimiter{};
    delimiter.sequence_index = result.sequences.size();
    delimiter.block_index = 0;
    delimiter.cursor_begin = cursor;
    delimiter.literal_end = cursor;
    delimiter.match_source_begin = cursor;
    delimiter.match_end = cursor;
    delimiter.block_delimiter = true;
    result.sequences.push_back(delimiter);
    result.raw_sequence_count = result.sequences.size();
    result.block_count = 1;
    result.consumed_size = cursor;
    return result;
}

}  // namespace hz::r2
