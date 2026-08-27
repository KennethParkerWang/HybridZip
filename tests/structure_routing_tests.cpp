#include <cstdint>
#include <cstddef>
#include <string>
#include <stdexcept>
#include <vector>

#include "r2/representation/structure_analyzer.h"
#include "r2/routing/activation_router.h"
#include "r2/routing/block_features.h"
#include "r2/routing/mode_ranker.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void require_k8(const hz::r2::BlockFeaturesV1& features,
                const hz::r2::BlockClass expected_class,
                const hz::r2::BlockMode expected_extra) {
    const std::vector<hz::r2::BlockMode> modes = hz::r2::rank_modes_k8(features);
    if (features.classify() != expected_class) {
        throw std::runtime_error(
            "K=8 feature class mismatch: expected=" +
            std::to_string(static_cast<unsigned>(expected_class)) +
            " actual=" + std::to_string(static_cast<unsigned>(features.classify())));
    }
    require(modes.size() == 8U, "K=8 ranker did not return eight modes");
    for (std::size_t index = 0; index < modes.size(); ++index) {
        for (std::size_t peer = index + 1; peer < modes.size(); ++peer) {
            require(modes[index] != modes[peer], "K=8 ranker returned a duplicate mode");
        }
    }
    if (!hz::r2::shortlist_contains(modes, hz::r2::BlockMode::Stored) ||
        !hz::r2::shortlist_contains(modes, hz::r2::BlockMode::Zstd) ||
        !hz::r2::shortlist_contains(
            modes, hz::r2::BlockMode::Paq8pxGenericSse) ||
        !hz::r2::shortlist_contains(
            modes, hz::r2::BlockMode::Paq8pxDetectedSse) ||
        !hz::r2::shortlist_contains(modes, expected_extra)) {
        throw std::runtime_error(
            "K=8 ranker omitted a mandatory or class-specific mode: expected=" +
            std::to_string(static_cast<unsigned>(expected_extra)));
    }
}

void require_ablation_shortlists(const hz::r2::BlockFeaturesV1& features) {
    const std::vector<hz::r2::BlockMode> k2 =
        hz::r2::rank_modes_k2(features);
    require(k2.size() == 2U &&
                hz::r2::shortlist_contains(k2, hz::r2::BlockMode::Stored) &&
                hz::r2::shortlist_contains(
                    k2, hz::r2::BlockMode::Paq8pxGenericSse),
            "K=2 ablation shortlist is not canonical");

    const std::vector<hz::r2::BlockMode> k4 =
        hz::r2::rank_modes_k4(features);
    require(k4.size() == 4U &&
                hz::r2::shortlist_contains(k4, hz::r2::BlockMode::Stored) &&
                hz::r2::shortlist_contains(k4, hz::r2::BlockMode::Zstd) &&
                hz::r2::shortlist_contains(
                    k4, hz::r2::BlockMode::Paq8pxGenericSse) &&
                hz::r2::shortlist_contains(
                    k4, hz::r2::BlockMode::Paq8pxDetectedSse),
            "K=4 ablation shortlist is not canonical");
}

}  // namespace

int main() {
    require(hz::r2::fixed_point_ranker_model_v1_valid(),
            "Fixed-point ranker model checksum is invalid");
    require(hz::r2::fixed_point_ranker_model_v1().version == 0x00010000U,
            "Fixed-point ranker model version is not pinned");
    require(hz::r2::fixed_point_ranker_model_v1_sha256_hex() ==
                "4B1AC26C40AD4DA50312FD3B694D7E636FB768C2336FE773BC82D36424C27A4B",
            "Fixed-point ranker model SHA-256 is not pinned");
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
        x86[offset + 1] = 0U;
        x86[offset + 2] = 0U;
        x86[offset + 3] = 0U;
        x86[offset + 4] = 0U;
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

    const hz::r2::BlockFeaturesV1 text_k8 =
        hz::r2::extract_block_features(hz::r2::ByteView(text));
    const hz::r2::BlockFeaturesV1 text_k8_repeat =
        hz::r2::extract_block_features(hz::r2::ByteView(text));
    require(text_k8.byte_count == text_k8_repeat.byte_count &&
                text_k8.values == text_k8_repeat.values &&
                hz::r2::rank_modes_k8(text_k8) ==
                    hz::r2::rank_modes_k8(text_k8_repeat),
            "K=8 features changed between identical inputs");
    require(text_k8[hz::r2::BlockFeatureId::ByteEntropyQ12] > 0 &&
                text_k8[hz::r2::BlockFeatureId::PrintableFractionQ12] >
                    static_cast<std::int32_t>(3U * hz::r2::kBlockFeatureQ12One / 4U) &&
                text_k8.has_flag(hz::r2::kBlockFeatureUtf8),
            "Text block did not produce expected Q12/router features");
    require_k8(text_k8, hz::r2::BlockClass::Text,
               hz::r2::BlockMode::BrotliText);
    require_ablation_shortlists(text_k8);

    const hz::r2::BlockFeaturesV1 x86_k8 =
        hz::r2::extract_block_features(hz::r2::ByteView(x86));
    require_k8(x86_k8, hz::r2::BlockClass::X86,
               hz::r2::BlockMode::X86BcjZstd);

    const hz::r2::BlockFeaturesV1 numeric_k8 =
        hz::r2::extract_block_features(hz::r2::ByteView(correlated));
    require(numeric_k8[hz::r2::BlockFeatureId::BestPeriodicityQ12] >=
                static_cast<std::int32_t>(hz::r2::kBlockFeatureQ12One - 4U) &&
                numeric_k8[hz::r2::BlockFeatureId::SmallDeltaWidth1Q12] >=
                    static_cast<std::int32_t>(hz::r2::kBlockFeatureQ12One - 4U),
            "Periodic numeric block did not produce expected integer features");
    require_k8(numeric_k8, hz::r2::BlockClass::Numeric,
               hz::r2::BlockMode::ShuffleZstd);

    const hz::r2::BlockFeaturesV1 generic_k8 =
        hz::r2::extract_block_features(hz::r2::ByteView(noisy));
    require_k8(generic_k8, hz::r2::BlockClass::Generic,
               hz::r2::BlockMode::Ppmd8);

    const std::vector<std::uint8_t> zstd_magic{
        0x28U, 0xB5U, 0x2FU, 0xFDU, 0x00U, 0x00U, 0x00U, 0x00U};
    const hz::r2::BlockFeaturesV1 compressed_k8 =
        hz::r2::extract_block_features(hz::r2::ByteView(zstd_magic));
    require(compressed_k8.has_flag(hz::r2::kBlockFeatureKnownMagic) &&
                compressed_k8.has_flag(hz::r2::kBlockFeatureCompressedMagic),
            "Compressed magic was not retained in packed router flags");
    return 0;
}
