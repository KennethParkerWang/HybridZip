#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

#include "match_core.h"
#include "r2/experts/cmix_match_expert.h"

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

hz::cmix::MatchConfig small_config(const std::size_t history_size = 64) {
    hz::cmix::MatchConfig config{};
    config.history_size = history_size;
    return config;
}

hz::r2::BitPosterior predict_bit(hz::r2::CmixMatchExpert& expert,
                                 const hz::r2::ExpertContext& context) {
    const hz::r2::ExpertEvidence evidence = expert.predict(context);
    const auto* posterior = std::get_if<hz::r2::BitPosterior>(&evidence);
    require(posterior != nullptr,
            "cmix Match expert did not return a bit posterior");
    return *posterior;
}

void feed_byte(hz::cmix::MatchCore& core, const std::uint8_t byte) {
    for (int shift = 7; shift >= 0; --shift) {
        (void)core.predict();
        core.observe(static_cast<std::uint8_t>((byte >> shift) & 1U));
    }
}

void test_donor_golden_trace() {
    // Frozen from cmix 1d95fe9 Match/ContextManager/Predictor update order.
    constexpr std::array<std::uint8_t, 8> bytes = {
        0x41, 0x42, 0x41, 0x42, 0x41, 0x42, 0x41, 0x42};
    constexpr std::array<std::uint32_t, 64> expected_q24 = {
        8372224, 8339456, 2790741, 13964630, 8306688, 8273920, 8241152,
        8208384, 1674445, 8398438, 2768896, 2757973, 2747050, 13920939,
        8175616, 15581184, 4658517, 10792375, 1661338, 1654784, 1648230,
        8352564, 13910016, 8634368, 8110080, 8699904, 8044544, 8011776,
        7979008, 7946240, 8863744, 7880704, 7847936, 8962048, 7782400,
        7749632, 7716864, 7684096, 7651328, 9158656, 7585792, 9224192,
        7520256, 7487488, 7454720, 7421952, 9388032, 7356416, 7323648,
        9486336, 7258112, 7225344, 7192576, 7159808, 7127040, 9682944,
        7061504, 9748480, 6995968, 6963200, 6930432, 6897664, 9912320,
        6832128};
    constexpr std::array<std::uint64_t, 8> expected_context = {
        65, 16706, 16961, 16706, 16961, 16706, 16961, 16706};
    constexpr std::array<std::uint64_t, 8> expected_match_position = {
        0, 0, 1, 2, 3, 4, 5, 6};
    constexpr std::array<std::uint8_t, 8> expected_candidate = {
        65, 65, 66, 65, 66, 65, 66, 65};
    constexpr std::array<std::uint8_t, 8> expected_length = {
        0, 0, 8, 16, 24, 32, 40, 48};
    constexpr std::array<std::uint8_t, 8> expected_match_context = {
        0, 0, 0, 0, 0, 1, 1, 1};

    hz::r2::CmixMatchExpert expert(small_config(32));
    const hz::r2::ExpertContext context{};
    std::size_t trace_index = 0;
    for (std::size_t byte_index = 0; byte_index < bytes.size(); ++byte_index) {
        for (int shift = 7; shift >= 0; --shift) {
            const hz::r2::BitPosterior posterior =
                predict_bit(expert, context);
            require(posterior.scale == hz::r2::CmixMatchExpert::kProbabilityScale,
                    "golden trace used the wrong Q24 scale");
            require(posterior.p1 == expected_q24[trace_index],
                    "cmix donor Q24 trace diverged");
            expert.observe(
                static_cast<std::uint8_t>((bytes[byte_index] >> shift) & 1U),
                context);
            ++trace_index;
        }

        const hz::cmix::MatchDiagnostics diagnostics =
            expert.core().diagnostics();
        require(diagnostics.history_position == byte_index + 1,
                "golden history position diverged");
        require(diagnostics.byte_context == expected_context[byte_index],
                "golden byte context diverged");
        require(diagnostics.current_match ==
                    expected_match_position[byte_index],
                "golden match position diverged");
        require(diagnostics.current_byte == expected_candidate[byte_index],
                "golden candidate byte diverged");
        require(diagnostics.match_length == expected_length[byte_index],
                "golden match length diverged");
        require(diagnostics.match_context ==
                    expected_match_context[byte_index],
                "golden match context diverged");
    }
}

