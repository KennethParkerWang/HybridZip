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
#include "r2/entropy/predictive_v1_backend.h"
#include "r2/entropy/fse_backend.h"
#include "r2/entropy/lzma_backend.h"
#include "r2/entropy/stored_backend.h"
#include "r2/entropy/zstd_backend.h"
#include "r2/representation/bwt_transform.h"
#include "r2/representation/kanzi_mtf_transform.h"

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
        case BlockMode::Zstd:
            decoded = ZstdBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::Fse:
            decoded = FseBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::Lzma:
            decoded = LzmaBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::DonorMatchPredictive:
            decoded = DonorMatchPredictiveBackend(model_seed).decode(
                payload, header.uncompressed_size);
            break;
        case BlockMode::BwtZstd:
            decoded = ZstdBackend().decode(payload, header.uncompressed_size);
            break;
        case BlockMode::BwtMtfZstd:
            decoded = ZstdBackend().decode(payload, header.uncompressed_size);
            break;
    }
    if (header.transform == TransformKind::BwtMtf) {
        decoded = KanziMtfTransform().inverse(ByteView(decoded), ByteView{});
    }
    if (header.transform == TransformKind::Bwt ||
        header.transform == TransformKind::BwtMtf) {
        decoded = BwtTransform().inverse(ByteView(decoded), transform_metadata);
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

std::size_t maximum_payload_for(const BlockHeader& header) {
    if (header.mode == BlockMode::Stored) {
        return header.uncompressed_size;
    }
    if (header.mode == BlockMode::Zstd || header.mode == BlockMode::BwtZstd ||
        header.mode == BlockMode::BwtMtfZstd) {
        return ZstdBackend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::Fse) {
        return FseBackend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::Lzma) {
        return LzmaBackend::maximum_payload_size(header.uncompressed_size);
    }
    if (header.mode == BlockMode::DonorMatchPredictive) {
        return DonorMatchPredictiveBackend::maximum_payload_size(
            header.uncompressed_size);
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

    const std::filesystem::path temporary = temporary_path_for(output);
    validate_paths(input, output, temporary);
    const std::uintmax_t reported_size = std::filesystem::file_size(input);
    if (reported_size > std::numeric_limits<std::uint64_t>::max()) {
        throw std::runtime_error("Input is too large for HZ02");
    }

    CompressionStats stats{};
    stats.input_bytes = static_cast<std::uint64_t>(reported_size);
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
        const BlockPlanner planner(planner_options);

        std::uint64_t remaining = stats.input_bytes;
        for (std::uint32_t block = 0; block < block_count; ++block) {
            const std::size_t block_bytes = static_cast<std::size_t>(
                std::min<std::uint64_t>(remaining, options.block_size));
            std::vector<std::uint8_t> raw(block_bytes);
            read_exact(source, raw, "Input shrank during HZ02 compression");

            BlockDecision decision = planner.plan(ByteView(raw));
            if (decision.payload.size() >
                std::numeric_limits<std::uint32_t>::max()) {
                throw std::runtime_error("HZ02 block payload is too large");
            }

            BlockHeader block_header{};
            block_header.mode = decision.mode;
            block_header.transform = decision.transform;
            block_header.entropy = decision.entropy;
            block_header.uncompressed_size =
                static_cast<std::uint32_t>(block_bytes);
            block_header.payload_size =
                static_cast<std::uint32_t>(decision.payload.size());
            block_header.metadata_size = static_cast<std::uint32_t>(
                kR2BlockChecksumSize + decision.transform_metadata.size());
            write_block_header(archive, block_header);
            write_block_crc32(archive, crc32(ByteView(raw)));
            if (!decision.transform_metadata.empty()) {
                archive.write(reinterpret_cast<const char*>(
                                  decision.transform_metadata.data()),
                              static_cast<std::streamsize>(
                                  decision.transform_metadata.size()));
                if (!archive) {
                    throw std::runtime_error("Failed to write HZ02 transform metadata");
                }
            }
            archive.write(
                reinterpret_cast<const char*>(decision.payload.data()),
                static_cast<std::streamsize>(decision.payload.size()));
            if (!archive) {
                throw std::runtime_error("Failed to write HZ02 block payload");
            }

            ++stats.blocks_by_mode[static_cast<std::size_t>(decision.mode)];
            stats.payload_bytes += decision.payload.size();
            remaining -= block_bytes;
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
