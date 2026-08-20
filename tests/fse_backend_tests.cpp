#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "r2/core/byte_view.h"
#include "r2/entropy/fse_backend.h"

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
    std::uint32_t state = 0x12345678U;
    for (std::uint8_t& value : bytes) {
        state ^= state << 13U;
        state ^= state >> 17U;
        state ^= state << 5U;
        value = static_cast<std::uint8_t>(state >> 24U);
    }
    return bytes;
}

std::vector<std::uint8_t> round_trip(
    const hz::r2::FseBackend& backend,
    const std::vector<std::uint8_t>& input) {
    const std::vector<std::uint8_t> payload =
        backend.encode(hz::r2::ByteView(input));
    require(payload.size() <=
                hz::r2::FseBackend::maximum_payload_size(input.size()),
            "FSE payload exceeded its bound");
    require(backend.decode(hz::r2::ByteView(payload), input.size()) == input,
            "FSE round trip was not byte-exact");
    return payload;
}

}  // namespace

int main() {
    try {
        const hz::r2::FseBackend backend;
        require(backend.kind() == hz::r2::EntropyKind::Fse,
                "FSE entropy kind is wrong");

        const std::vector<std::uint8_t> empty_payload =
            round_trip(backend, {});
        require(empty_payload.size() == 1U && empty_payload[0] == 0U,
                "Empty FSE payload did not use raw framing");

        const std::vector<std::uint8_t> repeated(4096, 0x5AU);
        const std::vector<std::uint8_t> rle_payload =
            round_trip(backend, repeated);
        require(rle_payload.size() == 2U && rle_payload[0] == 1U,
                "Repeated FSE input did not use RLE framing");

        std::vector<std::uint8_t> patterned(16384);
        for (std::size_t index = 0; index < patterned.size(); ++index) {
            patterned[index] = static_cast<std::uint8_t>(index % 7U);
        }
        const std::vector<std::uint8_t> compressed_payload =
            round_trip(backend, patterned);
        require(compressed_payload[0] == 2U &&
                    compressed_payload.size() < patterned.size(),
                "Patterned FSE input did not use the donor codec");

        const std::vector<std::uint8_t> random = pseudo_random_bytes(4096);
        const std::vector<std::uint8_t> raw_payload =
            round_trip(backend, random);
        require(raw_payload[0] == 0U &&
                    raw_payload.size() == random.size() + 1U,
                "High-entropy FSE input did not use raw framing");

        require_failure(
            [&] { backend.decode(hz::r2::ByteView(), 1U); },
            "Empty malformed FSE payload was accepted");
        require_failure(
            [&] {
                const std::vector<std::uint8_t> invalid{0x7FU};
                backend.decode(hz::r2::ByteView(invalid), 1U);
            },
            "Unknown FSE payload kind was accepted");
        require_failure(
            [&] {
                std::vector<std::uint8_t> truncated = compressed_payload;
                truncated.pop_back();
                backend.decode(hz::r2::ByteView(truncated), patterned.size());
            },
            "Truncated compressed FSE payload was accepted");
        require_failure(
            [&] {
                const std::vector<std::uint8_t> malformed{0U, 1U};
                backend.decode(hz::r2::ByteView(malformed), 2U);
            },
            "Wrong-sized raw FSE payload was accepted");

        std::cout << "fse_backend_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "fse_backend_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
