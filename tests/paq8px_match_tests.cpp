#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "../third_party/paq8px/match_core.h"
#include "r2/experts/paq8px_match_expert.h"
#include "r2/match/paq8px_match_service.h"

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

std::vector<std::uint8_t> bytes(const std::string_view text) {
    return {text.begin(), text.end()};
}

void advance_to(hz::paq8px::MatchCore& core,
                const std::vector<std::uint8_t>& history,
                const std::uint32_t position) {
    while (core.position() < position) {
        core.advance(history.data(), history.size(), core.position() + 1U);
    }
}

hz::paq8px::MatchSnapshot only_predictive(
    const hz::paq8px::MatchCore& core) {
    const std::vector<hz::paq8px::MatchSnapshot> candidates =
        core.predictive_candidates();
    require(candidates.size() == 1,
            "Expected exactly one predictive PAQ8px candidate");
    return candidates.front();
}

bool same_match(const hz::r2::MatchCandidate& left,
                const hz::r2::MatchCandidate& right) {
    return left.distance == right.distance && left.length == right.length &&
           left.next_byte == right.next_byte &&
           left.confidence == right.confidence &&
           left.estimated_parse_cost == right.estimated_parse_cost;
}

const hz::r2::MatchEvidence& match_evidence(
    const hz::r2::ExpertEvidence& evidence) {
    require(std::holds_alternative<hz::r2::MatchEvidence>(evidence),
            "PAQ8px expert did not return MatchEvidence");
    return std::get<hz::r2::MatchEvidence>(evidence);
}

void test_allocation_and_guards() {
    using hz::paq8px::MatchCore;
    require(MatchCore::allocation_bytes_for_hash_bits(1) == 24,
            "One-bit PAQ8px allocation formula is wrong");
    require(MatchCore::allocation_bytes_for_hash_bits(20) == 12582912,
            "Default PAQ8px allocation formula is wrong");
    require(MatchCore::allocation_bytes_for_hash_bits(26) == 805306368,
            "Maximum PAQ8px allocation formula is wrong");
    require_failure<std::invalid_argument>(
        [] { (void)MatchCore::allocation_bytes_for_hash_bits(0); },
        "Zero PAQ8px hash bits were accepted");
    require_failure<std::invalid_argument>(
        [] { (void)MatchCore::allocation_bytes_for_hash_bits(27); },
        "Out-of-range PAQ8px hash bits were accepted");
    require_failure<std::invalid_argument>(
        [] { MatchCore invalid(0); },
        "PAQ8px constructor accepted zero hash bits");
    require_failure<std::invalid_argument>(
        [] { MatchCore invalid(27); },
        "PAQ8px constructor accepted out-of-range hash bits");

    MatchCore default_core;
    require(default_core.hash_bits() == MatchCore::kDefaultHashBits &&
                default_core.allocation_bytes() == 12582912,
            "PAQ8px default table allocation changed");

    MatchCore core(1);
    require(core.bucket_count() == 2 && core.allocation_bytes() == 24,
            "PAQ8px core did not expose its exact table allocation");
    require_failure<std::out_of_range>(
        [&] { (void)core.bucket_positions(2); },
        "Out-of-range PAQ8px bucket was accepted");
    require_failure<std::out_of_range>(
        [&] { (void)core.order_hash(15); },
        "Out-of-range PAQ8px order hash was accepted");

    const std::vector<std::uint8_t> one{0x41U};
    require_failure<std::invalid_argument>(
        [&] { core.advance(one.data(), one.size(), 0); },
        "Non-forward PAQ8px core advance was accepted");
    require_failure<std::out_of_range>(
        [&] { core.advance(nullptr, one.size(), 1); },
        "Null PAQ8px history was accepted");
}

