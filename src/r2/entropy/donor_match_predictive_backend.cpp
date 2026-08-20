#include "r2/entropy/donor_match_predictive_backend.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

#include "codec/model_pipeline.h"
#include "core/profile.h"
#include "r2/entropy/binary_arithmetic_codec.h"
#include "r2/experts/cmix_match_expert.h"
#include "r2/experts/paq8px_match_expert.h"

namespace hz::r2 {
namespace {

constexpr std::size_t kMaximumPaqCandidates = 4;

void validate_prefix(const std::uint8_t prefix_length,
                     const std::uint8_t prefix_value) {
    if (prefix_length > 7U ||
        (prefix_length != 0U &&
         prefix_value >= (std::uint32_t{1} << prefix_length)) ||
        (prefix_length == 0U && prefix_value != 0U)) {
        throw std::invalid_argument("Invalid byte-prefix probability query");
    }
}

std::uint32_t rounded_ratio_q24(const std::uint64_t numerator,
                                const std::uint64_t denominator) {
    if (denominator == 0U || numerator > denominator) {
        throw std::invalid_argument("Invalid conditional probability ratio");
    }
    const std::uint64_t scaled =
        numerator * DonorMatchPredictiveBackend::kProbabilityScale;
    const std::uint64_t rounded =
        (scaled + denominator / 2U) / denominator;
    return static_cast<std::uint32_t>(std::clamp<std::uint64_t>(
        rounded, 1U,
        DonorMatchPredictiveBackend::kProbabilityScale - 1U));
}

const MatchEvidence& require_match_evidence(
    const ExpertEvidence& evidence) {
    const auto* matches = std::get_if<MatchEvidence>(&evidence);
    if (matches == nullptr) {
        throw std::logic_error(
            "PAQ8px Match expert returned the wrong evidence type");
    }
    return *matches;
}

const BitPosterior& require_bit_posterior(
    const ExpertEvidence& evidence) {
    const auto* posterior = std::get_if<BitPosterior>(&evidence);
    if (posterior == nullptr ||
        posterior->scale != DonorMatchPredictiveBackend::kProbabilityScale) {
        throw std::logic_error(
            "cmix Match expert returned an invalid bit posterior");
    }
    return *posterior;
}

ExpertContext context_for(const std::size_t position) {
    if (position > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error(
            "Donor Match predictive block exceeds the 32-bit position limit");
    }
    ExpertContext context{};
    context.absolute_position = position;
    context.byte_in_block = static_cast<std::uint32_t>(position);
    return context;
}

template <typename ReadBit>
std::uint8_t process_byte(ModelPipeline& pipeline,
                          CmixMatchExpert& cmix,
                          Paq8pxMatchExpert& paq,
                          const std::size_t position,
                          ReadBit&& read_bit) {
    const ExpertContext context = context_for(position);
    const Cdf& cdf = pipeline.predict_cdf();
    const MatchEvidence matches = require_match_evidence(paq.predict(context));

    std::uint8_t value = 0;
    for (std::uint8_t prefix_length = 0; prefix_length < 8U;
         ++prefix_length) {
        const BitPosterior cmix_posterior =
            require_bit_posterior(cmix.predict(context));
        const std::uint32_t v1_p1 = DonorMatchPredictiveBackend::conditional_v1_p1(
            cdf, prefix_length, value);
        const MatchBitProbability paq_p1 =
            DonorMatchPredictiveBackend::conditional_match_p1(
                matches, prefix_length, value);
        const std::uint32_t fused = DonorMatchPredictiveBackend::fuse_p1(
            v1_p1, cmix_posterior.p1, paq_p1);
        const std::uint8_t bit = read_bit(fused, prefix_length);
        if (bit > 1U) {
            throw std::logic_error("Donor Match bit callback returned non-bit");
        }
        cmix.observe(bit, context);
        value = static_cast<std::uint8_t>((value << 1U) | bit);
    }

    pipeline.observe(value);
    paq.observe(value, context);
    return value;
}

}  // namespace

std::uint32_t DonorMatchPredictiveBackend::conditional_v1_p1(
    const Cdf& cdf,
    const std::uint8_t prefix_length,
    const std::uint8_t prefix_value) {
    validate_prefix(prefix_length, prefix_value);
    const unsigned suffix_bits = 8U - prefix_length;
    const std::size_t low =
        static_cast<std::size_t>(prefix_value) << suffix_bits;
    const std::size_t high =
        (static_cast<std::size_t>(prefix_value) + 1U) << suffix_bits;
    const std::size_t middle = low + (std::size_t{1} << (suffix_bits - 1U));
    const std::uint64_t total = cdf.v[high] - cdf.v[low];
    const std::uint64_t one = cdf.v[high] - cdf.v[middle];
    return rounded_ratio_q24(one, total);
}

MatchBitProbability DonorMatchPredictiveBackend::conditional_match_p1(
    const MatchEvidence& evidence,
    const std::uint8_t prefix_length,
    const std::uint8_t prefix_value) {
    validate_prefix(prefix_length, prefix_value);
    std::uint64_t zero_weight = 0;
    std::uint64_t one_weight = 0;
    const std::size_t count =
        std::min(evidence.candidates.size(), kMaximumPaqCandidates);
    for (std::size_t index = 0; index < count; ++index) {
        const MatchHypothesis& candidate = evidence.candidates[index];
        if (prefix_length != 0U &&
            (candidate.next_byte >> (8U - prefix_length)) != prefix_value) {
            continue;
        }
        const std::uint64_t weight =
            1U + std::min<std::uint32_t>(candidate.confidence, 65535U) +
            std::min<std::uint32_t>(candidate.length, 65535U);
        const std::uint8_t bit = static_cast<std::uint8_t>(
            (candidate.next_byte >> (7U - prefix_length)) & 1U);
        (bit == 0U ? zero_weight : one_weight) += weight;
    }
    const std::uint64_t evidence_weight = zero_weight + one_weight;
    if (evidence_weight == 0U) {
        return MatchBitProbability{kProbabilityScale / 2U, false};
    }
    return MatchBitProbability{
        rounded_ratio_q24(one_weight + 1U, evidence_weight + 2U), true};
}

std::uint32_t DonorMatchPredictiveBackend::fuse_p1(
    const std::uint32_t v1_p1,
    const std::uint32_t cmix_p1,
    const MatchBitProbability paq) {
    const auto valid = [](const std::uint32_t probability) {
        return probability > 0U && probability < kProbabilityScale;
    };
    if (!valid(v1_p1) || !valid(cmix_p1) ||
        (paq.active && !valid(paq.p1))) {
        throw std::invalid_argument(
            "Donor Match fusion received an invalid Q24 probability");
    }

    std::uint64_t weighted = 0;
    std::uint32_t divisor = 0;
    if (paq.active) {
        weighted = std::uint64_t{2} * v1_p1 + cmix_p1 + paq.p1;
        divisor = 4U;
    } else {
        weighted = std::uint64_t{3} * v1_p1 + cmix_p1;
        divisor = 4U;
    }
    const std::uint32_t rounded = static_cast<std::uint32_t>(
        (weighted + divisor / 2U) / divisor);
    return std::clamp(rounded, kMinimumBitFrequency,
                      kProbabilityScale - kMinimumBitFrequency);
}

std::size_t DonorMatchPredictiveBackend::maximum_payload_size(
    const std::size_t input_size) {
    if (input_size > (std::numeric_limits<std::size_t>::max() - 64U) / 4U) {
        throw std::length_error(
            "Donor Match predictive payload bound overflows size_t");
    }
    return input_size * 4U + 64U;
}

std::vector<std::uint8_t> DonorMatchPredictiveBackend::encode(
    const ByteView input) const {
    if (input.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error(
            "Donor Match predictive input exceeds the 32-bit block limit");
    }
    if (!input.empty() && input.data() == nullptr) {
        throw std::invalid_argument(
            "Donor Match predictive input has a null data pointer");
    }

    ModelPipeline pipeline(make_profile_v1());
    pipeline.reset(model_seed_);
    CmixMatchExpert cmix;
    Paq8pxMatchExpert paq;
    const ExpertContext first_context = context_for(0);
    cmix.reset_block(first_context);
    paq.reset_block(first_context);

    std::ostringstream encoded(std::ios::out | std::ios::binary);
    BinaryArithmeticEncoderStream coder(encoded);
    for (std::size_t position = 0; position < input.size(); ++position) {
        const std::uint8_t actual = input[position];
        std::uint8_t bit_index = 0;
        const std::uint8_t reconstructed = process_byte(
            pipeline, cmix, paq, position,
            [&](const std::uint32_t probability,
                const std::uint8_t) -> std::uint8_t {
                const std::uint8_t bit = static_cast<std::uint8_t>(
                    (actual >> (7U - bit_index)) & 1U);
                ++bit_index;
                coder.write_bit(probability, kProbabilityScale, bit);
                return bit;
            });
        if (reconstructed != actual) {
            throw std::logic_error(
                "Donor Match encoder reconstructed the wrong source byte");
        }
    }
    coder.finish();

    const std::string bytes = encoded.str();
    if (bytes.size() > maximum_payload_size(input.size())) {
        throw std::runtime_error(
            "Donor Match predictive payload exceeded its safe bound");
    }
    return {reinterpret_cast<const std::uint8_t*>(bytes.data()),
            reinterpret_cast<const std::uint8_t*>(bytes.data()) + bytes.size()};
}

std::vector<std::uint8_t> DonorMatchPredictiveBackend::decode(
    const ByteView payload,
    const std::size_t expected_size) const {
    if (expected_size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error(
            "Donor Match predictive output exceeds the 32-bit block limit");
    }
    if (payload.size() > maximum_payload_size(expected_size)) {
        throw std::invalid_argument(
            "Donor Match predictive payload exceeds its safe bound");
    }
    if (expected_size != 0U && payload.empty()) {
        throw std::invalid_argument(
            "Donor Match predictive payload is empty");
    }
    if (!payload.empty() && payload.data() == nullptr) {
        throw std::invalid_argument(
            "Donor Match predictive payload has a null data pointer");
    }

    const std::string bytes = payload.empty()
                                  ? std::string{}
                                  : std::string(
                                        reinterpret_cast<const char*>(
                                            payload.data()),
                                        payload.size());
    std::istringstream encoded(bytes, std::ios::in | std::ios::binary);
    BinaryArithmeticDecoderStream coder(encoded);

    ModelPipeline pipeline(make_profile_v1());
    pipeline.reset(model_seed_);
    CmixMatchExpert cmix;
    Paq8pxMatchExpert paq;
    const ExpertContext first_context = context_for(0);
    cmix.reset_block(first_context);
    paq.reset_block(first_context);

    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    for (std::size_t position = 0; position < expected_size; ++position) {
        output.push_back(process_byte(
            pipeline, cmix, paq, position,
            [&](const std::uint32_t probability,
                const std::uint8_t) -> std::uint8_t {
                return coder.read_bit(probability, kProbabilityScale);
            }));
    }
    return output;
}

}  // namespace hz::r2