void test_msb_lifecycle_and_pending_guards() {
    hz::r2::CmixMatchExpert expert(small_config(16));
    const hz::r2::ExpertContext context{17, 3, 9};

    require_throws<std::logic_error>(
        [&] { expert.observe(0, context); },
        "observe without predict was accepted");

    const hz::r2::BitPosterior first = predict_bit(expert, context);
    const hz::cmix::MatchDiagnostics before_repeat =
        expert.core().diagnostics();
    const hz::r2::BitPosterior repeated = predict_bit(expert, context);
    const hz::cmix::MatchDiagnostics after_repeat =
        expert.core().diagnostics();
    require(first.p1 == 8372224 && first.scale == (1U << 24U),
            "cold Q24 prediction is wrong");
    require(repeated.p1 == first.p1 && repeated.scale == first.scale,
            "repeated prediction changed the posterior");
    require(before_repeat.bit_context == after_repeat.bit_context &&
                before_repeat.bit_mask == after_repeat.bit_mask &&
                before_repeat.match_length == after_repeat.match_length,
            "repeated prediction changed core state");
    require(expert.prediction_pending(),
            "prediction did not establish a pending observation");

    hz::r2::ExpertContext wrong_context = context;
    ++wrong_context.absolute_position;
    require_throws<std::logic_error>(
        [&] { (void)predict_bit(expert, wrong_context); },
        "cross-context repeated prediction was accepted");
    require_throws<std::invalid_argument>(
        [&] { expert.observe(1, wrong_context); },
        "mismatched observation context was accepted");
    const hz::cmix::MatchDiagnostics after_context_rejections =
        expert.core().diagnostics();
    require(expert.prediction_pending() &&
                after_context_rejections.bit_context ==
                    after_repeat.bit_context &&
                after_context_rejections.bit_mask == after_repeat.bit_mask &&
                after_context_rejections.match_length ==
                    after_repeat.match_length,
            "rejected context operation changed pending or core state");

    require_throws<std::invalid_argument>(
        [&] { expert.observe(2, context); },
        "non-bit expert observation was accepted");
    require(expert.prediction_pending(),
            "rejected observation consumed the pending prediction");

    expert.observe(1, context);
    require(!expert.prediction_pending(),
            "successful observation did not consume the prediction");
    hz::cmix::MatchDiagnostics diagnostics = expert.core().diagnostics();
    require(diagnostics.bit_context == 3 && diagnostics.bit_mask == 64,
            "first MSB did not advance the donor bit context");
    require(!diagnostics.has_completed_byte,
            "one bit incorrectly completed a byte");

    constexpr std::array<std::uint8_t, 7> remaining = {0, 1, 0, 0, 1, 0, 1};
    for (const std::uint8_t bit : remaining) {
        (void)predict_bit(expert, context);
        expert.observe(bit, context);
    }
    diagnostics = expert.core().diagnostics();
    require(diagnostics.history_position == 1 &&
                diagnostics.last_completed_byte == 0xA5 &&
                diagnostics.bit_context == 1 && diagnostics.bit_mask == 128,
            "MSB-first byte completion produced the wrong state");
    require(expert.core().history_byte(0) == 0xA5,
            "completed byte was not written to history");

    (void)predict_bit(expert, context);
    expert.reset_block(context);
    require(!expert.prediction_pending(),
            "block reset retained a pending prediction");
    require(expert.core().diagnostics().history_position == 0,
            "block reset retained byte state");
}

std::vector<std::uint32_t> collect_trace(
    hz::r2::CmixMatchExpert& expert,
    const hz::r2::ExpertContext& context) {
    constexpr std::array<std::uint8_t, 8> bytes = {
        'A', 'B', 'A', 'B', 'A', 'B', 'A', 'B'};
    std::vector<std::uint32_t> trace;
    trace.reserve(bytes.size() * 8U);
    for (const std::uint8_t byte : bytes) {
        for (int shift = 7; shift >= 0; --shift) {
            trace.push_back(predict_bit(expert, context).p1);
            expert.observe(
                static_cast<std::uint8_t>((byte >> shift) & 1U), context);
        }
    }
    return trace;
}

void test_reset_determinism_and_ignored_context() {
    hz::r2::CmixMatchExpert expert(small_config(64));
    const hz::r2::ExpertContext first_context{1, 2, 3};
    const hz::r2::ExpertContext second_context{999, 77, 88};
    const std::vector<std::uint32_t> first =
        collect_trace(expert, first_context);

    expert.reset_block(second_context);
    const std::vector<std::uint32_t> second =
        collect_trace(expert, second_context);
    require(first == second,
            "reset or ignored expert context changed the deterministic trace");

    expert.reset_block(first_context);
    require(expert.core().bucket_count(0) == 0 &&
                expert.core().map_entry(0) == 0 &&
                expert.core().history_byte(0) == 0,
            "reset did not clear learned allocations");
    require(expert.core().bucket_probability(0) == 0.5009765625F,
            "reset did not restore the donor probability prior");
}

