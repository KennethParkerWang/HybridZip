#include "codec/decoder.h"

#include <cstdint>
#include <fstream>
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
        throw std::runtime_error("Archive input is not a regular file");
    }
    if (std::filesystem::absolute(input).lexically_normal() ==
        std::filesystem::absolute(output).lexically_normal()) {
        throw std::runtime_error("Archive and output paths must differ");
    }
    if (std::filesystem::exists(output)) {
        throw std::runtime_error("Refusing to overwrite an existing output");
    }
    if (std::filesystem::exists(temporary)) {
        throw std::runtime_error("Temporary output path already exists");
    }
}

}  // namespace

void decompress_file(const std::filesystem::path& input,
                     const std::filesystem::path& output) {
    const std::filesystem::path temporary = temporary_path_for(output);
    validate_paths(input, output, temporary);

    try {
        std::ifstream archive(input, std::ios::binary);
        std::ofstream restored(temporary,
                               std::ios::binary | std::ios::trunc);
        if (!archive || !restored) {
            throw std::runtime_error("Failed to open decompression paths");
        }

        const ArchiveHeader header = read_archive_header(archive);
        const Profile profile = make_profile_v1();
        ModelPipeline pipeline(profile);
        pipeline.reset(header.model_seed);
        ArithmeticDecoderStream coder(archive);

        for (std::uint64_t position = 0; position < header.original_size;
             ++position) {
            const Cdf& cdf = pipeline.predict_cdf();
            const std::uint8_t symbol = coder.read_symbol(cdf);
            restored.put(static_cast<char>(symbol));
            if (!restored) {
                throw std::runtime_error("Failed while writing decoded data");
            }
            pipeline.observe(symbol);
        }

        restored.flush();
        if (!restored) {
            throw std::runtime_error("Failed to flush decoded data");
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

}  // namespace hz
