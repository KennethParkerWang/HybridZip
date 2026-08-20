#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "archive/archive_header.h"
#include "core/byte_history.h"
#include "core/cdf.h"
#include "core/probability.h"
#include "entropy/arithmetic_codec.h"
#include "mixer/adaptive_linear_mixer.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void test_probability_and_cdf() {
    hz::ProbVector probability{};
    probability[0] = std::numeric_limits<double>::quiet_NaN();
    probability[1] = 2.0;
    hz::normalize_probability(probability);

    double sum = 0.0;
    for (const double value : probability) {
        require(std::isfinite(value) && value > 0.0,
                "probability normalization failed");
        sum += value;
    }
    require(std::abs(sum - 1.0) < 1e-12, "probability sum is not one");

    const hz::Cdf first = hz::quantize_to_cdf(probability);
    const hz::Cdf second = hz::quantize_to_cdf(probability);
    require(first.v == second.v, "CDF quantization is not deterministic");
    require(first.v.front() == 0 && first.v.back() == hz::kCdfTotal,
            "CDF endpoints are invalid");
}

void test_byte_history() {
    hz::ByteHistory history(3);
    history.push(10);
    history.push(20);
    history.push(30);
    history.push(40);
    require(history.position() == 4 && history.size() == 3,
            "history position or size is invalid");
    require(history.back(1) == 40 && history.back(3) == 20,
            "history ring access is invalid");
    require(!history.contains(0) && history.contains(1) && history.contains(3),
            "history containment is invalid");
}

void test_archive_header() {
    hz::ArchiveHeader expected{};
    expected.original_size = 123456789;
    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    hz::write_archive_header(stream, expected);
    require(stream.str().size() == hz::kArchiveHeaderSize,
            "archive header is not 40 bytes");
    stream.seekg(0);
    const hz::ArchiveHeader actual = hz::read_archive_header(stream);
    require(actual.original_size == expected.original_size &&
                actual.model_seed == expected.model_seed,
            "archive header round trip failed");
}

void test_mixer_update() {
    hz::AdaptiveLinearMixer mixer(4, 0.5);
    std::vector<hz::ProbVector> experts(4);
    for (hz::ProbVector& expert : experts) {
        hz::set_uniform_probability(expert);
    }
    experts[0].fill(hz::kProbFloor);
    experts[0][42] = 1.0;
    hz::normalize_probability(experts[0]);

    hz::ProbVector mixed{};
    mixer.mix(experts, mixed);
    mixer.update(42, experts);
    require(mixer.weights()[0] > mixer.weights()[1],
            "mixer did not reward the accurate expert");
}

void test_arithmetic_round_trip() {
    hz::ProbVector probability{};
    hz::set_uniform_probability(probability);
    probability[0] = 0.2;
    probability[1] = 0.1;
    hz::normalize_probability(probability);
    const hz::Cdf cdf = hz::quantize_to_cdf(probability);

    const std::array<std::uint8_t, 10> expected{
        0, 1, 2, 255, 0, 42, 1, 1, 200, 0};
    std::stringstream encoded(std::ios::in | std::ios::out | std::ios::binary);
    {
        hz::ArithmeticEncoderStream encoder(encoded);
        for (const std::uint8_t symbol : expected) {
            encoder.write_symbol(cdf, symbol);
        }
        encoder.finish();
    }

    encoded.seekg(0);
    hz::ArithmeticDecoderStream decoder(encoded);
    for (const std::uint8_t symbol : expected) {
        require(decoder.read_symbol(cdf) == symbol,
                "arithmetic coder round trip failed");
    }
}

}  // namespace

int main() {
    try {
        test_probability_and_cdf();
        test_byte_history();
        test_archive_header();
        test_mixer_update();
        test_arithmetic_round_trip();
        std::cout << "hz_core_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "hz_core_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
