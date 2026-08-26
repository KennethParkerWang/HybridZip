#include <cstdint>
#include <stdexcept>
#include <vector>

#include "r2/representation/structure_analyzer.h"
#include "r2/routing/activation_router.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

}  // namespace

int main() {
    const hz::r2::StructureAnalyzer analyzer;
    const hz::r2::StructureActivationRouter router;

    std::vector<std::uint8_t> text;
    for (int repeat = 0; repeat < 256; ++repeat) {
        constexpr char kSentence[] = "deterministic representation routing\n";
        text.insert(text.end(), kSentence, kSentence + sizeof(kSentence) - 1);
    }
    const hz::r2::StructureFeatures text_features = analyzer.analyze(
        hz::r2::ByteView(text));
    const hz::r2::RepresentationActivation text_activation = router.activate(text_features);
    require(text_features.printable_fraction > 0.9,
            "Text printable fraction was not measured");
    require(text_features.longest_match >= 12,
            "Repeated windows were not measured");
    require(text_activation.bwt_zstd && text_activation.bwt_mtf_zstd,
            "Structured text did not activate BWT candidates");

    std::vector<std::uint8_t> noisy(4096);
    std::uint32_t state = 0xA3C59AC3U;
    for (std::uint8_t& value : noisy) {
        state = state * 1664525U + 1013904223U;
        value = static_cast<std::uint8_t>(state >> 24U);
    }
    const hz::r2::RepresentationActivation noisy_activation = router.activate(
        analyzer.analyze(hz::r2::ByteView(noisy)));
    require(!noisy_activation.bwt_zstd && !noisy_activation.delta_zstd,
            "Unstructured bytes activated expensive representations");

    std::vector<std::uint8_t> x86(4096, 0x90U);
    for (std::size_t offset = 0; offset + 5 <= x86.size(); offset += 16) {
        x86[offset] = 0xE8U;
    }
    require(router.activate(analyzer.analyze(hz::r2::ByteView(x86))).x86_bcj_zstd,
            "x86 branch density did not activate BCJ");

    std::vector<std::uint8_t> correlated(4096, 0U);
    for (std::size_t index = 0; index < correlated.size(); ++index) {
        correlated[index] = static_cast<std::uint8_t>(index & 1U);
    }
    const auto correlated_activation = router.activate(
        analyzer.analyze(hz::r2::ByteView(correlated)));
    require(correlated_activation.lstm_compress,
            "Low-printable correlated bytes did not activate LSTM-Compress");

    const hz::r2::StructureFeatures text_structure = analyzer.analyze(
        hz::r2::ByteView(text));
    std::vector<hz::r2::ExpertTelemetry> telemetry(
        hz::r2::HierarchicalActivationRouter::family_slot_count());
    for (auto& item : telemetry) {
        item.recent_log_loss_256 = 1.0;
    }
    hz::r2::HierarchicalActivationRouter hierarchical;
    const std::vector<bool> text_families = hierarchical.active_experts(
        text_structure, telemetry);
    require(text_families.size() == telemetry.size() &&
                text_families[0] && text_families[1] && text_families[2] &&
                text_families[5] && text_families[7],
            "Text family router did not retain expected causal families");

    telemetry[2].recent_log_loss_256 = 9.0;
    telemetry[2].age = 8192;
    require(!hierarchical.active_experts(text_structure, telemetry)[2],
            "Stale high-loss neural family was not put to sleep");
    return 0;
}
