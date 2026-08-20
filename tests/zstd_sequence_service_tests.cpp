#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "r2/core/byte_view.h"
#include "r2/lz/zstd_sequence_service.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <typename Exception, typename Function>
void require_throws(Function&& function, const char* message) {
    bool caught = false;
    try {
        std::forward<Function>(function)();
    } catch (const Exception&) {
        caught = true;
    } catch (...) {
        throw std::runtime_error("operation threw the wrong exception type");
    }
    require(caught, message);
}

std::vector<std::uint8_t> bytes(const std::string_view text) {
    return {text.begin(), text.end()};
}

std::vector<std::uint8_t> pseudo_random_bytes(const std::size_t size) {
    std::vector<std::uint8_t> result(size);
    std::uint32_t state = 0xA341316CU;
    for (std::uint8_t& value : result) {
        state ^= state << 13U;
        state ^= state >> 17U;
        state ^= state << 5U;
        value = static_cast<std::uint8_t>(state >> 24U);
    }
    return result;
}

std::vector<std::uint8_t> reconstruct(
    const std::vector<std::uint8_t>& input,
    const hz::r2::LzParseResult& parse) {
    require(parse.input_size == input.size(),
            "parse input size does not match the source");
    require(parse.raw_sequence_count == parse.sequences.size(),
            "raw sequence count does not match preserved records");
    require(parse.raw_sequence_count <= parse.sequence_bound,
            "raw sequence count exceeds ZSTD_sequenceBound");
    require(parse.block_delimiters_preserved,
            "zstd block delimiters were not preserved");

    std::vector<std::uint8_t> output;
    output.reserve(input.size());
    std::size_t literal_bytes = 0;
    std::size_t match_bytes = 0;
    std::size_t delimiters = 0;
    for (std::size_t index = 0; index < parse.sequences.size(); ++index) {
        const hz::r2::LzParseSequence& sequence = parse.sequences[index];
        require(sequence.sequence_index == index,
                "sequence index is not contiguous");
        require(sequence.cursor_begin == output.size(),
                "sequence cursor does not begin at reconstructed output");
        require(sequence.literal_end >= sequence.cursor_begin &&
                    sequence.literal_end - sequence.cursor_begin ==
                        sequence.literal_length,
                "literal cursor accounting is inconsistent");
        require(sequence.literal_end <= input.size(),
                "literal cursor exceeds source size");

        output.insert(output.end(),
                      input.begin() +
                          static_cast<std::ptrdiff_t>(sequence.cursor_begin),
                      input.begin() +
                          static_cast<std::ptrdiff_t>(sequence.literal_end));
        literal_bytes += sequence.literal_length;

        if (sequence.block_delimiter) {
            require(sequence.offset == 0 && sequence.distance == 0 &&
                        sequence.match_length == 0,
                    "delimiter contains match metadata");
            require(sequence.match_source_begin == sequence.literal_end &&
                        sequence.match_end == sequence.literal_end,
                    "delimiter advanced the match cursor");
            require(sequence.block_index == delimiters,
                    "delimiter block index is inconsistent");
            ++delimiters;
            continue;
        }

        require(sequence.block_index == delimiters,
                "match record block index is inconsistent");
        require(sequence.offset == sequence.distance &&
                    sequence.distance > 0 && sequence.rep <= 3,
                "match offset, distance, or repcode is invalid");
        require(sequence.distance <= sequence.literal_end &&
                    sequence.match_source_begin ==
                        sequence.literal_end - sequence.distance,
                "match source cursor is invalid");
        require(sequence.match_end >= sequence.literal_end &&
                    sequence.match_end - sequence.literal_end ==
                        sequence.match_length &&
                    sequence.match_end <= input.size(),
                "match end cursor is invalid");

        for (std::size_t i = 0; i < sequence.match_length; ++i) {
            const std::size_t source = sequence.match_source_begin + i;
            require(source < output.size(),
                    "match source is not available during reconstruction");
            output.push_back(output[source]);
        }
        require(output.size() == sequence.match_end,
                "reconstructed match did not reach its cursor end");
        match_bytes += sequence.match_length;
    }

    require(output == input, "parse did not reconstruct the original bytes");
    require(parse.consumed_size == output.size() &&
                parse.consumed_size == input.size(),
            "parse did not consume the complete source");
    require(parse.literal_bytes == literal_bytes &&
                parse.match_bytes == match_bytes &&
                literal_bytes + match_bytes == input.size(),
            "parse byte totals are inconsistent");
    require(parse.block_count == delimiters,
            "parse block count does not match delimiters");
    if (!input.empty()) {
        require(!parse.sequences.empty() &&
                    parse.sequences.back().block_delimiter,
                "non-empty parse lacks its final block delimiter");
    }
    return output;
}

