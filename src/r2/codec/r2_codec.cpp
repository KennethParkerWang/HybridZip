#include "r2/codec/r2_codec.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <vector>

#include "r2/archive/r2_archive.h"
#include "r2/core/byte_view.h"
#include "r2/entropy/donor_match_predictive_backend.h"
#include "r2/entropy/paq8px_apm_backend.h"
#include "r2/entropy/paq8px_record_model_backend.h"
#include "r2/entropy/paq8px_linear_prediction_backend.h"
#include "r2/entropy/paq8px_similarity_backend.h"
#include "r2/entropy/paq8px_similarity_sse_backend.h"
#include "r2/entropy/paq8px_generic_sse_backend.h"
#include "r2/entropy/paq8px_detected_sse_backend.h"
#include "r2/entropy/wavpack_backend.h"
#include "r2/entropy/predictive_v1_backend.h"
#include "r2/entropy/fse_backend.h"
#include "r2/entropy/fastpfor_backend.h"
#include "r2/entropy/fast_extension_backend.h"
#include "r2/entropy/rans_backend.h"
#include "r2/entropy/lzma_backend.h"
#include "r2/entropy/lz4_backend.h"
#include "r2/entropy/kanzi_ans_backend.h"
#include "r2/entropy/ppmd7_backend.h"
#include "r2/entropy/ppmd8_backend.h"
#include "r2/entropy/zpaq_backend.h"
#include "r2/entropy/ctw_backend.h"
#include "r2/entropy/neural_lstm_backend.h"
#include "r2/entropy/lstm_compress_backend.h"
#include "r2/entropy/bgpt_shared_prior_backend.h"
#include "r2/entropy/jax_compress_portable_backend.h"
#include "r2/entropy/lmic_arithmetic_backend.h"
#include "r2/entropy/stored_backend.h"
#include "r2/entropy/zstd_backend.h"
#include "r2/representation/bwt_transform.h"
#include "r2/representation/kanzi_mtf_transform.h"
#include "r2/representation/kanzi_rlt_transform.h"
#include "r2/representation/xz_x86_bcj_transform.h"
#include "r2/representation/bcj2_transform.h"
#include "r2/representation/jpegls_transform.h"
#include "r2/representation/flac_residual_transform.h"
#include "r2/representation/brotli_text_transform.h"
#include "r2/representation/cmix_word_dictionary_transform.h"
#include "r2/representation/record_transpose_transform.h"
#include "r2/representation/blosc_shuffle_transform.h"
#include "r2/representation/blosc_bitshuffle_transform.h"
#include "r2/representation/blosc_delta_transform.h"
#include "r2/representation/delta_of_delta_transform.h"
#include "r2/representation/delta_binary_packed_transform.h"
#include "r2/runtime/block_executor.h"

