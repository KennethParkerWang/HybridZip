#include "app/cli.h"

#include <charconv>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "codec/decoder.h"
#include "codec/encoder.h"
#include "r2/codec/r2_codec.h"

namespace hz {
namespace {

void print_usage(std::ostream& output) {
    output << "Usage:\n\n"
              "  hybridzip c <input> <archive>\n"
              "  hybridzip c --profile=r2 "
              "[--r2-mode=auto|stored|zstd|fse|lzma|predictive|donor-match|bwt-zstd|bwt-mtf-zstd|bwt-rlt-zstd|x86-bcj-zstd|shuffle-zstd] "
              "[--block-size=BYTES] [--zstd-level=LEVEL] "
              "[--lzma-level=LEVEL] [--lzma-dictionary=BYTES] "
              "<input> <archive>\n"
              "  hybridzip d <archive> <output>\n";
}

std::uint32_t parse_u32(const std::string_view value, const char* option) {
    std::uint32_t parsed = 0;
    const char* const begin = value.data();
    const char* const end = begin + value.size();
    const auto result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc{} || result.ptr != end) {
        throw std::invalid_argument(std::string("Invalid ") + option);
    }
    return parsed;
}

int parse_int(const std::string_view value, const char* option) {
    int parsed = 0;
    const char* const begin = value.data();
    const char* const end = begin + value.size();
    const auto result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc{} || result.ptr != end) {
        throw std::invalid_argument(std::string("Invalid ") + option);
    }
    return parsed;
}

r2::CandidatePolicy parse_r2_mode(const std::string_view value) {
    if (value == "auto") {
        return r2::CandidatePolicy::Auto;
    }
    if (value == "stored") {
        return r2::CandidatePolicy::StoredOnly;
    }
    if (value == "zstd") {
        return r2::CandidatePolicy::ZstdOnly;
    }
    if (value == "predictive") {
        return r2::CandidatePolicy::PredictiveV1Only;
    }
    if (value == "fse") {
        return r2::CandidatePolicy::FseOnly;
    }
    if (value == "lzma") {
        return r2::CandidatePolicy::LzmaOnly;
    }
    if (value == "donor-match") {
        return r2::CandidatePolicy::DonorMatchPredictiveOnly;
    }
    if (value == "bwt-zstd") {
        return r2::CandidatePolicy::BwtZstdOnly;
    }
    if (value == "bwt-mtf-zstd") {
        return r2::CandidatePolicy::BwtMtfZstdOnly;
    }
    if (value == "bwt-rlt-zstd") {
        return r2::CandidatePolicy::BwtRltZstdOnly;
    }
    if (value == "x86-bcj-zstd") return r2::CandidatePolicy::X86BcjZstdOnly;
    if (value == "shuffle-zstd") return r2::CandidatePolicy::ShuffleZstdOnly;
    throw std::invalid_argument("Invalid --r2-mode");
}

void print_r2_stats(const r2::CompressionStats& stats) {
    std::cout << "HZ02 input=" << stats.input_bytes
              << " archive=" << stats.archive_bytes
              << " payload=" << stats.payload_bytes
              << " blocks(stored/predictive/zstd/fse/lzma/donor-match/bwt-zstd/bwt-mtf-zstd/bwt-rlt-zstd/x86-bcj-zstd/shuffle-zstd)="
              << stats.blocks_by_mode[0] << '/'
              << stats.blocks_by_mode[1] << '/'
              << stats.blocks_by_mode[2] << '/'
              << stats.blocks_by_mode[3] << '/'
              << stats.blocks_by_mode[4] << '/'
              << stats.blocks_by_mode[5] << '/'
              << stats.blocks_by_mode[6] << '/'
              << stats.blocks_by_mode[7] << '/'
              << stats.blocks_by_mode[8] << '/'
              << stats.blocks_by_mode[9] << '/'
              << stats.blocks_by_mode[10] << '\n';
}

}  // namespace

