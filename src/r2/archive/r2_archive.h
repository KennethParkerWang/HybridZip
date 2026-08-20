#pragma once

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
constexpr std::uint32_t kR2ProfileId = 2;
constexpr std::uint32_t kR2DefaultBlockSize = 64U * 1024U;
constexpr std::uint32_t kR2MaximumBlockSize = 16U * 1024U * 1024U;

enum class BlockMode : std::uint8_t {
    Stored = 0,
    PredictiveV1 = 1,
    Zstd = 2,
    Fse = 3,
    Lzma = 4,
    DonorMatchPredictive = 5
};

enum class TransformKind : std::uint8_t {
    Raw = 0
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
