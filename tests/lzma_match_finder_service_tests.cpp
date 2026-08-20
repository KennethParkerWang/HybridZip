#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "r2/lz/lzma_match_finder_service.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void reconstruct(const std::vector<std::uint8_t>& input,
                 const hz::r2::LzParseResult& parse) {
    std::vector<std::uint8_t> output;
    for (const hz::r2::LzParseSequence& sequence : parse.sequences) {
        require(sequence.cursor_begin == output.size(), "cursor is not contiguous");
        output.insert(output.end(), input.begin() + sequence.cursor_begin,
                      input.begin() + sequence.literal_end);
        if (!sequence.block_delimiter) {
            for (std::size_t i = 0; i < sequence.match_length; ++i) {
                require(sequence.match_source_begin + i < output.size(),
                        "match source is unavailable");
                output.push_back(output[sequence.match_source_begin + i]);
            }
        }
    }
    require(output == input, "parse does not reconstruct source bytes");
    require(parse.consumed_size == input.size(), "parse consumed size is wrong");
    require(parse.literal_bytes + parse.match_bytes == input.size(),
            "parse byte accounting is wrong");
}

void test_parse_contract() {
    std::vector<std::uint8_t> input;
    constexpr std::string_view text = "abracadabra|";
    for (std::size_t i = 0; i < 700; ++i) input.insert(input.end(), text.begin(), text.end());
    const hz::r2::LzmaMatchFinderService service;
    const hz::r2::LzParseResult first = service.parse(hz::r2::ByteView(input));
    const hz::r2::LzParseResult second = service.parse(hz::r2::ByteView(input));
    require(first == second, "LzFind parse is not deterministic");
    require(first.match_bytes > first.literal_bytes, "repetitive input has no match gain");
    require(first.block_count == 1 && first.sequences.back().block_delimiter,
            "final delimiter is missing");
    reconstruct(input, first);
}

void test_empty_and_errors() {
    const hz::r2::LzmaMatchFinderService service;
    const std::vector<std::uint8_t> empty;
    reconstruct(empty, service.parse(hz::r2::ByteView(empty)));
    bool caught = false;
    try { (void)service.parse(hz::r2::ByteView(nullptr, 1)); }
    catch (const std::invalid_argument&) { caught = true; }
    require(caught, "non-empty null input was accepted");
}

}  // namespace

int main() {
    try {
        test_parse_contract();
        test_empty_and_errors();
        std::cout << "lzma_match_finder_service_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "lzma_match_finder_service_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
