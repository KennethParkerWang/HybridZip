#include "r2/routing/activation_router.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace hz::r2 {

RepresentationActivation StructureActivationRouter::activate(
    const StructureFeatures& structure) const {
    const bool structured = structure.entropy_bits < 7.9 &&
        (structure.printable_fraction >= 0.25 ||
         structure.repeated_window_fraction >= 0.02 ||
         structure.longest_match >= 12);
    const double delta_similarity = std::max(
        std::max(structure.delta_similarity_1, structure.delta_similarity_2),
        std::max(structure.delta_similarity_4, structure.delta_similarity_8));
    const bool numeric_correlation = delta_similarity >= 0.12 ||
        structure.zero_fraction >= 0.08;

    RepresentationActivation activation{};
    activation.bwt_zstd = structured;
    activation.bwt_mtf_zstd = structured;
    activation.bwt_rlt_zstd = structured;
    activation.x86_bcj_zstd = structure.entropy_bits < 7.95 &&
        structure.x86_branch_fraction >= 0.002;
    activation.shuffle_zstd = numeric_correlation;
    activation.bitshuffle_zstd = numeric_correlation;
    activation.delta_zstd = numeric_correlation;
    activation.delta_of_delta_zstd = numeric_correlation &&
        (structure.delta_similarity_4 >= 0.16 ||
         structure.delta_similarity_8 >= 0.16);
    activation.fastpfor = numeric_correlation;
    activation.record_transpose_zstd = numeric_correlation;
    activation.jpeg_ls = structure.entropy_bits < 7.8 &&
        structure.printable_fraction < 0.40 &&
        structure.image_gradient_score >= 0.82;
    activation.flac_residual = structure.entropy_bits < 7.9 &&
        structure.printable_fraction < 0.35 &&
        structure.delta_similarity_2 >= 0.10;
    activation.wavpack = activation.flac_residual;
    activation.brotli_text = structure.entropy_bits < 7.9 &&
        structure.printable_fraction >= 0.70 &&
        (structure.whitespace_fraction >= 0.01 ||
         structure.markup_or_code_fraction >= 0.02);
    activation.cmix_word_dictionary_zstd = activation.brotli_text &&
        structure.whitespace_fraction >= 0.04;
    // These are portfolio candidates, not unconditional fallbacks.  The
    // complete archive still has to beat the current winner before selection.
    activation.rans = structure.entropy_bits >= 5.0 ||
        structure.zero_fraction >= 0.02;
    activation.neural_lstm = structure.entropy_bits < 7.95 &&
        structure.printable_fraction >= 0.35;
    activation.shared_neural_lstm = activation.neural_lstm &&
        (structure.whitespace_fraction >= 0.01 ||
         structure.markup_or_code_fraction >= 0.02);
    // The report-listed 90-cell donor profile also learns binary/mixed blocks
    // where printable text is scarce. Keep it separate from the more
    // expensive full neural gates and use only byte-derived structure.
    activation.lstm_compress = structure.entropy_bits < 7.95 &&
        (structure.printable_fraction >= 0.20 ||
         structure.delta_similarity_2 >= 0.40 ||
         structure.zero_fraction >= 0.20);
    activation.bgpt_shared_prior = activation.brotli_text;
    activation.jax_compress_portable = activation.neural_lstm;
    activation.lmic_arithmetic = activation.brotli_text;
    activation.delta_binary_packed_zstd = numeric_correlation;
    return activation;
}

bool HierarchicalActivationRouter::family_gate(
    const std::size_t slot,
    const StructureFeatures& structure) noexcept {
    const bool structured = structure.entropy_bits < 7.9 &&
        (structure.printable_fraction >= 0.25 ||
         structure.repeated_window_fraction >= 0.02 ||
         structure.longest_match >= 12);
    const bool match = structure.repeated_window_fraction >= 0.01 ||
        structure.longest_match >= 12;
    const bool neural = structure.entropy_bits < 7.95 &&
        (structure.printable_fraction >= 0.35 ||
         structure.delta_similarity_2 >= 0.40 ||
         structure.zero_fraction >= 0.20);
    const bool numeric = std::max(
        std::max(structure.delta_similarity_1, structure.delta_similarity_2),
        std::max(structure.delta_similarity_4, structure.delta_similarity_8)) >=
            0.12 || structure.zero_fraction >= 0.08;
    const bool text = structure.printable_fraction >= 0.45 &&
        (structure.whitespace_fraction >= 0.01 ||
         structure.markup_or_code_fraction >= 0.02);
    const bool media = structure.image_gradient_score >= 0.82 ||
        (structure.delta_similarity_2 >= 0.10 &&
         structure.printable_fraction < 0.35);
    switch (slot) {
    case 0:  // statistical generalists are always available.
    case 7:  // LZ/coding candidates are cheap enough to retain as a fallback.
        return true;
    case 1:
        return match;
    case 2:
        return neural;
    case 3:
        return structured;
    case 4:
        return numeric;
    case 5:
        return text;
    case 6:
        return media;
    default:
        return false;
    }
}

std::vector<bool> HierarchicalActivationRouter::active_experts(
    const StructureFeatures& structure,
    const std::vector<ExpertTelemetry>& telemetry) {
    std::vector<bool> active(telemetry.size(), false);
    if (telemetry.empty()) {
        return active;
    }

    constexpr double kStaleLossMargin = 2.0;
    for (std::size_t index = 0; index < telemetry.size(); ++index) {
        if (!family_gate(index, structure)) {
            continue;
        }
        const ExpertTelemetry& candidate = telemetry[index];
        const double candidate_loss = candidate.recent_log_loss_256;
        if (!std::isfinite(candidate_loss) || candidate.age <= 4096U) {
            active[index] = true;
            continue;
        }

        double best_loss = std::numeric_limits<double>::infinity();
        for (std::size_t peer = 0; peer < telemetry.size(); ++peer) {
            if (family_gate(peer, structure) &&
                std::isfinite(telemetry[peer].recent_log_loss_256)) {
                best_loss = std::min(best_loss,
                                     telemetry[peer].recent_log_loss_256);
            }
        }
        active[index] = candidate_loss <= best_loss + kStaleLossMargin;
    }
    return active;
}

}  // namespace hz::r2