void test_lookup_orders_and_golden_trace() {
    struct OrderCase {
        std::string_view history;
        std::uint32_t position;
        std::uint32_t donor_index;
        std::uint8_t order;
    };
    constexpr std::array<OrderCase, 3> cases{{
        {"ABCDEFGHI!ABCDEFGHI", 19, 9, 9},
        {"12ABCDEFG!34ABCDEFG", 19, 9, 7},
        {"12abcde!34abcde", 15, 7, 5},
    }};

    require(hz::paq8px::MatchCore::kLookupOrders ==
                std::array<std::uint8_t, 3>{{9, 7, 5}},
            "PAQ8px candidate lookup order changed");

    for (const OrderCase& test : cases) {
        const std::vector<std::uint8_t> history = bytes(test.history);
        hz::paq8px::MatchCore core(12);
        advance_to(core, history, test.position);
        const hz::paq8px::MatchSnapshot candidate = only_predictive(core);
        require(candidate.index == test.donor_index &&
                    candidate.strength == test.order - 4U &&
                    candidate.contiguous_length == test.order &&
                    candidate.next_byte == '!' &&
                    candidate.mode == hz::paq8px::MatchMode::normal,
                "PAQ8px 9/7/5 lookup order or rebasing changed");
    }

    // Golden byte-boundary trace derived from MatchModel/MatchInfo at PAQ8px
    // 29237fb44cb1995690e3eb72c6c3b1e4aede5791. The original candidate follows
    // donor positions 9, 10, 11, and 12 while strength and exact contiguous
    // length extend independently.
    const std::vector<std::uint8_t> trace_history =
        bytes("ABCDEFGHIxABCDEFGHIxABC");
    hz::paq8px::MatchCore core(15);
    struct Golden {
        std::uint32_t position;
        std::uint32_t index;
        std::uint32_t strength;
        std::uint32_t contiguous;
        std::uint8_t next;
    };
    constexpr std::array<Golden, 4> golden{{
        {19, 9, 5, 9, 'x'},
        {20, 10, 6, 10, 'A'},
        {21, 11, 7, 11, 'B'},
        {22, 12, 8, 12, 'C'},
    }};
    for (const Golden& expected : golden) {
        advance_to(core, trace_history, expected.position);
        const hz::paq8px::MatchSnapshot candidate = only_predictive(core);
        require(candidate.index == expected.index &&
                    candidate.strength == expected.strength &&
                    candidate.contiguous_length == expected.contiguous &&
                    candidate.next_byte == expected.next &&
                    candidate.registration_order == 5 &&
                    candidate.priority ==
                        ((UINT64_C(1) << 49U) |
                         (static_cast<std::uint64_t>(expected.strength)
                          << 32U) |
                         expected.index),
                "Pinned PAQ8px golden candidate trace changed");
    }
}

void test_bucket_cap_dedup_and_priority() {
    const std::vector<std::uint8_t> history = bytes(
        "11ABCDEFGa22ABCDEFGb11ABCDEFGc11ABCDEFGd11ABCDEFG");
    constexpr std::uint32_t position = 49;
    hz::paq8px::MatchCore core(12);
    advance_to(core, history, position);

    const std::vector<hz::paq8px::MatchSnapshot> active =
        core.active_candidates();
    require(active.size() == hz::paq8px::MatchCore::kMaximumCandidates,
            "PAQ8px four-candidate cap was not reached");
    for (std::size_t i = 0; i < active.size(); ++i) {
        for (std::size_t j = i + 1; j < active.size(); ++j) {
            require(active[i].index != active[j].index,
                    "PAQ8px candidate deduplication failed");
        }
    }

    const auto bucket = core.bucket_positions(core.order_bucket(9));
    require(bucket == std::array<std::uint32_t, 3>{{49, 39, 29}},
            "PAQ8px bucket did not retain the three most recent positions");

    hz::r2::Paq8pxMatchService service(12);
    const std::vector<hz::r2::MatchCandidate> matches = service.find(
        hz::r2::ByteView(history), position, 99);
    require(matches.size() == hz::r2::Paq8pxMatchService::kMaximumCandidates,
            "PAQ8px service did not truncate to four candidates");
    for (std::size_t i = 1; i < matches.size(); ++i) {
        const bool ordered =
            matches[i - 1].confidence > matches[i].confidence ||
            (matches[i - 1].confidence == matches[i].confidence &&
             matches[i - 1].distance <= matches[i].distance);
        require(ordered, "PAQ8px service did not preserve donor priority");
    }
}

