#include "codec/encoder.h"

#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <system_error>

#include "archive/archive_header.h"
#include "codec/model_pipeline.h"
#include "core/profile.h"
#include "entropy/arithmetic_codec.h"

namespace hz {
namespace {

std::filesystem::path temporary_path_for(const std::filesystem::path& output) {
    std::filesystem::path temporary = output;
    temporary += ".tmp";
    return temporary;
}

void validate_paths(const std::filesystem::path& input,
                    const std::filesystem::path& output,
                    const std::filesystem::path& temporary) {
    if (!std::filesystem::is_regular_file(input)) {
        throw std::runtime_error("Compression input is not a regular file");
    }
    if (std::filesystem::absolute(input).lexically_normal() ==
        std::filesystem::absolute(output).lexically_normal()) {
        throw std::runtime_error("Input and archive paths must differ");
    }
    if (std::filesystem::exists(output)) {
        throw std::runtime_error("Refusing to overwrite an existing archive");
    }
    if (std::filesystem::exists(temporary)) {
        throw std::runtime_error("Temporary archive path already exists");
    }
}

}  // namespace

void compress_file(const std::filesystem::path& input,
                   const std::filesystem::path& output) {
    const std::filesystem::path temporary = temporary_path_for(output);
    validate_paths(input, output, temporary);

    const std::uintmax_t reported_size = std::filesystem::file_size(input);
    if (reported_size > std::numeric_limits<std::uint64_t>::max()) {
        throw std::runtime_error("Input file is too large for HZ01");
    }
    const auto original_size = static_cast<std::uint64_t>(reported_size);

    try {
        std::ifstream source(input, std::ios::binary);
        std::ofstream archive(temporary,
                              std::ios::binary | std::ios::trunc);
        if (!source || !archive) {
            throw std::runtime_error("Failed to open compression paths");
        }

        ArchiveHeader header{};
        header.original_size = original_size;
        write_archive_header(archive, header);

        const Profile profile = make_profile_v1();
        ModelPipeline pipeline(profile);
        pipeline.reset(header.model_seed);
        ArithmeticEncoderStream coder(archive);

        std::uint64_t processed = 0;
        char value = 0;
        while (source.get(value)) {
            if (processed >= original_size) {
                throw std::runtime_error("Input file grew during compression");
            }
            const auto symbol = static_cast<std::uint8_t>(
                static_cast<unsigned char>(value));
            const Cdf& cdf = pipeline.predict_cdf();
            coder.write_symbol(cdf, symbol);
            pipeline.observe(symbol);
            ++processed;
        }
        if (source.bad()) {
            throw std::runtime_error("Failed while reading compression input");
        }
        if (processed != original_size) {
            throw std::runtime_error("Input file shrank during compression");
        }

        coder.finish();
        archive.flush();
        if (!archive) {
            throw std::runtime_error("Failed while writing HZ01 archive");
        }
        archive.close();
        source.close();
        std::filesystem::rename(temporary, output);
    } catch (...) {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        throw;
    }
}

}  // namespace hz
