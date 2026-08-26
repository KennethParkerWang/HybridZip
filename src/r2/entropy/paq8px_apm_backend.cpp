#include "r2/entropy/paq8px_apm_backend.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

#include "codec/model_pipeline.h"
#include "core/profile.h"
#include "r2/entropy/binary_arithmetic_codec.h"
#include "r2/entropy/donor_match_predictive_backend.h"
#include "r2/experts/cmix_match_expert.h"
#include "r2/experts/paq8px_match_expert.h"
#include "apm1.h"

namespace hz::r2 {
namespace {

constexpr std::uint32_t kApmContextCount = 1U << 16U;
constexpr std::uint32_t kApmRate = 7U;

const MatchEvidence& require_match_evidence(const ExpertEvidence& evidence) {
    const auto* matches = std::get_if<MatchEvidence>(&evidence);
    if (matches == nullptr) {
        throw std::logic_error("PAQ8px Match expert returned wrong evidence");
    }
    return *matches;
}

const BitPosterior& require_bit_posterior(const ExpertEvidence& evidence) {
    const auto* posterior = std::get_if<BitPosterior>(&evidence);
    if (posterior == nullptr ||
        posterior->scale != Paq8pxApmBackend::kProbabilityScale) {
        throw std::logic_error("cmix Match expert returned wrong posterior");
    }
    return *posterior;
}

ExpertContext context_for(const std::size_t position) {
    if (position > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("PAQ8px APM block exceeds position limit");
    }
    ExpertContext context{};
    context.absolute_position = position;
    context.byte_in_block = static_cast<std::uint32_t>(position);
    return context;
}

std::uint32_t calibrated_probability(const std::uint32_t base_q24,
                                     const std::uint8_t prefix_length,
                                     const std::uint8_t prefix_value,
                                     paq8px::Apm1& apm,
                                     const std::uint8_t previous_byte) {
    const std::uint32_t base_q12 =
        std::clamp((base_q24 + (1U << 11U)) >> 12U, 1U, 4095U);
    const std::uint32_t c0 =
        (1U << prefix_length) | static_cast<std::uint32_t>(prefix_value);
    const std::uint32_t context =
        (static_cast<std::uint32_t>(previous_byte) << 8U) | c0;
    const std::uint32_t adjusted_q12 = apm.predict(base_q12, context);
    return std::clamp(adjusted_q12 << 12U, 1U,
                      Paq8pxApmBackend::kProbabilityScale - 1U);
}

template <typename ReadBit>
std::uint8_t process_byte(ModelPipeline& pipeline,
                          CmixMatchExpert& cmix,
                          Paq8pxMatchExpert& paq,
                          paq8px::Apm1& apm,
                          const std::size_t position,
                          const std::uint8_t previous_byte,
                          ReadBit&& read_bit) {
    const ExpertContext context = context_for(position);
    const Cdf& cdf = pipeline.predict_cdf();
    const MatchEvidence matches = require_match_evidence(paq.predict(context));
    std::uint8_t value = 0;
    for (std::uint8_t prefix_length = 0; prefix_length < 8U;
         ++prefix_length) {
        const BitPosterior cmix_posterior =
            require_bit_posterior(cmix.predict(context));
        const std::uint32_t v1_p1 =
            DonorMatchPredictiveBackend::conditional_v1_p1(
                cdf, prefix_length, value);
        const MatchBitProbability paq_p1 =
            DonorMatchPredictiveBackend::conditional_match_p1(
                matches, prefix_length, value);
        const std::uint32_t fused = DonorMatchPredictiveBackend::fuse_p1(
            v1_p1, cmix_posterior.p1, paq_p1);
        const std::uint32_t probability = calibrated_probability(
            fused, prefix_length, value, apm, previous_byte);
        const std::uint8_t bit = read_bit(probability, prefix_length);
        if (bit > 1U) {
            throw std::logic_error("PAQ8px APM bit callback returned non-bit");
        }
        cmix.observe(bit, context);
        apm.update(bit);
        value = static_cast<std::uint8_t>((value << 1U) | bit);
    }
    pipeline.observe(value);
    paq.observe(value, context);
    return value;
}

}  // namespace

std::size_t Paq8pxApmBackend::maximum_payload_size(const std::size_t input_size) {
    if (input_size > (std::numeric_limits<std::size_t>::max() - 64U) / 4U) {
        throw std::length_error("PAQ8px APM payload bound overflows size_t");
    }
    return input_size * 4U + 64U;
}

std::vector<std::uint8_t> Paq8pxApmBackend::encode(const ByteView input) const {
    if (input.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("PAQ8px APM input exceeds block limit");
    }
    if (!input.empty() && input.data() == nullptr) {
        throw std::invalid_argument("PAQ8px APM input has null data");
    }
    ModelPipeline pipeline(make_profile_v1());
    pipeline.reset(model_seed_);
    CmixMatchExpert cmix;
    Paq8pxMatchExpert paq;
    const ExpertContext first_context = context_for(0);
    cmix.reset_block(first_context);
    paq.reset_block(first_context);
    paq8px::Apm1 apm(kApmContextCount, kApmRate);
    std::ostringstream encoded(std::ios::out | std::ios::binary);
    BinaryArithmeticEncoderStream coder(encoded);
    std::uint8_t previous = 0;
    for (std::size_t position = 0; position < input.size(); ++position) {
        const std::uint8_t actual = input[position];
        std::uint8_t bit_index = 0;
        const std::uint8_t reconstructed = process_byte(
            pipeline, cmix, paq, apm, position, previous,
            [&](const std::uint32_t probability,
                const std::uint8_t) -> std::uint8_t {
                const std::uint8_t bit = static_cast<std::uint8_t>(
                    (actual >> (7U - bit_index)) & 1U);
                ++bit_index;
                coder.write_bit(probability, kProbabilityScale, bit);
                return bit;
            });
        if (reconstructed != actual) {
            throw std::logic_error("PAQ8px APM encoder reconstructed wrong byte");
        }
        previous = actual;
    }
    coder.finish();
    const std::string bytes = encoded.str();
    if (bytes.size() > maximum_payload_size(input.size())) {
        throw std::runtime_error("PAQ8px APM payload exceeded its safe bound");
    }
    return {reinterpret_cast<const std::uint8_t*>(bytes.data()),
            reinterpret_cast<const std::uint8_t*>(bytes.data()) + bytes.size()};
}

std::vector<std::uint8_t> Paq8pxApmBackend::decode(
    const ByteView payload, const std::size_t expected_size) const {
    if (expected_size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("PAQ8px APM output exceeds block limit");
    }
    if (payload.size() > maximum_payload_size(expected_size) ||
        (expected_size != 0U && payload.empty())) {
        throw std::invalid_argument("PAQ8px APM payload violates size contract");
    }
    if (!payload.empty() && payload.data() == nullptr) {
        throw std::invalid_argument("PAQ8px APM payload has null data");
    }
    const std::string bytes = payload.empty()
        ? std::string{}
        : std::string(reinterpret_cast<const char*>(payload.data()),
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
    paq8px::Apm1 apm(kApmContextCount, kApmRate);
    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    std::uint8_t previous = 0;
    for (std::size_t position = 0; position < expected_size; ++position) {
        const std::uint8_t value = process_byte(
            pipeline, cmix, paq, apm, position, previous,
            [&](const std::uint32_t probability,
                const std::uint8_t) -> std::uint8_t {
                return coder.read_bit(probability, kProbabilityScale);
            });
        output.push_back(value);
        previous = value;
    }
    return output;
}

}  // namespace hz::r2