hz::r2::ZstdSequenceService service_at_level(const int level = 3) {
    return hz::r2::ZstdSequenceService(hz::r2::ZstdSequenceConfig{level});
}

void test_contract_and_api_notice() {
    const hz::r2::ZstdSequenceService service;
    require(std::string_view(service.name()) ==
                "zstd-generate-sequences-candidate",
            "zstd sequence service name changed");
    require(service.stability() ==
                hz::r2::LzParseStability::CandidateInstrumentationOnly,
            "deprecated zstd API was presented as production-stable");
    const std::string_view notice(service.stability_notice());
    require(notice.find("deprecated") != std::string_view::npos &&
                notice.find("debug-only") != std::string_view::npos &&
                notice.find("not production-stable") !=
                    std::string_view::npos,
            "zstd API limitation notice is incomplete");
    require(service.config().compression_level == 19,
            "default zstd parser compression level changed");
    require(hz::r2::ZstdSequenceService::minimum_compression_level() < 0 &&
                hz::r2::ZstdSequenceService::maximum_compression_level() >=
                    19,
            "zstd donor compression-level bounds are unexpected");
}

void test_empty_input() {
    const hz::r2::ZstdSequenceService service = service_at_level();
    const std::vector<std::uint8_t> input;
    const hz::r2::LzParseResult result =
        service.parse(hz::r2::ByteView(input));
    (void)reconstruct(input, result);
    require(result.consumed_size == 0 && result.literal_bytes == 0 &&
                result.match_bytes == 0,
            "empty parse retained byte accounting");
}

void test_repetitive_parse_and_repcodes() {
    std::vector<std::uint8_t> input;
    constexpr std::string_view pattern = "abracadabra|";
    input.reserve((pattern.size() + 1U) * 240U);
    for (std::size_t i = 0; i < 240; ++i) {
        input.insert(input.end(), pattern.begin(), pattern.end());
        input.push_back(static_cast<std::uint8_t>(i));
    }

    const hz::r2::ZstdSequenceService service = service_at_level(9);
    const hz::r2::LzParseResult result =
        service.parse(hz::r2::ByteView(input));
    (void)reconstruct(input, result);
    require(result.match_bytes > result.literal_bytes,
            "repetitive input did not expose zstd matches");
    const bool has_repcode = std::any_of(
        result.sequences.begin(), result.sequences.end(),
        [](const hz::r2::LzParseSequence& sequence) {
            return !sequence.block_delimiter && sequence.rep != 0;
        });
    require(has_repcode,
            "repetitive zstd parse did not preserve donor repcode metadata");
}

void test_mixed_literals_and_matches() {
    std::vector<std::uint8_t> input = bytes("unique-prefix:0123456789|");
    constexpr std::string_view repeated =
        "the quick brown fox jumps over the lazy dog|";
    for (std::size_t i = 0; i < 600; ++i) {
        input.insert(input.end(), repeated.begin(), repeated.end());
        if ((i % 97U) == 0) {
            input.push_back(static_cast<std::uint8_t>(i));
        }
    }
    const std::string_view tail = "|unique-tail:ZYXWVUT";
    input.insert(input.end(), tail.begin(), tail.end());

    const hz::r2::ZstdSequenceService service = service_at_level(5);
    const hz::r2::LzParseResult result =
        service.parse(hz::r2::ByteView(input));
    (void)reconstruct(input, result);
    require(result.literal_bytes > 0 && result.match_bytes > 0,
            "mixed input did not expose both literals and matches");
}

void test_random_no_match_parse() {
    const std::vector<std::uint8_t> input = pseudo_random_bytes(256);
    std::set<std::uint32_t> trigrams;
    for (std::size_t i = 0; i + 2 < input.size(); ++i) {
        const std::uint32_t key =
            (static_cast<std::uint32_t>(input[i]) << 16U) |
            (static_cast<std::uint32_t>(input[i + 1]) << 8U) |
            input[i + 2];
        require(trigrams.insert(key).second,
                "random no-match fixture contains a repeated trigram");
    }

    const hz::r2::ZstdSequenceService service = service_at_level(3);
    const hz::r2::LzParseResult result =
        service.parse(hz::r2::ByteView(input));
    (void)reconstruct(input, result);
    require(result.match_bytes == 0 && result.literal_bytes == input.size(),
            "no-match fixture unexpectedly produced an LZ match");
}