namespace hz::r2 {
namespace {

constexpr std::array<char, 4> kMagic{'H', 'Z', '0', '2'};

std::filesystem::path temporary_path_for(const std::filesystem::path& output) {
    std::filesystem::path temporary = output;
    temporary += ".tmp";
    return temporary;
}

void validate_paths(const std::filesystem::path& input,
                    const std::filesystem::path& output,
                    const std::filesystem::path& temporary) {
    if (!std::filesystem::is_regular_file(input)) {
        throw std::runtime_error("R2 input is not a regular file");
    }
    if (std::filesystem::absolute(input).lexically_normal() ==
        std::filesystem::absolute(output).lexically_normal()) {
        throw std::runtime_error("R2 input and output paths must differ");
    }
    if (std::filesystem::exists(output)) {
        throw std::runtime_error("Refusing to overwrite an existing R2 output");
    }
    if (std::filesystem::exists(temporary)) {
        throw std::runtime_error("R2 temporary output path already exists");
    }
}

std::uint32_t block_count_for(const std::uint64_t size,
                              const std::uint32_t block_size) {
    const std::uint64_t count =
        size / block_size + (size % block_size != 0 ? 1U : 0U);
    if (count > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("Input requires too many HZ02 blocks");
    }
    return static_cast<std::uint32_t>(count);
}

void read_exact(std::istream& input,
                std::vector<std::uint8_t>& bytes,
                const char* failure) {
    if (bytes.empty()) {
        return;
    }
    input.read(reinterpret_cast<char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
    if (input.gcount() != static_cast<std::streamsize>(bytes.size())) {
        throw std::runtime_error(failure);
    }
}

std::uint32_t read_u32_le(const ByteView input) {
    if (input.size() != 4U) {
        throw std::runtime_error("HZ02 uint32 metadata is malformed");
    }
    return static_cast<std::uint32_t>(input[0]) |
           (static_cast<std::uint32_t>(input[1]) << 8U) |
           (static_cast<std::uint32_t>(input[2]) << 16U) |
           (static_cast<std::uint32_t>(input[3]) << 24U);
}

std::vector<std::uint8_t> decode_block(const BlockHeader& header,
                                       const ByteView payload,
                                       const ByteView transform_metadata,
                                       const std::uint64_t model_seed) {
    std::vector<std::uint8_t> decoded;
    switch (header.mode) {
        case BlockMode::Stored:
            decoded = StoredBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::PredictiveV1:
            decoded = PredictiveV1Backend(model_seed).decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::NeuralLstm:
            decoded = NeuralLstmBackend(model_seed).decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::SharedNeuralLstm:
            if (transform_metadata.size() != 4U ||
                read_u32_le(transform_metadata) != kSharedNeuralModelId) {
                throw std::runtime_error(
                    "HZ02 shared neural model identity is unsupported");
            }
            decoded = NeuralLstmBackend(kSharedNeuralModelSeed).decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::LstmCompress:
            decoded = LstmCompressBackend(model_seed).decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::BgptSharedPrior:
            if (transform_metadata.size() !=
                    kR2BgptSharedPriorIdentitySize ||
                read_u32_le(ByteView(transform_metadata.data(), 4U)) !=
                    kBgptSharedPriorModelId ||
                !std::equal(kBgptTextCheckpointSha256.begin(),
                            kBgptTextCheckpointSha256.end(),
                            transform_metadata.data() + 4U)) {
                throw std::runtime_error(
                    "HZ02 bGPT shared-prior identity is unsupported");
            }
            decoded = BgptSharedPriorBackend().decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::JaxCompressPortable:
            if (transform_metadata.size() !=
                    kR2JaxCompressPortableIdentitySize ||
                read_u32_le(ByteView(transform_metadata.data(), 4U)) !=
                    kJaxCompressPortableModelId ||
                !std::equal(kJaxCompressSourceRevision.begin(),
                            kJaxCompressSourceRevision.end(),
                            transform_metadata.data() + 4U) ||
                !std::equal(kJaxCompressPortableProfileSha256.begin(),
                            kJaxCompressPortableProfileSha256.end(),
                            transform_metadata.data() + 24U)) {
                throw std::runtime_error(
                    "HZ02 jax-compress portable identity is unsupported");
            }
            decoded = JaxCompressPortableBackend().decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::LmicArithmetic:
            if (transform_metadata.size() !=
                    kR2LmicArithmeticIdentitySize ||
                read_u32_le(ByteView(transform_metadata.data(), 4U)) !=
                    kLmicArithmeticModelId ||
                !std::equal(kLmicArithmeticProfileSha256.begin(),
                            kLmicArithmeticProfileSha256.end(),
                            transform_metadata.data() + 4U)) {
                throw std::runtime_error(
                    "HZ02 LMIC arithmetic identity is unsupported");
            }
            decoded = LmicArithmeticBackend().decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::Zstd:
            decoded = ZstdBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::FastExtension:
        {
            FastExtensionDecodedBlock extension =
                FastExtensionBackend::decode_zstd(
                    payload, transform_metadata, header.uncompressed_size);
            decoded = std::move(extension.bytes);
            switch (extension.metadata.transform) {
            case FastExtensionTransform::None:
                break;
            case FastExtensionTransform::ByteShuffle:
                decoded = BloscShuffleTransform().inverse(
                    ByteView(decoded), extension.metadata.side_information[0]);
                break;
            case FastExtensionTransform::BitShuffle:
                decoded = BloscBitshuffleTransform().inverse(
                    ByteView(decoded), extension.metadata.side_information[0]);
                break;
            case FastExtensionTransform::XorDelta:
                decoded = BloscDeltaTransform().inverse(
                    ByteView(decoded), extension.metadata.side_information[0]);
                break;
            case FastExtensionTransform::X86Bcj:
                decoded = XzX86BcjTransform().inverse(ByteView(decoded), ByteView{});
                break;
            }
            break;
        }
        case BlockMode::Fse:
            decoded = FseBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::Lzma:
            decoded = LzmaBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::Lz4:
            decoded = Lz4Backend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::KanziAns:
            decoded = KanziAnsBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::Ppmd7:
            decoded = Ppmd7Backend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::Ppmd8:
            decoded = Ppmd8Backend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::Zpaq:
            decoded = ZpaqBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::Ctw:
            decoded = CtwBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::DonorMatchPredictive:
            decoded = DonorMatchPredictiveBackend(model_seed).decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::Paq8pxApmPredictive:
            decoded = Paq8pxApmBackend(model_seed).decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::Paq8pxRecordModel:
            decoded = Paq8pxRecordModelBackend().decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::Paq8pxLinearPrediction:
            decoded = Paq8pxLinearPredictionBackend().decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::Paq8pxSimilarity:
            decoded = Paq8pxSimilarityBackend().decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::Paq8pxSimilaritySse:
            decoded = Paq8pxSimilaritySseBackend().decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::Paq8pxGenericSse:
            decoded = Paq8pxGenericSseBackend().decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::Paq8pxDetectedSse:
            decoded = Paq8pxDetectedSseBackend().decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::Wavpack:
            decoded = WavpackBackend().decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::BwtZstd:
            decoded = ZstdBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::BwtMtfZstd:
            decoded = ZstdBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::BwtRltZstd: {
            if (transform_metadata.size() !=
                kR2BwtRltMetadataSize - kR2BlockChecksumSize) {
                throw std::runtime_error("HZ02 BWT+RLT metadata is malformed");
            }
            const std::uint32_t rlt_size = read_u32_le(ByteView(
                transform_metadata.data() + kR2BwtPrimaryIndexSize, 4U));
            if (rlt_size == 0 || rlt_size >= header.uncompressed_size ||
                rlt_size > kR2MaximumBlockSize) {
                throw std::runtime_error("HZ02 BWT+RLT length is invalid");
            }
            decoded = ZstdBackend().decode(payload, rlt_size);
            decoded = KanziRltTransform().inverse(
                ByteView(decoded), header.uncompressed_size);
            break;
        }
        case BlockMode::X86BcjZstd:
            decoded = ZstdBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::ShuffleZstd:
            decoded = ZstdBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::BitshuffleZstd:
            decoded = ZstdBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::DeltaZstd:
            decoded = ZstdBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::DeltaOfDeltaZstd:
            decoded = ZstdBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::DeltaBinaryPackedZstd: {
            if (transform_metadata.size() != 5U ||
                (transform_metadata[0] != 4U && transform_metadata[0] != 8U)) {
                throw std::runtime_error(
                    "HZ02 delta-binary-packed metadata is malformed");
            }
            const std::uint32_t transformed_size = read_u32_le(ByteView(
                transform_metadata.data() + 1U, 4U));
            const auto maximum = DeltaBinaryPackedTransform::maximum_transformed_size(
                header.uncompressed_size, transform_metadata[0]);
            if (transformed_size == 0U || transformed_size > maximum) {
                throw std::runtime_error(
                    "HZ02 delta-binary-packed transform length is invalid");
            }
            decoded = ZstdBackend().decode(payload, transformed_size);
            break;
        }
        case BlockMode::FastPfor:
            decoded = FastPforBackend().decode(payload, transform_metadata,
                                                header.uncompressed_size);
            break;
        case BlockMode::Rans:
            decoded = RansBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::Bcj2Zstd: {
            if (transform_metadata.size() != 16U) {
                throw std::runtime_error("HZ02 BCJ2 metadata is malformed");
            }
            std::size_t transformed_size = 0;
            for (std::size_t offset = 0; offset < transform_metadata.size(); offset += 4U) {
                transformed_size += static_cast<std::size_t>(
                    static_cast<std::uint32_t>(transform_metadata[offset]) |
                    (static_cast<std::uint32_t>(transform_metadata[offset + 1U]) << 8U) |
                    (static_cast<std::uint32_t>(transform_metadata[offset + 2U]) << 16U) |
                    (static_cast<std::uint32_t>(transform_metadata[offset + 3U]) << 24U));
            }
            decoded = Bcj2Transform().inverse(
                ByteView(ZstdBackend().decode(payload, transformed_size)),
                transform_metadata, header.uncompressed_size);
            break;
        }
        case BlockMode::RecordTransposeZstd:
            decoded = ZstdBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::JpegLs:
            decoded = JpegLsTransform().inverse(payload, transform_metadata,
                                                 header.uncompressed_size);
            break;
        case BlockMode::FlacResidual:
            decoded = FlacResidualTransform().inverse(payload, transform_metadata,
                                                       header.uncompressed_size);
            break;
        case BlockMode::BrotliText:
            decoded = BrotliTextTransform().inverse(payload,
                                                     header.uncompressed_size);
            break;
        case BlockMode::CmixWordDictionaryZstd: {
            if (transform_metadata.size() != 4U) {
                throw std::runtime_error(
                    "HZ02 cmix word dictionary metadata is malformed");
            }
            const std::uint32_t transformed_size =
                read_u32_le(transform_metadata);
            if (transformed_size == 0 ||
                transformed_size > CmixWordDictionaryTransform::
                    maximum_transformed_size(header.uncompressed_size)) {
                throw std::runtime_error(
                    "HZ02 cmix word dictionary transform length is invalid");
            }
            decoded = CmixWordDictionaryTransform().inverse(
                ByteView(ZstdBackend().decode(payload, transformed_size)),
                header.uncompressed_size);
            break;
        }
    }
    if (header.transform == TransformKind::BwtMtf) {
        decoded = KanziMtfTransform().inverse(ByteView(decoded), ByteView{});
    }
    if (header.transform == TransformKind::Bwt ||
        header.transform == TransformKind::BwtMtf) {
        decoded = BwtTransform().inverse(ByteView(decoded), transform_metadata);
    }
    if (header.transform == TransformKind::BwtRlt) {
        decoded = BwtTransform().inverse(
            ByteView(decoded),
            ByteView(transform_metadata.data(), kR2BwtPrimaryIndexSize));
    }
    if (header.transform == TransformKind::X86Bcj) {
        decoded = XzX86BcjTransform().inverse(ByteView(decoded), ByteView{});
    }
    if (header.transform == TransformKind::Shuffle) {
        if (transform_metadata.size() != 1) throw std::runtime_error("HZ02 shuffle metadata is malformed");
        decoded = BloscShuffleTransform().inverse(ByteView(decoded), transform_metadata[0]);
    }
    if (header.transform == TransformKind::Bitshuffle) {
        if (transform_metadata.size() != 1) throw std::runtime_error("HZ02 bitshuffle metadata is malformed");
        decoded = BloscBitshuffleTransform().inverse(ByteView(decoded), transform_metadata[0]);
    }
    if (header.transform == TransformKind::Delta) {
        if (transform_metadata.size() != 1) throw std::runtime_error("HZ02 delta metadata is malformed");
        decoded = BloscDeltaTransform().inverse(ByteView(decoded), transform_metadata[0]);
    }
    if (header.transform == TransformKind::DeltaOfDelta) {
        if (transform_metadata.size() != 1U ||
            (transform_metadata[0] != 4U && transform_metadata[0] != 8U)) {
            throw std::runtime_error(
                "HZ02 delta-of-delta metadata is malformed");
        }
        decoded = DeltaOfDeltaTransform().inverse(
            ByteView(decoded), transform_metadata[0]);
    }
    if (header.transform == TransformKind::DeltaBinaryPacked) {
        if (transform_metadata.size() != 5U ||
            (transform_metadata[0] != 4U && transform_metadata[0] != 8U)) {
            throw std::runtime_error(
                "HZ02 delta-binary-packed metadata is malformed");
        }
        const std::uint32_t transformed_size = read_u32_le(ByteView(
            transform_metadata.data() + 1U, 4U));
        if (decoded.size() != transformed_size) {
            throw std::runtime_error(
                "HZ02 delta-binary-packed transformed size mismatch");
        }
        decoded = DeltaBinaryPackedTransform().inverse(
            ByteView(decoded), transform_metadata[0], header.uncompressed_size);
    }
    if (header.transform == TransformKind::RecordTranspose) {
        if (transform_metadata.size() != 1U ||
            (transform_metadata[0] != 16U && transform_metadata[0] != 32U)) {
            throw std::runtime_error("HZ02 record transpose metadata is malformed");
        }
        decoded = RecordTransposeTransform().inverse(
            ByteView(decoded), transform_metadata[0]);
    }
    return decoded;
}

std::uint32_t crc32(const ByteView input) noexcept {
    std::uint32_t checksum = 0xFFFFFFFFU;
    for (std::size_t index = 0; index < input.size(); ++index) {
        checksum ^= input[index];
        for (unsigned bit = 0; bit < 8U; ++bit) {
            const std::uint32_t mask =
                0U - static_cast<std::uint32_t>(checksum & 1U);
            checksum = (checksum >> 1U) ^ (0xEDB88320U & mask);
        }
    }
    return ~checksum;
}

bool uses_auto_accounting(const CandidatePolicy policy) noexcept {
    return policy == CandidatePolicy::Auto ||
           policy == CandidatePolicy::AutoK2 ||
           policy == CandidatePolicy::AutoK4 ||
           policy == CandidatePolicy::AutoK8;
}

void write_and_account_block(std::ostream& archive,
                             CompressionStats& stats,
                             BlockDecision decision,
                             const CandidatePolicy policy,
                             const std::uint32_t uncompressed_size,
                             const std::uint32_t checksum) {
    if (decision.payload.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("HZ02 block payload is too large");
    }
    if (!uses_auto_accounting(policy)) {
        const std::size_t serialized_block_bytes =
            kR2BlockHeaderSize + kR2BlockChecksumSize +
            decision.transform_metadata.size() + decision.payload.size();
        decision.selected_candidate_bytes = serialized_block_bytes;
        decision.oracle_candidate_bytes = serialized_block_bytes;
        decision.oracle_gap_bytes = 0;
    }

    BlockHeader block_header{};
    block_header.mode = decision.mode;
    block_header.transform = decision.transform;
    block_header.entropy = decision.entropy;
    block_header.uncompressed_size = uncompressed_size;
    block_header.payload_size = static_cast<std::uint32_t>(decision.payload.size());
    block_header.metadata_size = static_cast<std::uint32_t>(
        kR2BlockChecksumSize + decision.transform_metadata.size());
    write_block_header(archive, block_header);
    write_block_crc32(archive, checksum);
    if (!decision.transform_metadata.empty()) {
        archive.write(reinterpret_cast<const char*>(
                          decision.transform_metadata.data()),
                      static_cast<std::streamsize>(
                          decision.transform_metadata.size()));
        if (!archive) {
            throw std::runtime_error("Failed to write HZ02 transform metadata");
        }
    }
    archive.write(reinterpret_cast<const char*>(decision.payload.data()),
                  static_cast<std::streamsize>(decision.payload.size()));
    if (!archive) {
        throw std::runtime_error("Failed to write HZ02 block payload");
    }

    ++stats.blocks_by_mode[static_cast<std::size_t>(decision.mode)];
    for (std::size_t mode = 0; mode < stats.candidate_blocks_by_mode.size();
         ++mode) {
        stats.candidate_blocks_by_mode[mode] +=
            decision.candidate_blocks_by_mode[mode];
    }
    stats.payload_bytes += decision.payload.size();
    stats.candidates_evaluated += decision.candidates_evaluated;
    stats.selected_candidate_bytes += decision.selected_candidate_bytes;
    stats.oracle_candidate_bytes += decision.oracle_candidate_bytes;
    stats.oracle_gap_bytes += decision.oracle_gap_bytes;
}

std::size_t maximum_payload_for(const BlockHeader& header) {
    if (header.mode == BlockMode::Stored) {
        return header.uncompressed_size;
    }
    if (header.mode == BlockMode::Zstd ||
        header.mode == BlockMode::FastExtension ||
        header.mode == BlockMode::BwtZstd ||
        header.mode == BlockMode::BwtMtfZstd ||
        header.mode == BlockMode::BwtRltZstd || header.mode == BlockMode::X86BcjZstd ||
        header.mode == BlockMode::ShuffleZstd || header.mode == BlockMode::BitshuffleZstd ||
        header.mode == BlockMode::DeltaZstd ||
        header.mode == BlockMode::DeltaOfDeltaZstd ||
        header.mode == BlockMode::RecordTransposeZstd) {
        return ZstdBackend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::DeltaBinaryPackedZstd) {
        const std::size_t transformed_size =
            DeltaBinaryPackedTransform::maximum_transformed_size(
                header.uncompressed_size, 4U);
        return ZstdBackend::maximum_payload_size(transformed_size);
    }
    if (header.mode == BlockMode::Fse) {
        return FseBackend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::Lzma) {
        return LzmaBackend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::Lz4) {
        return Lz4Backend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::KanziAns) {
        return KanziAnsBackend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::LmicArithmetic) {
        return LmicArithmeticBackend::maximum_payload_size(
            header.uncompressed_size);
    }
    if (header.mode == BlockMode::Ppmd7) {
        return Ppmd7Backend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::Ppmd8) {
        return Ppmd8Backend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::Zpaq) {
        return ZpaqBackend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::Ctw) {
        return CtwBackend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::Paq8pxApmPredictive) {
        return Paq8pxApmBackend::maximum_payload_size(
            header.uncompressed_size);
    }
    if (header.mode == BlockMode::Paq8pxRecordModel) {
        return Paq8pxRecordModelBackend::maximum_payload_size(
            header.uncompressed_size);
    }
    if (header.mode == BlockMode::Paq8pxLinearPrediction) {
        return Paq8pxLinearPredictionBackend::maximum_payload_size(
            header.uncompressed_size);
    }
    if (header.mode == BlockMode::Paq8pxSimilarity) {
        return Paq8pxSimilarityBackend::maximum_payload_size(
            header.uncompressed_size);
    }
    if (header.mode == BlockMode::Paq8pxSimilaritySse) {
        return Paq8pxSimilaritySseBackend::maximum_payload_size(
            header.uncompressed_size);
    }
    if (header.mode == BlockMode::Paq8pxGenericSse) {
        return Paq8pxGenericSseBackend::maximum_payload_size(
            header.uncompressed_size);
    }
    if (header.mode == BlockMode::Paq8pxDetectedSse) {
        return Paq8pxDetectedSseBackend::maximum_payload_size(
            header.uncompressed_size);
    }
    if (header.mode == BlockMode::Wavpack) {
        return WavpackBackend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::LstmCompress) {
        return LstmCompressBackend::maximum_payload_size(
            header.uncompressed_size);
    }
    if (header.mode == BlockMode::DonorMatchPredictive ||
        header.mode == BlockMode::NeuralLstm ||
        header.mode == BlockMode::SharedNeuralLstm ||
        header.mode == BlockMode::BgptSharedPrior ||
        header.mode == BlockMode::JaxCompressPortable) {
        return DonorMatchPredictiveBackend::maximum_payload_size(
            header.uncompressed_size);
    }
    if (header.mode == BlockMode::FastPfor) {
        if (header.uncompressed_size >
            (std::numeric_limits<std::size_t>::max() - 4096U) / 8U) {
            throw std::runtime_error("HZ02 FastPFOR payload bound overflow");
        }
        return static_cast<std::size_t>(header.uncompressed_size) * 8U + 4096U;
    }
    if (header.mode == BlockMode::Rans) {
        return RansBackend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::Bcj2Zstd) {
        return ZstdBackend::maximum_payload_size(
            static_cast<std::size_t>(header.uncompressed_size) * 2U + 32U);
    }
    if (header.mode == BlockMode::JpegLs) {
        return JpegLsTransform::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::FlacResidual) {
        return FlacResidualTransform::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::BrotliText) {
        return BrotliTextTransform::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::CmixWordDictionaryZstd) {
        return ZstdBackend::maximum_payload_size(
            CmixWordDictionaryTransform::maximum_transformed_size(
                header.uncompressed_size));
    }
    const std::size_t size = header.uncompressed_size;
    if (size > (std::numeric_limits<std::size_t>::max() - 64U) / 4U) {
        throw std::runtime_error("HZ02 predictive payload bound overflow");
    }
    return size * 4U + 64U;
}

}  // namespace

CompressionStats compress_file(const std::filesystem::path& input,
                               const std::filesystem::path& output,
                               const CompressionOptions& options) {
    if (options.block_size == 0 ||
        options.block_size > kR2MaximumBlockSize) {
        throw std::invalid_argument("Invalid HZ02 block size");
    }
    if (options.thread_count == 0U) {
        throw std::invalid_argument("Invalid HZ02 thread count");
    }
    if (options.thread_count != 1U &&
        options.policy != CandidatePolicy::Fast) {
        throw std::invalid_argument(
            "HZ02 worker threads are currently supported only for Fast");
    }

    const std::filesystem::path temporary = temporary_path_for(output);
    validate_paths(input, output, temporary);
    const std::uintmax_t reported_size = std::filesystem::file_size(input);
    if (reported_size > std::numeric_limits<std::uint64_t>::max()) {
        throw std::runtime_error("Input is too large for HZ02");
    }

    CompressionStats stats{};
    stats.full_oracle_evaluated = options.policy == CandidatePolicy::Auto;
    stats.worker_count = options.thread_count;
    stats.input_bytes = static_cast<std::uint64_t>(reported_size);
    stats.selected_candidate_bytes = kR2ArchiveHeaderSize;
    stats.oracle_candidate_bytes = kR2ArchiveHeaderSize;
    const std::uint32_t block_count =
        block_count_for(stats.input_bytes, options.block_size);

    try {
        std::ifstream source(input, std::ios::binary);
        std::ofstream archive(temporary,
                              std::ios::binary | std::ios::trunc);
        if (!source || !archive) {
            throw std::runtime_error("Failed to open HZ02 compression paths");
        }

        ArchiveHeader archive_header{};
        archive_header.original_size = stats.input_bytes;
        archive_header.block_size = options.block_size;
        archive_header.block_count = block_count;
        archive_header.model_seed = options.model_seed;
        write_archive_header(archive, archive_header);

        BlockPlannerOptions planner_options{};
        planner_options.policy = options.policy;
        planner_options.zstd_level = options.zstd_level;
        planner_options.lzma_level = options.lzma_level;
        planner_options.lzma_dictionary_size =
            options.lzma_dictionary_size;
        planner_options.model_seed = options.model_seed;
        std::uint64_t remaining = stats.input_bytes;
        if (options.policy == CandidatePolicy::Fast) {
            FastBlockExecutor executor(planner_options, options.thread_count);
            stats.fast_block_queue_plus_service_ns.reserve(block_count);
            stats.fast_block_service_ns.reserve(block_count);
            const std::uint64_t in_flight_limit =
                static_cast<std::uint64_t>(options.thread_count) * 2U;
            std::uint32_t submitted = 0;
            std::uint32_t written = 0;
            while (written < block_count) {
                while (submitted < block_count &&
                       static_cast<std::uint64_t>(submitted - written) <
                           in_flight_limit) {
                    const std::size_t block_bytes = static_cast<std::size_t>(
                        std::min<std::uint64_t>(remaining, options.block_size));
                    FastBlockTask task;
                    task.index = submitted;
                    task.raw.resize(block_bytes);
                    read_exact(source, task.raw,
                               "Input shrank during HZ02 compression");
                    task.checksum = crc32(ByteView(task.raw));
                    executor.submit(std::move(task));
                    remaining -= block_bytes;
                    ++submitted;
                }
                FastBlockResult result = executor.take(written);
                const std::uint32_t expected_size = static_cast<std::uint32_t>(
                    std::min<std::uint64_t>(
                        options.block_size,
                        stats.input_bytes -
                            static_cast<std::uint64_t>(written) *
                                options.block_size));
                if (result.index != written ||
                    result.uncompressed_size != expected_size) {
                    throw std::runtime_error(
                        "Fast executor returned an invalid block result");
                }
                stats.fast_block_queue_plus_service_ns.push_back(
                    result.queue_plus_service_ns);
                stats.fast_block_service_ns.push_back(result.service_ns);
                write_and_account_block(archive, stats, std::move(result.decision),
                                        options.policy, result.uncompressed_size,
                                        result.checksum);
                ++written;
            }
        } else {
            BlockPlanner planner(planner_options);
            for (std::uint32_t block = 0; block < block_count; ++block) {
                const std::size_t block_bytes = static_cast<std::size_t>(
                    std::min<std::uint64_t>(remaining, options.block_size));
                std::vector<std::uint8_t> raw(block_bytes);
                read_exact(source, raw, "Input shrank during HZ02 compression");
                BlockDecision decision = planner.plan(ByteView(raw));
                write_and_account_block(archive, stats, std::move(decision),
                                        options.policy,
                                        static_cast<std::uint32_t>(block_bytes),
                                        crc32(ByteView(raw)));
                remaining -= block_bytes;
            }
        }

        char extra = 0;
        if (source.get(extra)) {
            throw std::runtime_error("Input grew during HZ02 compression");
        }
        if (source.bad()) {
            throw std::runtime_error("Failed while reading HZ02 input");
        }

        archive.flush();
        if (!archive) {
            throw std::runtime_error("Failed to flush HZ02 archive");
        }
        archive.close();
        source.close();
        std::filesystem::rename(temporary, output);
        stats.archive_bytes = std::filesystem::file_size(output);
        return stats;
    } catch (...) {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        throw;
    }
}

void decompress_file(const std::filesystem::path& input,
                     const std::filesystem::path& output) {
    const std::filesystem::path temporary = temporary_path_for(output);
    validate_paths(input, output, temporary);

    try {
        std::ifstream archive(input, std::ios::binary);
        std::ofstream restored(temporary,
                               std::ios::binary | std::ios::trunc);
        if (!archive || !restored) {
            throw std::runtime_error("Failed to open HZ02 decompression paths");
        }

        const ArchiveHeader archive_header = read_archive_header(archive);
        std::uint64_t remaining = archive_header.original_size;
        for (std::uint32_t block = 0; block < archive_header.block_count;
             ++block) {
            const BlockHeader block_header = read_block_header(archive);
            const std::uint32_t expected_size = static_cast<std::uint32_t>(
                std::min<std::uint64_t>(remaining,
                                        archive_header.block_size));
            if (block_header.uncompressed_size != expected_size ||
                block_header.payload_size > maximum_payload_for(block_header)) {
                throw std::runtime_error("Invalid HZ02 block size contract");
            }

            const std::uint32_t expected_checksum = read_block_crc32(archive);
            std::vector<std::uint8_t> transform_metadata(
                block_header.metadata_size - kR2BlockChecksumSize);
            read_exact(archive, transform_metadata,
                       "Truncated HZ02 transform metadata");
            std::vector<std::uint8_t> payload(block_header.payload_size);
            read_exact(archive, payload, "Truncated HZ02 block payload");
            const std::vector<std::uint8_t> decoded = decode_block(
                block_header, ByteView(payload), ByteView(transform_metadata),
                archive_header.model_seed);
            if (decoded.size() != block_header.uncompressed_size) {
                throw std::runtime_error("HZ02 transform output size mismatch");
            }
            if (crc32(ByteView(decoded)) != expected_checksum) {
                throw std::runtime_error("HZ02 block checksum mismatch");
            }
            restored.write(reinterpret_cast<const char*>(decoded.data()),
                           static_cast<std::streamsize>(decoded.size()));
            if (!restored) {
                throw std::runtime_error("Failed to write HZ02 decoded block");
            }
            remaining -= decoded.size();
        }

        if (remaining != 0) {
            throw std::runtime_error("HZ02 archive ended before original size");
        }
        char trailing = 0;
        if (archive.get(trailing)) {
            throw std::runtime_error("HZ02 archive has trailing data");
        }
        if (archive.bad()) {
            throw std::runtime_error("Failed while reading HZ02 archive");
        }

        restored.flush();
        if (!restored) {
            throw std::runtime_error("Failed to flush HZ02 output");
        }
        restored.close();
        archive.close();
        std::filesystem::rename(temporary, output);
    } catch (...) {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        throw;
    }
}

bool is_r2_archive(const std::filesystem::path& input) {
    std::ifstream archive(input, std::ios::binary);
    if (!archive) {
        return false;
    }
    std::array<char, 4> magic{};
    archive.read(magic.data(), static_cast<std::streamsize>(magic.size()));
    return archive.gcount() == static_cast<std::streamsize>(magic.size()) &&
           std::memcmp(magic.data(), kMagic.data(), magic.size()) == 0;
}

}  // namespace hz::r2
