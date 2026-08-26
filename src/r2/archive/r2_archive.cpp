#include "r2/archive/r2_archive.h"

#include <array>
#include <cstddef>
#include <cstring>
#include <istream>
#include <ostream>
#include <stdexcept>

namespace hz::r2 {
namespace {

constexpr std::array<char, 4> kMagic{'H', 'Z', '0', '2'};

void require_output(const std::ostream& output) {
    if (!output) {
        throw std::runtime_error("Failed to write HZ02 metadata");
    }
}

void write_u8(std::ostream& output, const std::uint8_t value) {
    output.put(static_cast<char>(value));
    require_output(output);
}

void write_u16_le(std::ostream& output, const std::uint16_t value) {
    write_u8(output, static_cast<std::uint8_t>(value));
    write_u8(output, static_cast<std::uint8_t>(value >> 8U));
}

void write_u32_le(std::ostream& output, const std::uint32_t value) {
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        write_u8(output, static_cast<std::uint8_t>(value >> shift));
    }
}

void write_u64_le(std::ostream& output, const std::uint64_t value) {
    for (unsigned shift = 0; shift < 64U; shift += 8U) {
        write_u8(output, static_cast<std::uint8_t>(value >> shift));
    }
}

std::uint8_t read_u8(std::istream& input) {
    const int value = input.get();
    if (value == std::char_traits<char>::eof()) {
        throw std::runtime_error("Truncated HZ02 metadata");
    }
    return static_cast<std::uint8_t>(value);
}

std::uint16_t read_u16_le(std::istream& input) {
    std::uint16_t value = 0;
    for (unsigned shift = 0; shift < 16U; shift += 8U) {
        value |= static_cast<std::uint16_t>(read_u8(input)) << shift;
    }
    return value;
}

std::uint32_t read_u32_le(std::istream& input) {
    std::uint32_t value = 0;
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        value |= static_cast<std::uint32_t>(read_u8(input)) << shift;
    }
    return value;
}

std::uint64_t read_u64_le(std::istream& input) {
    std::uint64_t value = 0;
    for (unsigned shift = 0; shift < 64U; shift += 8U) {
        value |= static_cast<std::uint64_t>(read_u8(input)) << shift;
    }
    return value;
}

void validate_archive_header(const ArchiveHeader& header) {
    if (header.block_size == 0 ||
        header.block_size > kR2MaximumBlockSize || header.flags != 0 ||
        header.profile_id != kR2ProfileId) {
        throw std::runtime_error("Unsupported HZ02 archive profile");
    }
    const std::uint64_t expected_blocks =
        header.original_size / header.block_size +
        (header.original_size % header.block_size != 0 ? 1U : 0U);
    if (expected_blocks != header.block_count) {
        throw std::runtime_error("HZ02 block count does not match file size");
    }
}

