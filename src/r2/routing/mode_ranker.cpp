#include "r2/routing/mode_ranker.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

#include "r2/entropy/zpaq_backend.h"

namespace hz::r2 {
namespace {

constexpr std::uint32_t kFixedPointRankerVersion = 0x00010000U;

constexpr std::size_t mode_index(BlockMode mode) noexcept {
    return static_cast<std::size_t>(mode);
}

constexpr std::size_t feature_index(BlockFeatureId id) noexcept {
    return static_cast<std::size_t>(id);
}

void set_weight(FixedPointRankerModelV1& model, BlockMode mode,
                BlockFeatureId feature, std::int16_t weight) noexcept {
    model.weights[mode_index(mode)][feature_index(feature)] = weight;
}

void set_bias(FixedPointRankerModelV1& model, BlockMode mode,
              std::int32_t bias) noexcept {
    model.biases[mode_index(mode)] = bias;
}

std::uint32_t crc32_byte(std::uint32_t crc, std::uint8_t value) noexcept {
    crc ^= value;
    for (unsigned bit = 0U; bit < 8U; ++bit) {
        crc = (crc >> 1U) ^ ((crc & 1U) != 0U ? 0xEDB88320U : 0U);
    }
    return crc;
}

template <typename Unsigned>
void crc32_le(std::uint32_t& crc, Unsigned value) noexcept {
    for (std::size_t index = 0U; index < sizeof(Unsigned); ++index) {
        crc = crc32_byte(crc, static_cast<std::uint8_t>(value & 0xFFU));
        value = static_cast<Unsigned>(value >> 8U);
    }
}

template <typename Unsigned>
void append_le(
    std::array<std::uint8_t, kFixedPointRankerModelV1CanonicalByteCount>& bytes,
    std::size_t& offset, Unsigned value) noexcept {
    for (std::size_t index = 0U; index < sizeof(Unsigned); ++index) {
        bytes[offset++] = static_cast<std::uint8_t>(value & 0xFFU);
        value = static_cast<Unsigned>(value >> 8U);
    }
}

std::array<std::uint8_t, kFixedPointRankerModelV1CanonicalByteCount>
canonical_model_bytes(const FixedPointRankerModelV1& model) noexcept {
    std::array<std::uint8_t, kFixedPointRankerModelV1CanonicalByteCount> bytes{};
    std::size_t offset = 0U;
    for (const auto& row : model.weights) {
        for (const std::int16_t weight : row) {
            append_le(bytes, offset, static_cast<std::uint16_t>(weight));
        }
    }
    for (const std::int32_t bias : model.biases) {
        append_le(bytes, offset, static_cast<std::uint32_t>(bias));
    }
    for (const std::int16_t shift : model.feature_shifts) {
        append_le(bytes, offset, static_cast<std::uint16_t>(shift));
    }
    append_le(bytes, offset, model.version);
    append_le(bytes, offset, model.crc32);
    return bytes;
}

std::string calculate_model_sha256_hex(const FixedPointRankerModelV1& model) {
    const auto bytes = canonical_model_bytes(model);
    return zpaq_donor_sha256_hex(ByteView(bytes.data(), bytes.size()));
}

std::uint32_t calculate_model_crc32(const FixedPointRankerModelV1& model) noexcept {
    std::uint32_t crc = 0xFFFFFFFFU;
    for (const auto& row : model.weights) {
        for (const std::int16_t weight : row) {
            crc32_le(crc, static_cast<std::uint16_t>(weight));
        }
    }
    for (const std::int32_t bias : model.biases) {
        crc32_le(crc, static_cast<std::uint32_t>(bias));
    }
    for (const std::int16_t shift : model.feature_shifts) {
        crc32_le(crc, static_cast<std::uint16_t>(shift));
    }
    crc32_le(crc, model.version);
    return crc ^ 0xFFFFFFFFU;
}

FixedPointRankerModelV1 make_model() noexcept {
    FixedPointRankerModelV1 model{};
    model.version = kFixedPointRankerVersion;

    // Fractions and entropy are Q12. Shift them by six before multiplying so
    // the full signed-64 score remains comfortably bounded on every HZ02 block.
    for (BlockFeatureId feature : {
             BlockFeatureId::MaximumByteFrequencyQ12,
             BlockFeatureId::ZeroFractionQ12,
             BlockFeatureId::FfFractionQ12,
             BlockFeatureId::PrintableFractionQ12,
             BlockFeatureId::HighBitFractionQ12,
             BlockFeatureId::WhitespaceFractionQ12,
             BlockFeatureId::NewlineFractionQ12,
             BlockFeatureId::DigitFractionQ12,
             BlockFeatureId::MarkupFractionQ12,
             BlockFeatureId::SourcePunctuationFractionQ12,
             BlockFeatureId::EqualAdjacentFractionQ12,
             BlockFeatureId::LongRunCoverageQ12,
             BlockFeatureId::LongestRunLog2Q12,
             BlockFeatureId::SmallDeltaWidth1Q12,
             BlockFeatureId::SmallDeltaWidth2Q12,
             BlockFeatureId::SmallDeltaWidth4Q12,
             BlockFeatureId::SmallDeltaWidth8Q12,
             BlockFeatureId::LowByteConcentrationQ12,
             BlockFeatureId::BestPeriodicityQ12,
             BlockFeatureId::SampledLzCoverageQ12,
             BlockFeatureId::X86RelativeTargetDensityQ12}) {
        model.feature_shifts[feature_index(feature)] = 6;
    }
    model.feature_shifts[feature_index(BlockFeatureId::ByteEntropyQ12)] = 12;
    model.feature_shifts[feature_index(BlockFeatureId::CoarseEntropyQ12)] = 12;
    model.feature_shifts[feature_index(BlockFeatureId::LongestRunLog2Q12)] = 12;
    model.feature_shifts[feature_index(BlockFeatureId::SampledLzMeanLengthQ12)] = 12;

    // This is a deterministic bootstrap model, not an E5-trained model. The
    // PAQ SSE modes remain mandatory by policy; weights below only rank quota
    // slots and global fills until forced-mode no-leakage labels are available.
    set_bias(model, BlockMode::Zstd, 180);
    set_weight(model, BlockMode::Zstd, BlockFeatureId::SampledLzCoverageQ12, 3);
    set_weight(model, BlockMode::Zstd, BlockFeatureId::ByteEntropyQ12, 1);

    set_bias(model, BlockMode::Fse, 160);
    set_weight(model, BlockMode::Fse, BlockFeatureId::ByteEntropyQ12, 3);
    set_weight(model, BlockMode::Fse, BlockFeatureId::MaximumByteFrequencyQ12, -2);

    set_bias(model, BlockMode::Lzma, 150);
    set_weight(model, BlockMode::Lzma, BlockFeatureId::SampledLzCoverageQ12, 4);
    set_weight(model, BlockMode::Lzma, BlockFeatureId::LongRunCoverageQ12, 2);

    set_bias(model, BlockMode::Ppmd7, 130);
    set_weight(model, BlockMode::Ppmd7, BlockFeatureId::PrintableFractionQ12, 5);
    set_weight(model, BlockMode::Ppmd7, BlockFeatureId::WhitespaceFractionQ12, 2);
    set_weight(model, BlockMode::Ppmd7, BlockFeatureId::ByteEntropyQ12, -1);

    set_bias(model, BlockMode::Ppmd8, 140);
    set_weight(model, BlockMode::Ppmd8, BlockFeatureId::PrintableFractionQ12, 6);
    set_weight(model, BlockMode::Ppmd8, BlockFeatureId::MarkupFractionQ12, 2);

    set_bias(model, BlockMode::BrotliText, 145);
    set_weight(model, BlockMode::BrotliText, BlockFeatureId::PrintableFractionQ12, 7);
    set_weight(model, BlockMode::BrotliText, BlockFeatureId::MarkupFractionQ12, 4);
    set_weight(model, BlockMode::BrotliText, BlockFeatureId::WhitespaceFractionQ12, 2);

    set_bias(model, BlockMode::CmixWordDictionaryZstd, 110);
    set_weight(model, BlockMode::CmixWordDictionaryZstd,
               BlockFeatureId::PrintableFractionQ12, 5);
    set_weight(model, BlockMode::CmixWordDictionaryZstd,
               BlockFeatureId::WhitespaceFractionQ12, 4);

    set_bias(model, BlockMode::BwtZstd, 120);
    set_weight(model, BlockMode::BwtZstd, BlockFeatureId::LongRunCoverageQ12, 5);
    set_weight(model, BlockMode::BwtZstd, BlockFeatureId::SampledLzCoverageQ12, 2);
    set_bias(model, BlockMode::BwtMtfZstd, 118);
    set_weight(model, BlockMode::BwtMtfZstd, BlockFeatureId::LongRunCoverageQ12, 6);

    set_bias(model, BlockMode::X86BcjZstd, 125);
    set_weight(model, BlockMode::X86BcjZstd,
               BlockFeatureId::X86RelativeTargetDensityQ12, 12);
    set_bias(model, BlockMode::Bcj2Zstd, 122);
    set_weight(model, BlockMode::Bcj2Zstd,
               BlockFeatureId::X86RelativeTargetDensityQ12, 11);

    set_bias(model, BlockMode::ShuffleZstd, 120);
    set_weight(model, BlockMode::ShuffleZstd,
               BlockFeatureId::SmallDeltaWidth4Q12, 7);
    set_weight(model, BlockMode::ShuffleZstd,
               BlockFeatureId::BestPeriodicityQ12, 4);
    set_bias(model, BlockMode::BitshuffleZstd, 118);
    set_weight(model, BlockMode::BitshuffleZstd,
               BlockFeatureId::SmallDeltaWidth4Q12, 8);
    set_bias(model, BlockMode::DeltaZstd, 124);
    set_weight(model, BlockMode::DeltaZstd,
               BlockFeatureId::SmallDeltaWidth1Q12, 6);
    set_weight(model, BlockMode::DeltaZstd,
               BlockFeatureId::BestPeriodicityQ12, 6);
    set_bias(model, BlockMode::DeltaOfDeltaZstd, 116);
    set_weight(model, BlockMode::DeltaOfDeltaZstd,
               BlockFeatureId::SmallDeltaWidth8Q12, 7);
    set_bias(model, BlockMode::FastPfor, 108);
    set_weight(model, BlockMode::FastPfor,
               BlockFeatureId::LowByteConcentrationQ12, 4);
    set_weight(model, BlockMode::FastPfor,
               BlockFeatureId::SmallDeltaWidth4Q12, 5);
    set_bias(model, BlockMode::RecordTransposeZstd, 105);
    set_weight(model, BlockMode::RecordTransposeZstd,
               BlockFeatureId::BestPeriodicityQ12, 5);

    set_bias(model, BlockMode::Rans, 80);
    set_weight(model, BlockMode::Rans, BlockFeatureId::ByteEntropyQ12, 2);
    set_bias(model, BlockMode::Lz4, 75);
    set_weight(model, BlockMode::Lz4, BlockFeatureId::SampledLzCoverageQ12, 2);

    model.crc32 = calculate_model_crc32(model);
    return model;
}

const FixedPointRankerModelV1 kModel = make_model();
const std::string kModelSha256 = calculate_model_sha256_hex(kModel);

void append_unique(std::vector<BlockMode>& modes, BlockMode mode) {
    if (!shortlist_contains(modes, mode)) modes.push_back(mode);
}

bool contains_mode(const std::vector<BlockMode>& modes, BlockMode mode) noexcept {
    return std::find(modes.begin(), modes.end(), mode) != modes.end();
}

BlockMode best_scored(const BlockFeaturesV1& features,
                      const std::vector<BlockMode>& modes,
                      const std::vector<BlockMode>& selected) noexcept {
    BlockMode best = BlockMode::Stored;
    std::int64_t best_score = std::numeric_limits<std::int64_t>::min();
    bool found = false;
    for (const BlockMode mode : modes) {
        if (contains_mode(selected, mode)) continue;
        const std::int64_t score = fixed_point_ranker_score(features, mode);
        if (!found || score > best_score ||
            (score == best_score && mode_index(mode) < mode_index(best))) {
            best = mode;
            best_score = score;
            found = true;
        }
    }
    return best;
}

void append_best(std::vector<BlockMode>& selected,
                 const BlockFeaturesV1& features,
                 const std::vector<BlockMode>& modes) {
    if (selected.size() >= 8U || modes.empty()) return;
    const BlockMode best = best_scored(features, modes, selected);
    if (!contains_mode(selected, best)) append_unique(selected, best);
}

std::vector<BlockMode> representation_modes(const BlockFeaturesV1& features) {
    switch (features.classify()) {
    case BlockClass::Text:
        return {BlockMode::BwtZstd, BlockMode::BwtMtfZstd};
    case BlockClass::X86:
        return {BlockMode::X86BcjZstd, BlockMode::Bcj2Zstd};
    case BlockClass::Numeric:
        return {BlockMode::DeltaZstd, BlockMode::ShuffleZstd,
                BlockMode::BitshuffleZstd, BlockMode::DeltaOfDeltaZstd};
    case BlockClass::Generic:
        return {};
    }
    return {};
}

std::vector<BlockMode> specialist_modes(const BlockFeaturesV1& features) {
    if (features.classify() == BlockClass::Text) {
        return {BlockMode::BrotliText, BlockMode::CmixWordDictionaryZstd,
                BlockMode::Ppmd8, BlockMode::Ppmd7};
    }
    return {};
}

std::vector<BlockMode> high_ratio_modes(const BlockFeaturesV1& features) {
    switch (features.classify()) {
    case BlockClass::Text:
        return {BlockMode::Ppmd8, BlockMode::Ppmd7, BlockMode::Lzma,
                BlockMode::BwtZstd};
    case BlockClass::X86:
        return {BlockMode::Lzma, BlockMode::Fse, BlockMode::X86BcjZstd};
    case BlockClass::Numeric:
        return {BlockMode::Lzma, BlockMode::Fse, BlockMode::DeltaZstd};
    case BlockClass::Generic:
        return {BlockMode::Fse, BlockMode::Lzma, BlockMode::Ppmd8,
                BlockMode::Ppmd7};
    }
    return {};
}

}  // namespace

const FixedPointRankerModelV1& fixed_point_ranker_model_v1() noexcept {
    return kModel;
}

bool fixed_point_ranker_model_v1_valid() noexcept {
    return kModel.version == kFixedPointRankerVersion &&
        kModel.crc32 == calculate_model_crc32(kModel);
}

const std::string& fixed_point_ranker_model_v1_sha256_hex() noexcept {
    return kModelSha256;
}

std::int64_t fixed_point_ranker_score(const BlockFeaturesV1& features,
                                      const BlockMode mode) noexcept {
    const std::size_t index = mode_index(mode);
    if (index >= kFixedPointRankerModeCount) {
        return std::numeric_limits<std::int64_t>::min();
    }
    std::int64_t score = kModel.biases[index];
    for (std::size_t feature = 0U; feature < kBlockFeatureCount; ++feature) {
        const std::int16_t shift = kModel.feature_shifts[feature];
        const std::int64_t value = static_cast<std::int64_t>(features.values[feature]) >>
            (shift > 0 ? shift : 0);
        score += static_cast<std::int64_t>(kModel.weights[index][feature]) * value;
    }
    return score;
}

std::vector<BlockMode> rank_modes(const BlockFeaturesV1& features,
                                  const std::uint8_t candidate_count) {
    if (candidate_count != 2U && candidate_count != 4U && candidate_count != 8U) {
        throw std::invalid_argument("R2 shortlist size must be 2, 4, or 8");
    }
    if (!fixed_point_ranker_model_v1_valid()) {
        throw std::logic_error("R2 fixed-point ranker model checksum is invalid");
    }

    std::vector<BlockMode> modes;
    modes.reserve(candidate_count);
    append_unique(modes, BlockMode::Stored);
    append_unique(modes, BlockMode::Paq8pxGenericSse);
    if (candidate_count == 2U) return modes;

    append_unique(modes, BlockMode::Zstd);
    append_unique(modes, BlockMode::Paq8pxDetectedSse);
    if (candidate_count == 4U) return modes;

    const std::vector<BlockMode> generic_lz{
        BlockMode::Zstd, BlockMode::Fse, BlockMode::Lzma, BlockMode::Rans,
        BlockMode::Lz4};
    append_best(modes, features, generic_lz);
    append_best(modes, features, representation_modes(features));
    append_best(modes, features, specialist_modes(features));
    append_best(modes, features, high_ratio_modes(features));

    std::vector<BlockMode> global_fallback{
        BlockMode::Fse, BlockMode::Lzma, BlockMode::Rans, BlockMode::Lz4};
    // Hard family gates run before the fixed-point order. They keep a text
    // bias from activating a text transform on x86/numeric bytes and preserve
    // at least one applicable specialist/representation family per shortlist.
    switch (features.classify()) {
    case BlockClass::Text:
        global_fallback.insert(global_fallback.end(), {
            BlockMode::Ppmd8, BlockMode::Ppmd7, BlockMode::BrotliText,
            BlockMode::CmixWordDictionaryZstd, BlockMode::BwtZstd,
            BlockMode::BwtMtfZstd});
        break;
    case BlockClass::X86:
        global_fallback.insert(global_fallback.end(), {
            BlockMode::X86BcjZstd, BlockMode::Bcj2Zstd});
        break;
    case BlockClass::Numeric:
        global_fallback.insert(global_fallback.end(), {
            BlockMode::DeltaZstd, BlockMode::ShuffleZstd,
            BlockMode::BitshuffleZstd, BlockMode::DeltaOfDeltaZstd});
        break;
    case BlockClass::Generic:
        global_fallback.insert(global_fallback.end(), {
            BlockMode::Ppmd8, BlockMode::Ppmd7});
        break;
    }
    while (modes.size() < candidate_count) {
        const BlockMode best = best_scored(features, global_fallback, modes);
        if (contains_mode(modes, best)) break;
        append_unique(modes, best);
    }
    if (modes.size() != candidate_count) {
        throw std::logic_error("R2 fixed-point ranker did not fill the shortlist");
    }
    return modes;
}

std::vector<BlockMode> rank_modes(ByteView input, std::uint8_t candidate_count) {
    return rank_modes(extract_block_features(input), candidate_count);
}

std::vector<BlockMode> rank_modes_k2(const BlockFeaturesV1& features) {
    return rank_modes(features, 2U);
}

std::vector<BlockMode> rank_modes_k4(const BlockFeaturesV1& features) {
    return rank_modes(features, 4U);
}

std::vector<BlockMode> rank_modes_k8(const BlockFeaturesV1& features) {
    return rank_modes(features, 8U);
}

std::vector<BlockMode> rank_modes_k8(ByteView input) {
    return rank_modes(input, 8U);
}

bool shortlist_contains(const std::vector<BlockMode>& modes,
                        const BlockMode mode) noexcept {
    return contains_mode(modes, mode);
}

}  // namespace hz::r2