void test_extension_recovery_and_saturation() {
    {
        const std::vector<std::uint8_t> history =
            bytes("ABCDEFGHIxABCDEFGHIx");
        hz::paq8px::MatchCore core(10);
        advance_to(core, history, 20);
        const auto candidate = only_predictive(core);
        require(candidate.strength == 6 &&
                    candidate.contiguous_length == 10 &&
                    candidate.mode == hz::paq8px::MatchMode::normal,
                "PAQ8px ordinary extension changed");
    }

    const std::vector<std::uint8_t> recovery_history =
        bytes("ABCDEFGHIxABCDEFGHIYABC");
    hz::paq8px::MatchCore recovery(10);
    advance_to(recovery, recovery_history, 19);
    advance_to(recovery, recovery_history, 20);
    std::vector<hz::paq8px::MatchSnapshot> active =
        recovery.active_candidates();
    require(active.size() == 1 && active[0].strength == 0 &&
                active[0].contiguous_length == 0 &&
                active[0].backup_strength == 5 &&
                active[0].backup_index == 9 &&
                active[0].mode == hz::paq8px::MatchMode::pre_recovery &&
                recovery.predictive_candidates().empty(),
            "PAQ8px mismatch did not enter byte-facing pre-recovery");

    advance_to(recovery, recovery_history, 21);
    auto candidate = only_predictive(recovery);
    require(candidate.index == 11 && candidate.strength == 7 &&
                candidate.contiguous_length == 1 &&
                candidate.backup_strength == 6 &&
                candidate.mode == hz::paq8px::MatchMode::recovery &&
                candidate.next_byte == 'B',
            "PAQ8px one-byte mismatch recovery changed");
    hz::r2::Paq8pxMatchService recovery_service(10);
    const auto recovered_match = recovery_service.find(
        hz::r2::ByteView(recovery_history), 21, 4);
    require(recovered_match.size() == 1 &&
                recovered_match[0].distance == 10 &&
                recovered_match[0].length == 1 &&
                recovered_match[0].confidence == 7 &&
                recovered_match[0].next_byte == 'B',
            "PAQ8px recovery field conversion changed");
    advance_to(recovery, recovery_history, 22);
    candidate = only_predictive(recovery);
    require(candidate.contiguous_length == 2 &&
                candidate.mode == hz::paq8px::MatchMode::recovery,
            "PAQ8px recovery did not remain active for the second byte");
    advance_to(recovery, recovery_history, 23);
    candidate = only_predictive(recovery);
    require(candidate.contiguous_length == 3 &&
                candidate.mode == hz::paq8px::MatchMode::normal &&
                candidate.backup_strength == 0,
            "PAQ8px recovery threshold did not stabilize at three bytes");

    {
        const std::vector<std::uint8_t> history =
            bytes("ABCDEFGHIxABCDEFGHIYAZ");
        hz::paq8px::MatchCore core(10);
        advance_to(core, history, 22);
        require(core.active_candidates().empty(),
                "PAQ8px second recovery mismatch did not drop candidate");
    }
    {
        const std::vector<std::uint8_t> history =
            bytes("ABCDEFGHIxABCDEFGHIYQ");
        hz::paq8px::MatchCore core(10);
        advance_to(core, history, 21);
        require(core.active_candidates().empty(),
                "PAQ8px failed recovery did not drop candidate");
    }

    std::vector<std::uint8_t> repeated(65560, static_cast<std::uint8_t>('A'));
    hz::paq8px::MatchCore saturated(4);
    advance_to(saturated, repeated,
               static_cast<std::uint32_t>(repeated.size()));
    const auto saturated_candidates = saturated.predictive_candidates();
    require(!saturated_candidates.empty(),
            "PAQ8px saturation corpus produced no candidates");
    const auto strongest = std::max_element(
        saturated_candidates.begin(), saturated_candidates.end(),
        [](const auto& left, const auto& right) {
            return left.strength < right.strength;
        });
    require(strongest->strength == hz::paq8px::MatchCore::kMaximumStrength &&
                strongest->contiguous_length > strongest->strength,
            "PAQ8px strength saturation or exact length tracking changed");
}