void validate_block_header(const BlockHeader& header) {
    const bool valid_transform_metadata =
        (header.transform == TransformKind::Raw &&
         header.metadata_size == kR2BlockChecksumSize) ||
        ((header.transform == TransformKind::Bwt ||
          header.transform == TransformKind::BwtMtf) &&
         header.metadata_size == kR2BwtMetadataSize) ||
        (header.transform == TransformKind::BwtRlt &&
         header.metadata_size == kR2BwtRltMetadataSize) ||
        (header.transform == TransformKind::X86Bcj &&
         header.metadata_size == kR2BlockChecksumSize) ||
        (header.transform == TransformKind::Shuffle &&
         header.metadata_size == kR2ShuffleMetadataSize) ||
        (header.transform == TransformKind::Bitshuffle &&
         header.metadata_size == kR2ShuffleMetadataSize) ||
        (header.transform == TransformKind::Delta &&
         header.metadata_size == kR2ShuffleMetadataSize) ||
        (header.transform == TransformKind::DeltaOfDelta &&
         header.metadata_size == kR2DeltaOfDeltaMetadataSize) ||
        (header.transform == TransformKind::DeltaBinaryPacked &&
         header.metadata_size == kR2DeltaBinaryPackedMetadataSize) ||
        (header.transform == TransformKind::FastPfor &&
         header.metadata_size >= kR2FastPforMetadataMinimumSize &&
         header.metadata_size <= kR2FastPforMetadataMinimumSize + 1023U) ||
        (header.transform == TransformKind::Bcj2 &&
         header.metadata_size == kR2Bcj2MetadataSize) ||
        (header.transform == TransformKind::RecordTranspose &&
         header.metadata_size == kR2ShuffleMetadataSize) ||
        (header.transform == TransformKind::JpegLs &&
         header.metadata_size == kR2JpegLsMetadataSize) ||
        (header.transform == TransformKind::FlacResidual &&
         header.metadata_size >= kR2FlacResidualMetadataMinimumSize &&
         header.metadata_size <= kR2FlacResidualMetadataMaximumSize) ||
        (header.transform == TransformKind::BrotliText &&
         header.metadata_size == kR2BlockChecksumSize) ||
        (header.transform == TransformKind::CmixWordDictionary &&
         header.metadata_size == kR2CmixDictionaryMetadataSize) ||
        (header.transform == TransformKind::NeuralLstm &&
         header.metadata_size == kR2BlockChecksumSize) ||
        (header.transform == TransformKind::NeuralShared &&
         header.metadata_size == kR2SharedNeuralMetadataSize) ||
        (header.transform == TransformKind::NeuralSharedPrior &&
         header.metadata_size == kR2BgptSharedPriorMetadataSize) ||
        (header.transform == TransformKind::NeuralOnlinePortable &&
         header.metadata_size == kR2JaxCompressPortableMetadataSize) ||
        (header.transform == TransformKind::NeuralLmicArithmetic &&
         header.metadata_size == kR2LmicArithmeticMetadataSize);
    if (!valid_transform_metadata || header.flags != kR2BlockFlagCrc32 ||
        header.uncompressed_size == 0 ||
        header.uncompressed_size > kR2MaximumBlockSize ||
        header.payload_size == 0) {
        throw std::runtime_error("Unsupported HZ02 block metadata");
    }

    const bool raw_transform = header.transform == TransformKind::Raw;
    const bool valid_pair =
        (header.mode == BlockMode::Stored &&
         raw_transform &&
         header.entropy == EntropyKind::Stored) ||
        (header.mode == BlockMode::PredictiveV1 &&
         raw_transform &&
         header.entropy == EntropyKind::SymbolArithmetic) ||
        (header.mode == BlockMode::Zstd &&
         raw_transform &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::Fse &&
         raw_transform &&
         header.entropy == EntropyKind::Fse) ||
        (header.mode == BlockMode::Lzma &&
         raw_transform &&
         header.entropy == EntropyKind::Lzma) ||
        (header.mode == BlockMode::Ppmd7 &&
         raw_transform &&
         header.entropy == EntropyKind::Ppmd7) ||
        (header.mode == BlockMode::Ppmd8 &&
         raw_transform &&
         header.entropy == EntropyKind::Ppmd8) ||
        (header.mode == BlockMode::Zpaq &&
         raw_transform &&
         header.entropy == EntropyKind::Zpaq) ||
        (header.mode == BlockMode::Ctw &&
         raw_transform &&
         header.entropy == EntropyKind::Ctw) ||
        (header.mode == BlockMode::Paq8pxApmPredictive &&
         raw_transform &&
         header.entropy == EntropyKind::Paq8pxApm) ||
        (header.mode == BlockMode::Paq8pxRecordModel &&
         raw_transform &&
         header.entropy == EntropyKind::Paq8pxRecordModel) ||
        (header.mode == BlockMode::Paq8pxLinearPrediction &&
         raw_transform &&
         header.entropy == EntropyKind::Paq8pxLinearPrediction) ||
        (header.mode == BlockMode::Paq8pxSimilarity &&
         raw_transform &&
         header.entropy == EntropyKind::Paq8pxSimilarity) ||
        (header.mode == BlockMode::Paq8pxSimilaritySse &&
         raw_transform &&
         header.entropy == EntropyKind::Paq8pxSimilaritySse) ||
        (header.mode == BlockMode::Paq8pxGenericSse &&
         raw_transform &&
         header.entropy == EntropyKind::Paq8pxGenericSse) ||
        (header.mode == BlockMode::Paq8pxDetectedSse &&
          raw_transform &&
          header.entropy == EntropyKind::Paq8pxDetectedSse) ||
        (header.mode == BlockMode::Wavpack &&
         raw_transform &&
         header.entropy == EntropyKind::Wavpack) ||
        (header.mode == BlockMode::Lz4 &&
         raw_transform &&
         header.entropy == EntropyKind::Lz4) ||
        (header.mode == BlockMode::KanziAns &&
         raw_transform &&
         header.entropy == EntropyKind::KanziAns) ||
        (header.mode == BlockMode::DonorMatchPredictive &&
         raw_transform &&
         header.entropy == EntropyKind::SymbolArithmetic) ||
        (header.mode == BlockMode::BwtZstd &&
         header.transform == TransformKind::Bwt &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::BwtMtfZstd &&
         header.transform == TransformKind::BwtMtf &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::BwtRltZstd &&
         header.transform == TransformKind::BwtRlt &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::X86BcjZstd &&
         header.transform == TransformKind::X86Bcj &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::ShuffleZstd &&
         header.transform == TransformKind::Shuffle &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::BitshuffleZstd &&
         header.transform == TransformKind::Bitshuffle &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::DeltaZstd &&
         header.transform == TransformKind::Delta &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::DeltaOfDeltaZstd &&
         header.transform == TransformKind::DeltaOfDelta &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::DeltaBinaryPackedZstd &&
         header.transform == TransformKind::DeltaBinaryPacked &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::FastPfor &&
         header.transform == TransformKind::FastPfor &&
         header.entropy == EntropyKind::FastPfor) ||
        (header.mode == BlockMode::Rans &&
         raw_transform &&
         header.entropy == EntropyKind::Rans) ||
        (header.mode == BlockMode::Bcj2Zstd &&
         header.transform == TransformKind::Bcj2 &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::RecordTransposeZstd &&
         header.transform == TransformKind::RecordTranspose &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::JpegLs &&
         header.transform == TransformKind::JpegLs &&
         header.entropy == EntropyKind::JpegLs) ||
        (header.mode == BlockMode::FlacResidual &&
         header.transform == TransformKind::FlacResidual &&
         header.entropy == EntropyKind::FlacResidual) ||
        (header.mode == BlockMode::BrotliText &&
         header.transform == TransformKind::BrotliText &&
         header.entropy == EntropyKind::BrotliText) ||
        (header.mode == BlockMode::CmixWordDictionaryZstd &&
         header.transform == TransformKind::CmixWordDictionary &&
         header.entropy == EntropyKind::ZstdFse) ||
        (header.mode == BlockMode::NeuralLstm && raw_transform &&
         header.entropy == EntropyKind::SymbolArithmetic) ||
        (header.mode == BlockMode::SharedNeuralLstm &&
         header.transform == TransformKind::NeuralShared &&
         header.entropy == EntropyKind::SymbolArithmetic) ||
        (header.mode == BlockMode::LstmCompress &&
         header.transform == TransformKind::NeuralLstm &&
         header.entropy == EntropyKind::SymbolArithmetic) ||
        (header.mode == BlockMode::BgptSharedPrior &&
         header.transform == TransformKind::NeuralSharedPrior &&
         header.entropy == EntropyKind::SymbolArithmetic) ||
        (header.mode == BlockMode::JaxCompressPortable &&
         header.transform == TransformKind::NeuralOnlinePortable &&
         header.entropy == EntropyKind::SymbolArithmetic) ||
        (header.mode == BlockMode::LmicArithmetic &&
         header.transform == TransformKind::NeuralLmicArithmetic &&
         header.entropy == EntropyKind::LmicArithmetic);
    if (!valid_pair) {
        throw std::runtime_error("HZ02 block mode and entropy backend disagree");
    }
}

}  // namespace