void test_context_recurrence_and_map_ordering() {
    hz::cmix::MatchCore core(small_config(16));
    feed_byte(core, 0x12);
    require(core.diagnostics().byte_context == 0x12,
            "first order-2 context byte is wrong");
    feed_byte(core, 0x34);
    require(core.diagnostics().byte_context == 0x1234,
            "second order-2 context byte is wrong");
    feed_byte(core, 0x56);

    const hz::cmix::MatchDiagnostics diagnostics = core.diagnostics();
    require(diagnostics.byte_context == 0x3456,
            "order-2 context did not retain the newest two bytes");
    require(core.map_entry(0x12) == 1 &&
                core.map_entry(0x1234) == 2,
            "map insertion did not use the pre-byte-update context");
    require(core.history_byte(0) == 0x12 &&
                core.history_byte(1) == 0x34 &&
                core.history_byte(2) == 0x56,
            "completed bytes were not written after map insertion");
}

void test_learning_hits_and_misses() {
    hz::cmix::MatchCore hit(small_config(8));
    hz::cmix::MatchCore miss(small_config(8));
    const float initial = hit.bucket_probability(0);

    hit.observe(0);
    miss.observe(1);
    require(hit.bucket_count(0) == 1 && miss.bucket_count(0) == 1,
            "first observation did not increment its probability bucket");
    require(hit.bucket_probability(0) > initial,
            "candidate hit did not raise match confidence");
    require(miss.bucket_probability(0) < initial,
            "candidate miss did not lower match confidence");
    require(hit.diagnostics().match_length == 1 &&
                hit.diagnostics().last_observation_matched,
            "candidate hit did not extend the bit match");
    require(miss.diagnostics().match_length == 0 &&
                !miss.diagnostics().last_observation_matched,
            "candidate miss did not reset the bit match");
}

void test_long_match_and_cold_start() {
    hz::cmix::MatchCore cold(small_config(8));
    feed_byte(cold, 0x00);
    require(cold.diagnostics().match_length == 8,
            "zero-filled cold history no longer activates a zero match");

    hz::cmix::MatchCore repetitive(small_config(64));
    for (std::size_t i = 0; i < 80; ++i) {
        feed_byte(repetitive, static_cast<std::uint8_t>((i & 1U) == 0 ? 'A' : 'B'));
    }
    const hz::cmix::MatchDiagnostics diagnostics = repetitive.diagnostics();
    require(diagnostics.match_length == 255 && diagnostics.match_context == 7,
            "long repeated match did not activate or saturate at 255 bits");
    require(diagnostics.current_byte == 'A',
            "long match continuation selected the wrong next byte");
}

void test_tiny_history_wraparound() {
    hz::cmix::MatchConfig config = small_config(3);
    config.context_order = 1;
    config.map_entries = 256;
    hz::cmix::MatchCore core(config);
    for (const std::uint8_t byte : {0x10, 0x20, 0x30, 0x40, 0x50}) {
        feed_byte(core, byte);
    }

    const hz::cmix::MatchDiagnostics diagnostics = core.diagnostics();
    require(diagnostics.history_position == 5 &&
                diagnostics.history_write_index == 2,
            "tiny history position did not wrap deterministically");
    require(core.history_byte(0) == 0x40 &&
                core.history_byte(1) == 0x50 &&
                core.history_byte(2) == 0x30,
            "tiny circular history retained the wrong bytes");
    require(core.map_entry(0x30) == 3 && core.map_entry(0x40) == 4,
            "tiny history map lost logical positions across wraparound");
}

void test_invalid_configuration_and_ranges() {
    hz::cmix::MatchConfig config = small_config();
    config.history_size = 0;
    require_throws<std::invalid_argument>(
        [&] { hz::cmix::MatchCore ignored(config); },
        "zero history size was accepted");

    config = small_config();
    config.map_entries = 0;
    require_throws<std::invalid_argument>(
        [&] { hz::cmix::MatchCore ignored(config); },
        "zero map size was accepted");

    config = small_config();
    config.hash_bits = 0;
    require_throws<std::invalid_argument>(
        [&] { hz::cmix::MatchCore ignored(config); },
        "zero hash width was accepted");

    config = small_config();
    config.hash_bits = 32;
    require_throws<std::invalid_argument>(
        [&] { hz::cmix::MatchCore ignored(config); },
        "oversized hash width was accepted");

    config = small_config();
    config.context_order = 8;
    require_throws<std::invalid_argument>(
        [&] { hz::cmix::MatchCore ignored(config); },
        "64-bit context width was accepted");

    config = small_config();
    config.limit = 0;
    require_throws<std::invalid_argument>(
        [&] { hz::cmix::MatchCore ignored(config); },
        "zero learning limit was accepted");

    config = small_config();
    config.delta = std::numeric_limits<float>::quiet_NaN();
    require_throws<std::invalid_argument>(
        [&] { hz::cmix::MatchCore ignored(config); },
        "NaN learning delta was accepted");

    config = small_config();
    config.map_entries =
        std::numeric_limits<std::size_t>::max() / sizeof(std::uint32_t) + 1U;
    require_throws<std::length_error>(
        [&] { (void)hz::cmix::MatchCore::required_allocation_bytes(config); },
        "overflowing map allocation was accepted");

    hz::cmix::MatchCore core(small_config(7));
    require_throws<std::invalid_argument>(
        [&] { core.observe(2); }, "non-bit core observation was accepted");
    require_throws<std::out_of_range>(
        [&] { (void)core.bucket_probability(256); },
        "out-of-range probability bucket was accepted");
    require_throws<std::out_of_range>(
        [&] { (void)core.bucket_count(256); },
        "out-of-range count bucket was accepted");
    require_throws<std::out_of_range>(
        [&] { (void)core.map_entry(core.config().map_entries); },
        "out-of-range map access was accepted");
    require_throws<std::out_of_range>(
        [&] { (void)core.history_byte(core.config().history_size); },
        "out-of-range history access was accepted");
}

