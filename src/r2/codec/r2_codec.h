#pragma once

#include <array>
#include <cstdint>
#include <filesystem>

#include "r2/block/block_planner.h"

namespace hz::r2 {

struct CompressionOptions {
    CandidatePolicy policy = CandidatePolicy::Auto;
    std::uint32_t block_size = kR2DefaultBlockSize;
    int zstd_level = 19;
    int lzma_level = 9;
    std::uint32_t lzma_dictionary_size = 0;
    std::uint64_t model_seed = kDefaultModelSeed;
};

struct CompressionStats {
    std::uint64_t input_bytes = 0;
    std::uint64_t archive_bytes = 0;
    std::uint64_t payload_bytes = 0;
    std::array<std::uint32_t, 10> blocks_by_mode{};
};

CompressionStats compress_file(const std::filesystem::path& input,
                               const std::filesystem::path& output,
                               const CompressionOptions& options = {});

void decompress_file(const std::filesystem::path& input,
                     const std::filesystem::path& output);

bool is_r2_archive(const std::filesystem::path& input);

}  // namespace hz::r2
