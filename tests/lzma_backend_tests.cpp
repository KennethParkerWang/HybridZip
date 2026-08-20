#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "r2/entropy/lzma_backend.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <typename Function>
void require_failure(Function&& function, const char* message) {
    try {
        std::forward<Function>(function)();
    } catch (const std::exception&) {
        return;
    }
    throw std::runtime_error(message);
}

std::vector<std::uint8_t> pseudo_random_bytes(const std::size_t size) {
    std::vector<std::uint8_t> bytes(size);
    std::uint32_t state = 0xA53C9E17U;
    for (std::uint8_t& value : bytes) {
        state ^= state << 13U;
        state ^= state >> 17U;
        state ^= state << 5U;
        value = static_cast<std::uint8_t>(state >> 24U);
    }
    return bytes;
}

void round_trip(const hz::r2::LzmaBackend& backend,
                const std::vector<std::uint8_t>& input) {
    const std::vector<std::uint8_t> encoded =
        backend.encode(hz::r2::ByteView(input));
    require(encoded.size() <=
                hz::r2::LzmaBackend::maximum_payload_size(input.size()),
            "LZMA output exceeded its advertised bound");
    require(backend.decode(hz::r2::ByteView(encoded), input.size()) == input,
            "LZMA round trip was not byte-exact");
}

void test_identity_and_round_trips() {
    const hz::r2::LzmaBackend backend;
    require(std::string(backend.name()) == "lzma" &&
                backend.kind() == hz::r2::EntropyKind::Lzma,
            "LZMA backend identity is wrong");

    round_trip(backend, {});
    round_trip(backend, std::vector<std::uint8_t>(64U * 1024U, 0x41U));

    const std::string paragraph =
        "HybridZip uses a donor-driven LZMA coding path. ";
    std::vector<std::uint8_t> text;
    for (int index = 0; index < 2048; ++index) {
        text.insert(text.end(), paragraph.begin(), paragraph.end());
    }
    round_trip(backend, text);
    round_trip(backend, pseudo_random_bytes(128U * 1024U));
    round_trip(backend, pseudo_random_bytes(1024U * 1024U));

    const auto first = backend.encode(hz::r2::ByteView(text));
    const auto second = backend.encode(hz::r2::ByteView(text));
    require(first == second, "Single-threaded LZMA output is not deterministic");
    require(first.size() < text.size(),
            "LZMA did not compress repetitive text");
}

void test_payload_validation() {
    const hz::r2::LzmaBackend backend;
    const auto input = pseudo_random_bytes(8192);
    const auto valid = backend.encode(hz::r2::ByteView(input));

    auto rejected = [&](std::vector<std::uint8_t> payload,
                        const std::size_t expected_size,
                        const char* message) {
        require_failure(
            [&] {
                static_cast<void>(backend.decode(hz::r2::ByteView(payload),
                                                 expected_size));
            },
            message);
    };

    std::vector<std::uint8_t> truncated_header(valid.begin(),
                                                valid.begin() + 12);
    rejected(std::move(truncated_header), input.size(),
             "Truncated LZMA header was accepted");

    auto truncated_stream = valid;
    truncated_stream.pop_back();
    rejected(std::move(truncated_stream), input.size(),
             "Truncated LZMA stream was accepted");

    auto trailing = valid;
    trailing.push_back(0);
    rejected(std::move(trailing), input.size(),
             "Trailing LZMA bytes were accepted");

    auto invalid_magic = valid;
    invalid_magic[0] ^= 0x80U;
    rejected(std::move(invalid_magic), input.size(),
             "Invalid LZMA magic was accepted");

    auto invalid_version = valid;
    invalid_version[4] = 2;
    rejected(std::move(invalid_version), input.size(),
             "Invalid LZMA version was accepted");

    auto invalid_reserved = valid;
    invalid_reserved[39] = 1;
    rejected(std::move(invalid_reserved), input.size(),
             "Nonzero LZMA reserved byte was accepted");

    auto invalid_properties = valid;
    invalid_properties[24] = 0xFFU;
    rejected(std::move(invalid_properties), input.size(),
             "Invalid LZMA properties were accepted");

    auto oversized_dictionary = valid;
    oversized_dictionary[25] = 0;
    oversized_dictionary[26] = 0;
    oversized_dictionary[27] = 0;
    oversized_dictionary[28] = 8;
    rejected(std::move(oversized_dictionary), input.size(),
             "Oversized LZMA dictionary was accepted");

    auto corrupt_stream = valid;
    corrupt_stream[hz::r2::LzmaBackend::kPayloadHeaderSize + 7] ^= 0x20U;
    rejected(std::move(corrupt_stream), input.size(),
             "Corrupt LZMA stream was accepted");

    rejected(valid, input.size() + 1,
             "Mismatched expected LZMA size was accepted");

    auto invalid_declared_size = valid;
    invalid_declared_size[8] ^= 1U;
    rejected(std::move(invalid_declared_size), input.size(),
             "Invalid declared LZMA size was accepted");

    auto invalid_compressed_size = valid;
    invalid_compressed_size[12] ^= 1U;
    rejected(std::move(invalid_compressed_size), input.size(),
             "Invalid compressed LZMA size was accepted");
}

void test_resource_limits_and_options() {
    require_failure([] { hz::r2::LzmaBackend invalid(-1); },
                    "Invalid LZMA level was accepted");
    require_failure([] { hz::r2::LzmaBackend invalid(5, 1024); },
                    "Invalid LZMA dictionary was accepted");
    require_failure(
        [] {
            static_cast<void>(hz::r2::LzmaBackend::maximum_payload_size(
                std::numeric_limits<std::size_t>::max()));
        },
        "Overflowing LZMA payload bound was accepted");

    const hz::r2::LzmaBackend limited(5, 0, 1024);
    const auto too_large = pseudo_random_bytes(1025);
    require_failure(
        [&] {
            static_cast<void>(limited.encode(hz::r2::ByteView(too_large)));
        },
        "Oversized LZMA input was accepted");

    const hz::r2::LzmaBackend normal;
    const auto source = pseudo_random_bytes(2048);
    const auto payload = normal.encode(hz::r2::ByteView(source));
    require_failure(
        [&] {
            static_cast<void>(limited.decode(hz::r2::ByteView(payload),
                                             source.size()));
        },
        "Oversized LZMA decode was accepted");
}

}  // namespace

int main() {
    try {
        test_identity_and_round_trips();
        test_payload_validation();
        test_resource_limits_and_options();
        std::cout << "lzma_backend_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "lzma_backend_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