void test_collisions_reset_cache_and_rejections() {
    std::vector<std::uint8_t> unique(64);
    for (std::size_t i = 0; i < unique.size(); ++i) {
        unique[i] = static_cast<std::uint8_t>(i);
    }
    hz::paq8px::MatchCore collision_core(1);
    advance_to(collision_core, unique,
               static_cast<std::uint32_t>(unique.size()));
    require(collision_core.active_candidates().empty(),
            "Hash collision bypassed PAQ8px exact context verification");
    collision_core.reset();
    require(collision_core.position() == 0 &&
                collision_core.active_candidates().empty() &&
                collision_core.bucket_positions(0) ==
                    std::array<std::uint32_t, 3>{},
            "PAQ8px core reset retained block state");

    const std::vector<std::uint8_t> history =
        bytes("ABCDEFGHIxABCDEFGHI");
    hz::r2::Paq8pxMatchService service(10);
    const auto original =
        service.find(hz::r2::ByteView(history), 19, 4);
    require(original.size() == 1, "PAQ8px cache setup failed");
    const std::size_t cached_position = service.position();
    require(service.find(hz::r2::ByteView(history), 19, 0).empty() &&
                service.position() == cached_position,
            "PAQ8px zero-limit cache lookup changed state");
    const auto cached =
        service.find(hz::r2::ByteView(history), 19, 4);
    require(cached.size() == original.size() &&
                same_match(cached.front(), original.front()),
            "PAQ8px same-position cache changed its result");
    std::vector<std::uint8_t> mutated_history = history;
    mutated_history[0] ^= 0xffU;
    const auto mutation_cached =
        service.find(hz::r2::ByteView(mutated_history), 19, 4);
    require(mutation_cached.size() == original.size() &&
                same_match(mutation_cached.front(), original.front()) &&
                service.position() == cached_position,
            "PAQ8px same-position lookup was not side-effect-free");

    require_failure<std::invalid_argument>(
        [&] { (void)service.find(hz::r2::ByteView(history), 18, 4); },
        "PAQ8px service accepted a backward position");
    require_failure<std::out_of_range>(
        [&] {
            (void)service.find(
                hz::r2::ByteView(history.data(), history.size() - 1), 20, 4);
        },
        "PAQ8px service accepted an out-of-range position");
    std::uint8_t dummy = 0;
    const std::size_t too_large =
        static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()) +
        1U;
    require_failure<std::length_error>(
        [&] {
            (void)service.find(hz::r2::ByteView(&dummy, too_large),
                               too_large, 4);
        },
        "PAQ8px service accepted a position beyond 32 bits");

    service.reset(12345);
    require(service.position() == 0 &&
                service.find(hz::r2::ByteView(history), 0, 4).empty(),
            "PAQ8px service reset retained cached state");
}

void test_service_field_conversion() {
    const std::vector<std::uint8_t> history =
        bytes("ABCDEFGHIxABCDEFGHI");
    hz::r2::Paq8pxMatchService service(10);
    require(std::string_view(service.name()) == "paq8px-match",
            "PAQ8px service name changed");
    const auto candidates =
        service.find(hz::r2::ByteView(history), 19, 4);
    require(candidates.size() == 1, "PAQ8px field conversion setup failed");
    const hz::r2::MatchCandidate& candidate = candidates.front();
    require(candidate.distance == 10 && candidate.length == 9 &&
                candidate.next_byte == 'x' && candidate.confidence == 5 &&
                candidate.estimated_parse_cost ==
                    std::numeric_limits<std::uint32_t>::max(),
            "PAQ8px service field conversion changed");
}