int run_cli(const int argc, char* argv[]) {
    if (argc == 2 && (std::string_view(argv[1]) == "-h" ||
                      std::string_view(argv[1]) == "--help")) {
        print_usage(std::cout);
        return 0;
    }
    if (argc < 2) {
        print_usage(std::cerr);
        return 2;
    }

    const std::string_view command(argv[1]);
    bool use_r2 = false;
    bool has_r2_options = false;
    r2::CompressionOptions r2_options{};
    std::vector<std::filesystem::path> paths;

    constexpr std::string_view kProfilePrefix = "--profile=";
    constexpr std::string_view kModePrefix = "--r2-mode=";
    constexpr std::string_view kBlockSizePrefix = "--block-size=";
    constexpr std::string_view kZstdLevelPrefix = "--zstd-level=";
    constexpr std::string_view kLzmaLevelPrefix = "--lzma-level=";
    constexpr std::string_view kLzmaDictionaryPrefix =
        "--lzma-dictionary=";
    for (int index = 2; index < argc; ++index) {
        const std::string_view argument(argv[index]);
        if (argument.compare(0, kProfilePrefix.size(), kProfilePrefix) == 0) {
            const std::string_view profile = argument.substr(kProfilePrefix.size());
            if (profile == "r2") {
                use_r2 = true;
            } else if (profile == "v1") {
                use_r2 = false;
            } else {
                throw std::invalid_argument("Invalid --profile");
            }
        } else if (argument.compare(0, kModePrefix.size(), kModePrefix) == 0) {
            r2_options.policy = parse_r2_mode(argument.substr(kModePrefix.size()));
            has_r2_options = true;
        } else if (argument.compare(0, kBlockSizePrefix.size(),
                                    kBlockSizePrefix) == 0) {
            r2_options.block_size = parse_u32(
                argument.substr(kBlockSizePrefix.size()), "--block-size");
            has_r2_options = true;
        } else if (argument.compare(0, kZstdLevelPrefix.size(),
                                    kZstdLevelPrefix) == 0) {
            r2_options.zstd_level = parse_int(
                argument.substr(kZstdLevelPrefix.size()), "--zstd-level");
            has_r2_options = true;
        } else if (argument.compare(0, kLzmaLevelPrefix.size(),
                                    kLzmaLevelPrefix) == 0) {
            r2_options.lzma_level = parse_int(
                argument.substr(kLzmaLevelPrefix.size()), "--lzma-level");
            has_r2_options = true;
        } else if (argument.compare(0, kLzmaDictionaryPrefix.size(),
                                    kLzmaDictionaryPrefix) == 0) {
            r2_options.lzma_dictionary_size = parse_u32(
                argument.substr(kLzmaDictionaryPrefix.size()),
                "--lzma-dictionary");
            has_r2_options = true;
        } else if (!argument.empty() && argument.front() == '-') {
            throw std::invalid_argument("Unknown command-line option");
        } else {
            paths.emplace_back(std::string(argument));
        }
    }

    if (paths.size() != 2) {
        print_usage(std::cerr);
        return 2;
    }
    const std::filesystem::path& input = paths[0];
    const std::filesystem::path& output = paths[1];
    if (command == "c") {
        if (has_r2_options && !use_r2) {
            throw std::invalid_argument(
                "R2 options require --profile=r2");
        }
        if (use_r2) {
            print_r2_stats(r2::compress_file(input, output, r2_options));
        } else {
            compress_file(input, output);
        }
        return 0;
    }
    if (command == "d") {
        if (use_r2 || has_r2_options) {
            throw std::invalid_argument(
                "Decompression detects the archive profile automatically");
        }
        if (r2::is_r2_archive(input)) {
            r2::decompress_file(input, output);
        } else {
            decompress_file(input, output);
        }
        return 0;
    }

    print_usage(std::cerr);
    return 2;
}

}  // namespace hz
