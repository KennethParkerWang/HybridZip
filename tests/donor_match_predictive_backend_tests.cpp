#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "core/types.h"
#include "r2/core/byte_view.h"
#include "r2/entropy/binary_arithmetic_codec.h"
#include "r2/entropy/donor_match_predictive_backend.h"
#include "r2/entropy/predictive_v1_backend.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <typename Exception, typename Function>
void require_failure(Function&& function, const char* message) {
    try {
        std::forward<Function>(function)();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error(message);
}

hz::Cdf uniform_cdf() {
    hz::Cdf cdf{};
    constexpr std::uint32_t frequency = hz::kCdfTotal / hz::kAlphabet;
    for (std::size_t symbol = 0; symbol <= hz::kAlphabet; ++symbol) {
        cdf.v[symbol] = static_cast<std::uint32_t>(symbol) * frequency;
    }
    return cdf;
}

void test_binary_arithmetic_adapter() {
    constexpr std::array<std::uint32_t, 12> probabilities{{
        1U << 23U, 1U << 22U, 3U << 22U, 1U << 20U,
        15U << 20U, 1U << 23U, 1U << 21U, 7U << 21U,
        1U << 23U, 5U << 21U, 3U << 21U, 1U << 23U}};
    constexpr std::array<std::uint8_t, 12> bits{{
        0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1}};

    std::ostringstream output(std::ios::out | std::ios::binary);
    hz::r2::BinaryArithmeticEncoderStream encoder(output);
    for (std::size_t i = 0; i < bits.size(); ++i) {
        encoder.write_bit(probabilities[i], 1U << 24U, bits[i]);
    }
    encoder.finish();
    encoder.finish();

    const std::string payload = output.str();
    require(!payload.empty(), "Binary arithmetic coder emitted no payload");
    std::istringstream input(payload, std::ios::in | std::ios::binary);
    hz::r2::BinaryArithmeticDecoderStream decoder(input);
    for (std::size_t i = 0; i < bits.size(); ++i) {
        require(decoder.read_bit(probabilities[i], 1U << 24U) == bits[i],
                "Binary arithmetic round trip changed a bit");
    }

    std::ostringstream rejected_output(std::ios::out | std::ios::binary);
    hz::r2::BinaryArithmeticEncoderStream rejected(rejected_output);
    require_failure<std::invalid_argument>(
        [&] { rejected.write_bit(0, 1U << 24U, 0); },
        "Binary arithmetic coder accepted a zero frequency");
    require_failure<std::invalid_argument>(
        [&] { rejected.write_bit(1, 1U << 24U, 2); },
        "Binary arithmetic coder accepted a non-bit symbol");
    rejected.finish();
    require_failure<std::logic_error>(
        [&] { rejected.write_bit(1U << 23U, 1U << 24U, 0); },
        "Binary arithmetic coder accepted data after finish");
}

void test_conditional_probability_and_fusion() {
    const hz::Cdf cdf = uniform_cdf();
    for (std::uint8_t prefix_length = 0; prefix_length < 8U;
         ++prefix_length) {
        const std::uint8_t maximum_prefix = static_cast<std::uint8_t>(
            (std::uint32_t{1} << prefix_length) - 1U);
        require(hz::r2::DonorMatchPredictiveBackend::conditional_v1_p1(
                    cdf, prefix_length, maximum_prefix) == (1U << 23U),
                "Uniform byte CDF did not condition to a fair bit");
    }
    require_failure<std::invalid_argument>(
        [&] {
            (void)hz::r2::DonorMatchPredictiveBackend::conditional_v1_p1(
                cdf, 8, 0);
        },
        "Eight-bit prefix was accepted for a next-bit query");

    hz::r2::MatchEvidence evidence{};
    evidence.candidates.push_back(
        hz::r2::MatchHypothesis{7, 40, 0xA5U, 20});
    const hz::r2::MatchBitProbability leading_one =
        hz::r2::DonorMatchPredictiveBackend::conditional_match_p1(
            evidence, 0, 0);
    require(leading_one.active && leading_one.p1 > (1U << 23U),
            "PAQ Match evidence did not favor its leading one bit");
    const hz::r2::MatchBitProbability matching_prefix =
        hz::r2::DonorMatchPredictiveBackend::conditional_match_p1(
            evidence, 1, 1);
    require(matching_prefix.active && matching_prefix.p1 < (1U << 23U),
            "PAQ Match evidence did not condition on the byte prefix");
    const hz::r2::MatchBitProbability rejected_prefix =
        hz::r2::DonorMatchPredictiveBackend::conditional_match_p1(
            evidence, 1, 0);
    require(!rejected_prefix.active && rejected_prefix.p1 == (1U << 23U),
            "Prefix-inconsistent PAQ evidence remained active");

    const std::uint32_t neutral =
        hz::r2::DonorMatchPredictiveBackend::fuse_p1(
            1U << 23U, 1U << 23U, rejected_prefix);
    const std::uint32_t donor_shifted =
        hz::r2::DonorMatchPredictiveBackend::fuse_p1(
            1U << 23U, 1U << 23U, leading_one);
    require(neutral == (1U << 23U) && donor_shifted > neutral,
            "Donor Match evidence did not change the coding posterior");

    const auto active_high = hz::r2::MatchBitProbability{
        hz::r2::DonorMatchPredictiveBackend::kProbabilityScale - 1U, true};
    require(hz::r2::DonorMatchPredictiveBackend::fuse_p1(
                hz::r2::DonorMatchPredictiveBackend::kProbabilityScale - 1U,
                hz::r2::DonorMatchPredictiveBackend::kProbabilityScale - 1U,
                active_high) ==
                hz::r2::DonorMatchPredictiveBackend::kProbabilityScale -
                    hz::r2::DonorMatchPredictiveBackend::kMinimumBitFrequency,
            "Fused probability did not enforce the payload-bound floor");
}

std::vector<std::uint8_t> pseudo_random_bytes(const std::size_t size) {
    std::vector<std::uint8_t> bytes(size);
    std::uint32_t state = 0xC001D00DU;
    for (std::uint8_t& value : bytes) {
        state ^= state << 13U;
        state ^= state >> 17U;
        state ^= state << 5U;
        value = static_cast<std::uint8_t>(state >> 24U);
    }
    return bytes;
}

void round_trip(const std::vector<std::uint8_t>& source) {
    const hz::r2::DonorMatchPredictiveBackend backend(hz::kDefaultModelSeed);
    const std::vector<std::uint8_t> first =
        backend.encode(hz::r2::ByteView(source));
    const std::vector<std::uint8_t> second =
        backend.encode(hz::r2::ByteView(source));
    require(first == second,
            "Donor Match predictive payload is not deterministic");
    require(first.size() <=
                hz::r2::DonorMatchPredictiveBackend::maximum_payload_size(
                    source.size()),
            "Donor Match predictive payload violated its declared bound");
    require(backend.decode(hz::r2::ByteView(first), source.size()) == source,
            "Donor Match predictive round trip was not byte-exact");
}

void test_backend_round_trips_and_distinct_path() {
    round_trip({});
    const std::string text =
        "ABCDEFGHIxABCDEFGHIxABCDEFGHIxABCDEFGHIx";
    const std::vector<std::uint8_t> repeated(text.begin(), text.end());
    round_trip(repeated);
    round_trip(pseudo_random_bytes(73));

    const hz::r2::DonorMatchPredictiveBackend donor(hz::kDefaultModelSeed);
    const hz::r2::PredictiveV1Backend v1(hz::kDefaultModelSeed);
    require(donor.encode(hz::r2::ByteView(repeated)) !=
                v1.encode(hz::r2::ByteView(repeated)),
            "Donor Match backend collapsed to the V1 coding path");

    require_failure<std::invalid_argument>(
        [&] { (void)donor.decode(hz::r2::ByteView{}, 1); },
        "Non-empty donor Match output accepted an empty payload");
    require_failure<std::length_error>(
        [] {
            (void)hz::r2::DonorMatchPredictiveBackend::maximum_payload_size(
                std::numeric_limits<std::size_t>::max());
        },
        "Overflowing donor Match payload bound was accepted");
}

}  // namespace

int main() {
    try {
        test_binary_arithmetic_adapter();
        test_conditional_probability_and_fusion();
        test_backend_round_trips_and_distinct_path();
        std::cout << "donor_match_predictive_backend_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "donor_match_predictive_backend_tests: FAIL: "
                  << error.what() << '\n';
        return 1;
    }
}
