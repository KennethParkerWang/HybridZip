#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "r2/archive/r2_archive.h"
#include "r2/core/byte_view.h"
#include "r2/routing/activation_router.h"

namespace hz::r2 {

enum class CandidatePolicy {
    Auto,
    AutoK2,
    AutoK4,
    AutoK8,
    Fast,
    StoredOnly,
    PredictiveV1Only,
    ZstdOnly,
    FseOnly,
    LzmaOnly,
    DonorMatchPredictiveOnly,
    BwtZstdOnly,
    BwtMtfZstdOnly,
    BwtRltZstdOnly,
    X86BcjZstdOnly,
    ShuffleZstdOnly,
    BitshuffleZstdOnly,
    DeltaZstdOnly,
    DeltaOfDeltaZstdOnly,
    FastPforOnly,
    RansOnly,
    Bcj2ZstdOnly,
    RecordTransposeZstdOnly,
    JpegLsOnly,
    FlacResidualOnly,
    BrotliTextOnly,
    CmixWordDictionaryZstdOnly,
    NeuralLstmOnly,
    SharedNeuralLstmOnly,
    LstmCompressOnly,
    BgptSharedPriorOnly,
    JaxCompressPortableOnly,
    LmicArithmeticOnly,
    Ppmd7Only,
    Ppmd8Only,
    ZpaqOnly,
    CtwOnly,
    Paq8pxApmPredictiveOnly,
    Paq8pxRecordModelOnly,
    Paq8pxLinearPredictionOnly,
    Paq8pxSimilarityOnly,
    Paq8pxSimilaritySseOnly,
    Paq8pxGenericSseOnly,
    Paq8pxDetectedSseOnly,
    WavpackOnly,
    Lz4Only,
    KanziAnsOnly,
    DeltaBinaryPackedZstdOnly
};

struct BlockPlannerOptions {
    CandidatePolicy policy = CandidatePolicy::Auto;
    int zstd_level = 19;
    int lzma_level = 9;
    std::uint32_t lzma_dictionary_size = 0;
    std::uint64_t model_seed = kDefaultModelSeed;
};

struct BlockDecision {
    BlockMode mode = BlockMode::Stored;
    TransformKind transform = TransformKind::Raw;
    EntropyKind entropy = EntropyKind::Stored;
    std::vector<std::uint8_t> payload;
    std::vector<std::uint8_t> transform_metadata;
    std::size_t stored_candidate_bytes = 0;
    std::optional<std::size_t> predictive_candidate_bytes;
    std::optional<std::size_t> zstd_candidate_bytes;
    std::optional<std::size_t> fse_candidate_bytes;
    std::optional<std::size_t> lzma_candidate_bytes;
    std::optional<std::size_t> lz4_candidate_bytes;
    std::optional<std::size_t> kanzi_ans_candidate_bytes;
    std::optional<std::size_t> donor_match_predictive_candidate_bytes;
    std::optional<std::size_t> paq8px_apm_predictive_candidate_bytes;
    std::optional<std::size_t> paq8px_record_model_candidate_bytes;
    std::optional<std::size_t> paq8px_linear_prediction_candidate_bytes;
    std::optional<std::size_t> paq8px_similarity_candidate_bytes;
    std::optional<std::size_t> paq8px_similarity_sse_candidate_bytes;
    std::optional<std::size_t> paq8px_generic_sse_candidate_bytes;
    std::optional<std::size_t> paq8px_detected_sse_candidate_bytes;
    std::optional<std::size_t> bwt_zstd_candidate_bytes;
    std::optional<std::size_t> bwt_mtf_zstd_candidate_bytes;
    std::optional<std::size_t> bwt_rlt_zstd_candidate_bytes;
    std::optional<std::size_t> x86_bcj_zstd_candidate_bytes;
    std::optional<std::size_t> shuffle_zstd_candidate_bytes;
    std::optional<std::size_t> bitshuffle_zstd_candidate_bytes;
    std::optional<std::size_t> delta_zstd_candidate_bytes;
    std::optional<std::size_t> delta_of_delta_zstd_candidate_bytes;
    std::optional<std::size_t> fastpfor_candidate_bytes;
    std::optional<std::size_t> rans_candidate_bytes;
    std::optional<std::size_t> bcj2_zstd_candidate_bytes;
    std::optional<std::size_t> record_transpose_zstd_candidate_bytes;
    std::optional<std::size_t> jpeg_ls_candidate_bytes;
    std::optional<std::size_t> flac_residual_candidate_bytes;
    std::optional<std::size_t> wavpack_candidate_bytes;
    std::optional<std::size_t> brotli_text_candidate_bytes;
    std::optional<std::size_t> cmix_word_dictionary_zstd_candidate_bytes;
    std::optional<std::size_t> neural_lstm_candidate_bytes;
    std::optional<std::size_t> shared_neural_lstm_candidate_bytes;
    std::optional<std::size_t> lstm_compress_candidate_bytes;
    std::optional<std::size_t> bgpt_shared_prior_candidate_bytes;
    std::optional<std::size_t> jax_compress_portable_candidate_bytes;
    std::optional<std::size_t> lmic_arithmetic_candidate_bytes;
    std::optional<std::size_t> delta_binary_packed_zstd_candidate_bytes;
    std::optional<std::size_t> ppmd7_candidate_bytes;
    std::optional<std::size_t> ppmd8_candidate_bytes;
    std::optional<std::size_t> zpaq_candidate_bytes;
    std::optional<std::size_t> ctw_candidate_bytes;
    // Candidate accounting for one serialized block. Auto records the chosen
    // candidate and the evaluated oracle; forced modes are filled from the
    // final block framing by the codec.
    std::uint32_t candidates_evaluated = 0;
    // Encoder telemetry only. It is not serialized into HZ02 archives.
    std::array<std::uint32_t, 43> candidate_blocks_by_mode{};
    std::size_t selected_candidate_bytes = 0;
    std::size_t oracle_candidate_bytes = 0;
    std::size_t oracle_gap_bytes = 0;
};

class BlockPlanner {
public:
    explicit BlockPlanner(BlockPlannerOptions options);

    BlockDecision plan(ByteView input);

private:
    void update_family_telemetry(const BlockDecision& decision,
                                 std::size_t input_size);

    BlockPlannerOptions options_;
    std::vector<ExpertTelemetry> family_telemetry_;
};

}  // namespace hz::r2
