#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "core/types.h"
#include "r2/entropy/entropy_backend.h"

namespace hz::r2 {

constexpr std::uint16_t kR2ArchiveVersion = 2;
constexpr std::uint16_t kR2ArchiveHeaderSize = 40;
constexpr std::uint16_t kR2BlockHeaderSize = 16;
constexpr std::uint8_t kR2BlockFlagCrc32 = 1U << 0U;
constexpr std::uint32_t kR2BlockChecksumSize = 4;
constexpr std::uint32_t kR2BwtPrimaryIndexSize = 4;
constexpr std::uint32_t kR2BwtMetadataSize =
    kR2BlockChecksumSize + kR2BwtPrimaryIndexSize;
constexpr std::uint32_t kR2BwtRltMetadataSize = kR2BwtMetadataSize + 4;
constexpr std::uint32_t kR2CmixDictionaryMetadataSize =
    kR2BlockChecksumSize + 4;
constexpr std::uint32_t kR2SharedNeuralMetadataSize =
    kR2BlockChecksumSize + 4;
constexpr std::uint32_t kR2BgptSharedPriorIdentitySize = 4U + 32U;
constexpr std::uint32_t kR2BgptSharedPriorMetadataSize =
    kR2BlockChecksumSize + kR2BgptSharedPriorIdentitySize;
constexpr std::uint32_t kR2JaxCompressPortableIdentitySize = 4U + 20U + 32U;
constexpr std::uint32_t kR2JaxCompressPortableMetadataSize =
    kR2BlockChecksumSize + kR2JaxCompressPortableIdentitySize;
constexpr std::uint32_t kR2LmicArithmeticIdentitySize = 4U + 32U;
constexpr std::uint32_t kR2LmicArithmeticMetadataSize =
    kR2BlockChecksumSize + kR2LmicArithmeticIdentitySize;
constexpr std::uint32_t kR2ShuffleMetadataSize = kR2BlockChecksumSize + 1;
constexpr std::uint32_t kR2DeltaOfDeltaMetadataSize =
    kR2BlockChecksumSize + 1;
constexpr std::uint32_t kR2DeltaBinaryPackedMetadataSize =
    kR2BlockChecksumSize + 5;
constexpr std::uint32_t kR2FastPforMetadataMinimumSize = kR2BlockChecksumSize + 2;
constexpr std::uint32_t kR2Bcj2MetadataSize = kR2BlockChecksumSize + 16;
constexpr std::uint32_t kR2JpegLsMetadataSize = kR2BlockChecksumSize + 8;
constexpr std::uint32_t kR2FlacResidualMetadataMinimumSize =
    kR2BlockChecksumSize + 16;
constexpr std::uint32_t kR2FlacResidualMetadataMaximumSize =
    kR2BlockChecksumSize + 152;
constexpr std::uint32_t kR2FastExtensionMetadataMinimumSize =
    kR2BlockChecksumSize + 4;
constexpr std::uint32_t kR2FastExtensionMetadataMaximumSize =
    kR2BlockChecksumSize + 68;
constexpr std::uint32_t kR2ProfileId = 2;
constexpr std::uint32_t kR2DefaultBlockSize = 64U * 1024U;
constexpr std::uint32_t kR2MaximumBlockSize = 16U * 1024U * 1024U;

enum class BlockMode : std::uint8_t {
    Stored = 0,
    PredictiveV1 = 1,
    Zstd = 2,
    Fse = 3,
    Lzma = 4,
    DonorMatchPredictive = 5,
    BwtZstd = 6,
    BwtMtfZstd = 7,
    BwtRltZstd = 8,
    X86BcjZstd = 9,
    ShuffleZstd = 10,
    BitshuffleZstd = 11,
    DeltaZstd = 12,
    FastPfor = 13,
    Rans = 14,
    Bcj2Zstd = 15,
    RecordTransposeZstd = 16,
    JpegLs = 17,
    FlacResidual = 18,
    BrotliText = 19,
    CmixWordDictionaryZstd = 20,
    NeuralLstm = 21,
    SharedNeuralLstm = 22,
    LstmCompress = 23,
    DeltaOfDeltaZstd = 24,
    BgptSharedPrior = 25,
    JaxCompressPortable = 26,
    Ppmd7 = 27,
    Ppmd8 = 28,
    Zpaq = 29,
    Ctw = 30,
    Paq8pxApmPredictive = 31,
    Paq8pxRecordModel = 32,
    Paq8pxLinearPrediction = 33,
    Paq8pxSimilarity = 34,
    Paq8pxSimilaritySse = 35,
    Paq8pxGenericSse = 36,
    Paq8pxDetectedSse = 37,
    Wavpack = 38,
    Lz4 = 39,
    KanziAns = 40,
    LmicArithmetic = 41,
    DeltaBinaryPackedZstd = 42,
    FastExtension = 43
};

constexpr std::size_t kR2BlockModeCount =
    static_cast<std::size_t>(BlockMode::FastExtension) + 1U;

enum class TransformKind : std::uint8_t {
    Raw = 0,
    Bwt = 1,
    BwtMtf = 2,
    BwtRlt = 3,
    X86Bcj = 4,
    Shuffle = 5,
    Bitshuffle = 6,
    Delta = 7,
    FastPfor = 8,
    Bcj2 = 9,
    RecordTranspose = 10,
    JpegLs = 11,
    FlacResidual = 12,
    BrotliText = 13,
    CmixWordDictionary = 14,
    NeuralLstm = 15,
    NeuralShared = 16,
    DeltaOfDelta = 17,
    NeuralSharedPrior = 18,
    NeuralOnlinePortable = 19,
    NeuralLmicArithmetic = 20,
    DeltaBinaryPacked = 21
};

struct ArchiveHeader {
    std::uint64_t original_size = 0;
    std::uint32_t block_size = kR2DefaultBlockSize;
    std::uint32_t block_count = 0;
    std::uint32_t flags = 0;
    std::uint32_t profile_id = kR2ProfileId;
    std::uint64_t model_seed = kDefaultModelSeed;
};

struct BlockHeader {
    BlockMode mode = BlockMode::Stored;
    TransformKind transform = TransformKind::Raw;
    EntropyKind entropy = EntropyKind::Stored;
    std::uint8_t flags = kR2BlockFlagCrc32;
    std::uint32_t uncompressed_size = 0;
    std::uint32_t payload_size = 0;
    std::uint32_t metadata_size = kR2BlockChecksumSize;
};

void write_archive_header(std::ostream& output, const ArchiveHeader& header);
ArchiveHeader read_archive_header(std::istream& input);

void write_block_header(std::ostream& output, const BlockHeader& header);
BlockHeader read_block_header(std::istream& input);

void write_block_crc32(std::ostream& output, std::uint32_t checksum);
std::uint32_t read_block_crc32(std::istream& input);

}  // namespace hz::r2