void write_archive_header(std::ostream& output, const ArchiveHeader& header) {
    validate_archive_header(header);
    output.write(kMagic.data(), static_cast<std::streamsize>(kMagic.size()));
    require_output(output);
    write_u16_le(output, kR2ArchiveVersion);
    write_u16_le(output, kR2ArchiveHeaderSize);
    write_u64_le(output, header.original_size);
    write_u32_le(output, header.block_size);
    write_u32_le(output, header.block_count);
    write_u32_le(output, header.flags);
    write_u32_le(output, header.profile_id);
    write_u64_le(output, header.model_seed);
}

ArchiveHeader read_archive_header(std::istream& input) {
    std::array<char, 4> magic{};
    input.read(magic.data(), static_cast<std::streamsize>(magic.size()));
    if (input.gcount() != static_cast<std::streamsize>(magic.size()) ||
        std::memcmp(magic.data(), kMagic.data(), kMagic.size()) != 0) {
        throw std::runtime_error("Invalid HZ02 magic");
    }
    if (read_u16_le(input) != kR2ArchiveVersion ||
        read_u16_le(input) != kR2ArchiveHeaderSize) {
        throw std::runtime_error("Unsupported HZ02 version or header size");
    }

    ArchiveHeader header{};
    header.original_size = read_u64_le(input);
    header.block_size = read_u32_le(input);
    header.block_count = read_u32_le(input);
    header.flags = read_u32_le(input);
    header.profile_id = read_u32_le(input);
    header.model_seed = read_u64_le(input);
    validate_archive_header(header);
    return header;
}

void write_block_header(std::ostream& output, const BlockHeader& header) {
    validate_block_header(header);
    write_u8(output, static_cast<std::uint8_t>(header.mode));
    write_u8(output, static_cast<std::uint8_t>(header.transform));
    write_u8(output, static_cast<std::uint8_t>(header.entropy));
    write_u8(output, header.flags);
    write_u32_le(output, header.uncompressed_size);
    write_u32_le(output, header.payload_size);
    write_u32_le(output, header.metadata_size);
}

BlockHeader read_block_header(std::istream& input) {
    BlockHeader header{};
    header.mode = static_cast<BlockMode>(read_u8(input));
    header.transform = static_cast<TransformKind>(read_u8(input));
    header.entropy = static_cast<EntropyKind>(read_u8(input));
    header.flags = read_u8(input);
    header.uncompressed_size = read_u32_le(input);
    header.payload_size = read_u32_le(input);
    header.metadata_size = read_u32_le(input);
    validate_block_header(header);
    return header;
}

void write_block_crc32(std::ostream& output, const std::uint32_t checksum) {
    write_u32_le(output, checksum);
}

std::uint32_t read_block_crc32(std::istream& input) {
    return read_u32_le(input);
}

}  // namespace hz::r2