void test_allocation_accounting_and_defaults() {
    hz::cmix::MatchConfig compact = small_config(17);
    compact.map_entries = 19;
    const std::size_t expected = 17 + 19 * sizeof(std::uint32_t);
    require(hz::cmix::MatchCore::required_allocation_bytes(compact) == expected,
            "static allocation accounting is wrong");
    hz::cmix::MatchCore core(compact);
    require(core.allocation_bytes() == expected,
            "instance allocation accounting is wrong");

    const hz::cmix::MatchConfig defaults{};
    require(defaults.history_size == 8U * 1024U * 1024U &&
                defaults.context_order == 2 && defaults.hash_bits == 8 &&
                defaults.map_entries == 65536,
            "canonical cmix Match defaults changed");
    require(hz::cmix::MatchCore::required_allocation_bytes(defaults) ==
                8U * 1024U * 1024U + 65536U * sizeof(std::uint32_t),
            "canonical allocation accounting is wrong");
}

void test_q24_endpoints_and_repetitive_nll() {
    hz::cmix::MatchConfig endpoint_config = small_config(16);
    endpoint_config.limit = 1;
    endpoint_config.delta = std::ldexp(1.0F, -25);
    const hz::r2::ExpertContext context{};

    hz::r2::CmixMatchExpert upper(endpoint_config);
    (void)predict_bit(upper, context);
    upper.observe(1, context);
    require(predict_bit(upper, context).p1 ==
                hz::r2::CmixMatchExpert::kProbabilityScale - 1U,
            "Q24 adapter did not clamp a trained probability-one endpoint");

    hz::r2::CmixMatchExpert lower(endpoint_config);
    (void)predict_bit(lower, context);
    lower.observe(0, context);
    (void)predict_bit(lower, context);
    lower.observe(1, context);
    require(predict_bit(lower, context).p1 == 1U,
            "Q24 adapter did not clamp a trained probability-zero endpoint");

    hz::r2::CmixMatchExpert expert(small_config(256));
    constexpr std::size_t warmup_bytes = 64;
    constexpr std::size_t measured_bytes = 128;
    double nll_bits = 0.0;
    std::size_t measured_bits = 0;

    for (std::size_t i = 0; i < warmup_bytes + measured_bytes; ++i) {
        const std::uint8_t byte =
            static_cast<std::uint8_t>((i & 1U) == 0 ? 'A' : 'B');
        for (int shift = 7; shift >= 0; --shift) {
            const hz::r2::BitPosterior posterior =
                predict_bit(expert, context);
            require(posterior.p1 > 0 && posterior.p1 < posterior.scale,
                    "Q24 adapter exposed a reserved probability endpoint");
            const std::uint8_t bit =
                static_cast<std::uint8_t>((byte >> shift) & 1U);
            if (i >= warmup_bytes) {
                const double p1 = static_cast<double>(posterior.p1) /
                                  static_cast<double>(posterior.scale);
                const double probability = bit != 0 ? p1 : 1.0 - p1;
                require(probability > 0.0 && probability <= 1.0,
                        "Q24 actual-bit probability is invalid");
                nll_bits -= std::log2(probability);
                ++measured_bits;
            }
            expert.observe(bit, context);
        }
    }

    require(std::isfinite(nll_bits), "repetitive-input NLL is not finite");
    require(nll_bits < static_cast<double>(measured_bits),
            "repetitive-input NLL did not beat a uniform bit model");
}

}  // namespace

int main() {
    try {
        test_donor_golden_trace();
        test_msb_lifecycle_and_pending_guards();
        test_reset_determinism_and_ignored_context();
        test_context_recurrence_and_map_ordering();
        test_learning_hits_and_misses();
        test_long_match_and_cold_start();
        test_tiny_history_wraparound();
        test_invalid_configuration_and_ranges();
        test_allocation_accounting_and_defaults();
        test_q24_endpoints_and_repetitive_nll();
        std::cout << "cmix_match_expert_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "cmix_match_expert_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