void test_expert_lifecycle_and_block_isolation() {
    hz::r2::Paq8pxMatchExpert expert(10);
    require(std::string_view(expert.name()) == "paq8px-match-expert",
            "PAQ8px expert name changed");
    require_failure<std::logic_error>(
        [&] { expert.observe('A', hz::r2::ExpertContext{}); },
        "PAQ8px expert accepted observe before predict");

    const std::vector<std::uint8_t> history =
        bytes("ABCDEFGHIxABCDEFGHI");
    for (std::size_t i = 0; i < history.size(); ++i) {
        hz::r2::ExpertContext context{};
        context.absolute_position = 1000 + i;
        context.byte_in_block = static_cast<std::uint32_t>(i);
        (void)expert.predict(context);
        expert.observe(history[i], context);
    }

    hz::r2::ExpertContext prediction_context{};
    prediction_context.absolute_position = 1000 + history.size();
    prediction_context.byte_in_block =
        static_cast<std::uint32_t>(history.size());
    const hz::r2::ExpertEvidence first = expert.predict(prediction_context);
    const hz::r2::ExpertEvidence repeated = expert.predict(prediction_context);
    const auto& first_matches = match_evidence(first).candidates;
    const auto& repeated_matches = match_evidence(repeated).candidates;
    require(first_matches.size() == 1 && repeated_matches.size() == 1 &&
                first_matches[0].distance == 10 &&
                first_matches[0].length == 9 &&
                first_matches[0].next_byte == 'x' &&
                first_matches[0].confidence == 5,
            "PAQ8px expert did not return service MatchEvidence");

    hz::r2::ExpertContext wrong_context = prediction_context;
    ++wrong_context.absolute_position;
    require_failure<std::invalid_argument>(
        [&] { expert.observe('x', wrong_context); },
        "PAQ8px expert accepted a mismatched observe context");
    expert.observe('x', prediction_context);
    require(expert.prefix_size() == history.size() + 1,
            "PAQ8px expert did not append the observed byte");

    hz::r2::ExpertContext new_block{};
    new_block.absolute_position = 5000;
    expert.reset_block(new_block);
    require(expert.prefix_size() == 0 &&
                match_evidence(expert.predict(new_block)).candidates.empty(),
            "PAQ8px expert block reset retained prefix evidence");
    expert.observe('A', new_block);

    hz::r2::ExpertContext bad_boundary{};
    bad_boundary.absolute_position = 5001;
    bad_boundary.byte_in_block = 0;
    require_failure<std::invalid_argument>(
        [&] { (void)expert.predict(bad_boundary); },
        "PAQ8px expert accepted a context away from its prefix boundary");

    expert.reset_block(new_block);
    const std::vector<std::uint8_t> single_context = bytes("ABCDEFGHI");
    for (std::size_t i = 0; i < single_context.size(); ++i) {
        hz::r2::ExpertContext context{};
        context.absolute_position = 5000 + i;
        context.byte_in_block = static_cast<std::uint32_t>(i);
        (void)expert.predict(context);
        expert.observe(single_context[i], context);
    }
    hz::r2::ExpertContext end{};
    end.absolute_position = 5000 + single_context.size();
    end.byte_in_block = static_cast<std::uint32_t>(single_context.size());
    require(match_evidence(expert.predict(end)).candidates.empty(),
            "PAQ8px expert matched across an isolated block boundary");
}

}  // namespace

int main() {
    try {
        test_allocation_and_guards();
        test_lookup_orders_and_golden_trace();
        test_bucket_cap_dedup_and_priority();
        test_extension_recovery_and_saturation();
        test_collisions_reset_cache_and_rejections();
        test_service_field_conversion();
        test_expert_lifecycle_and_block_isolation();
        std::cout << "paq8px_match_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "paq8px_match_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