void test_multiblock_delimiters() {
    std::vector<std::uint8_t> input;
    input.reserve(320000);
    constexpr std::string_view pattern = "ABCDEFGH01234567";
    while (input.size() < 320000) {
        input.insert(input.end(), pattern.begin(), pattern.end());
    }
    input.resize(320000);

    const hz::r2::ZstdSequenceService service = service_at_level(3);
    const hz::r2::LzParseResult result =
        service.parse(hz::r2::ByteView(input));
    (void)reconstruct(input, result);
    require(result.block_count >= 3,
            "multiblock input did not preserve all zstd delimiters");
    const std::size_t delimiter_count = static_cast<std::size_t>(
        std::count_if(result.sequences.begin(), result.sequences.end(),
                      [](const hz::r2::LzParseSequence& sequence) {
                          return sequence.block_delimiter;
                      }));
    require(delimiter_count == result.block_count,
            "delimiter records were filtered or duplicated");
}

void test_determinism_and_sequence_bound() {
    std::vector<std::uint8_t> input;
    constexpr std::string_view pattern = "deterministic parser corpus|";
    for (std::size_t i = 0; i < 2000; ++i) {
        input.insert(input.end(), pattern.begin(), pattern.end());
    }

    const hz::r2::ZstdSequenceService first = service_at_level(7);
    const hz::r2::ZstdSequenceService second = service_at_level(7);
    const hz::r2::LzParseResult a =
        first.parse(hz::r2::ByteView(input));
    const hz::r2::LzParseResult b =
        first.parse(hz::r2::ByteView(input));
    const hz::r2::LzParseResult c =
        second.parse(hz::r2::ByteView(input));
    require(a == b && a == c,
            "single-threaded zstd sequence generation is not deterministic");
    require(a == first.parse_with_sequence_capacity(
                     hz::r2::ByteView(input), a.sequence_bound),
            "explicit sequence-bound capacity changed the parse");

    require(hz::r2::ZstdSequenceService::sequence_bound(0) == 2,
            "zero-length ZSTD_sequenceBound changed");
    require(hz::r2::ZstdSequenceService::sequence_bound(4096) == 1371,
            "ZSTD_sequenceBound formula changed");
}

void test_error_and_capacity_surfaces() {
    const int minimum =
        hz::r2::ZstdSequenceService::minimum_compression_level();
    const int maximum =
        hz::r2::ZstdSequenceService::maximum_compression_level();
    require_throws<std::invalid_argument>(
        [&] {
            hz::r2::ZstdSequenceService invalid(
                hz::r2::ZstdSequenceConfig{minimum - 1});
        },
        "compression level below donor minimum was accepted");
    require_throws<std::invalid_argument>(
        [&] {
            hz::r2::ZstdSequenceService invalid(
                hz::r2::ZstdSequenceConfig{maximum + 1});
        },
        "compression level above donor maximum was accepted");

    const hz::r2::ZstdSequenceService service = service_at_level();
    require_throws<std::invalid_argument>(
        [&] {
            (void)service.parse(hz::r2::ByteView(nullptr, 1));
        },
        "non-empty null input was accepted");

    std::uint8_t dummy = 0;
    require_throws<std::length_error>(
        [&] {
            (void)service.parse(hz::r2::ByteView(
                &dummy, std::numeric_limits<std::size_t>::max()));
        },
        "unsupported zstd input size was accepted");

    const std::vector<std::uint8_t> repetitive(4096, 0x41U);
    const std::size_t bound =
        hz::r2::ZstdSequenceService::sequence_bound(repetitive.size());
    require_throws<std::invalid_argument>(
        [&] {
            (void)service.parse_with_sequence_capacity(
                hz::r2::ByteView(repetitive), bound + 1U);
        },
        "capacity beyond ZSTD_sequenceBound was accepted");
    require_throws<std::runtime_error>(
        [&] {
            (void)service.parse_with_sequence_capacity(
                hz::r2::ByteView(repetitive), 0);
        },
        "undersized sequence buffer did not surface a donor error");

    const std::vector<std::uint8_t> too_small{0x41U};
    require_throws<std::runtime_error>(
        [&] { (void)service.parse(hz::r2::ByteView(too_small)); },
        "deprecated donor API unexpectedly accepted its tiny-input limit");
}

}  // namespace

int main() {
    try {
        test_contract_and_api_notice();
        test_empty_input();
        test_repetitive_parse_and_repcodes();
        test_mixed_literals_and_matches();
        test_random_no_match_parse();
        test_multiblock_delimiters();
        test_determinism_and_sequence_bound();
        test_error_and_capacity_surfaces();
        std::cout << "zstd_sequence_service_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "zstd_sequence_service_tests: FAIL: " << error.what()
                  << '\n';
        return 1;
    }
}
