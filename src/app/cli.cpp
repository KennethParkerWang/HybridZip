#include "app/cli.h"

#include <array>
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
              "[--r2-mode=auto|auto-k2|auto-k4|auto-k8|fast|fast-ext|stored|zstd|fse|lzma|lz4|kanzi-ans|lmic-arithmetic|delta-binary-packed-zstd|ppmd7|ppmd8|zpaq|ctw|predictive|donor-match|paq8px-apm|paq8px-record-model|paq8px-linear-prediction|paq8px-similarity|paq8px-similarity-sse|paq8px-generic-sse|paq8px-detected-sse|wavpack|bwt-zstd|bwt-mtf-zstd|bwt-rlt-zstd|x86-bcj-zstd|bcj2-zstd|shuffle-zstd|bitshuffle-zstd|delta-zstd|delta-of-delta-zstd|fastpfor|rans|record-transpose-zstd|jpegls|flac-residual|brotli-text|cmix-word-zstd|neural-lstm|shared-neural-lstm|lstm-compress|bgpt-shared-prior|jax-compress-portable] "
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
    if (value == "auto-k2") {
        return r2::CandidatePolicy::AutoK2;
    }
    if (value == "auto-k4") {
        return r2::CandidatePolicy::AutoK4;
    }
    if (value == "auto-k8") {
        return r2::CandidatePolicy::AutoK8;
    }
    if (value == "fast") {
        return r2::CandidatePolicy::Fast;
    }
    if (value == "fast-ext") {
        return r2::CandidatePolicy::FastExtensionOnly;
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
    if (value == "lz4") {
        return r2::CandidatePolicy::Lz4Only;
    }
    if (value == "kanzi-ans") {
        return r2::CandidatePolicy::KanziAnsOnly;
    }
    if (value == "lmic-arithmetic") {
        return r2::CandidatePolicy::LmicArithmeticOnly;
    }
    if (value == "delta-binary-packed-zstd") {
        return r2::CandidatePolicy::DeltaBinaryPackedZstdOnly;
    }
    if (value == "ppmd7") {
        return r2::CandidatePolicy::Ppmd7Only;
    }
    if (value == "ppmd8") {
        return r2::CandidatePolicy::Ppmd8Only;
    }
    if (value == "zpaq") {
        return r2::CandidatePolicy::ZpaqOnly;
    }
    if (value == "ctw") {
        return r2::CandidatePolicy::CtwOnly;
    }
    if (value == "donor-match") {
        return r2::CandidatePolicy::DonorMatchPredictiveOnly;
    }
    if (value == "paq8px-apm") {
        return r2::CandidatePolicy::Paq8pxApmPredictiveOnly;
    }
    if (value == "paq8px-record-model") {
        return r2::CandidatePolicy::Paq8pxRecordModelOnly;
    }
    if (value == "paq8px-linear-prediction") {
        return r2::CandidatePolicy::Paq8pxLinearPredictionOnly;
    }
    if (value == "paq8px-similarity") {
        return r2::CandidatePolicy::Paq8pxSimilarityOnly;
    }
    if (value == "paq8px-similarity-sse") {
        return r2::CandidatePolicy::Paq8pxSimilaritySseOnly;
    }
    if (value == "paq8px-generic-sse") {
        return r2::CandidatePolicy::Paq8pxGenericSseOnly;
    }
    if (value == "paq8px-detected-sse") {
        return r2::CandidatePolicy::Paq8pxDetectedSseOnly;
    }
    if (value == "wavpack") {
        return r2::CandidatePolicy::WavpackOnly;
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
    if (value == "bitshuffle-zstd") return r2::CandidatePolicy::BitshuffleZstdOnly;
    if (value == "delta-zstd") return r2::CandidatePolicy::DeltaZstdOnly;
    if (value == "delta-of-delta-zstd") {
        return r2::CandidatePolicy::DeltaOfDeltaZstdOnly;
    }
    if (value == "fastpfor") return r2::CandidatePolicy::FastPforOnly;
    if (value == "rans") return r2::CandidatePolicy::RansOnly;
    if (value == "bcj2-zstd") return r2::CandidatePolicy::Bcj2ZstdOnly;
    if (value == "record-transpose-zstd") return r2::CandidatePolicy::RecordTransposeZstdOnly;
    if (value == "jpegls") return r2::CandidatePolicy::JpegLsOnly;
    if (value == "flac-residual") return r2::CandidatePolicy::FlacResidualOnly;
    if (value == "brotli-text") return r2::CandidatePolicy::BrotliTextOnly;
    if (value == "cmix-word-zstd") {
        return r2::CandidatePolicy::CmixWordDictionaryZstdOnly;
    }
    if (value == "neural-lstm") {
        return r2::CandidatePolicy::NeuralLstmOnly;
    }
    if (value == "shared-neural-lstm") {
        return r2::CandidatePolicy::SharedNeuralLstmOnly;
    }
    if (value == "lstm-compress") {
        return r2::CandidatePolicy::LstmCompressOnly;
    }
    if (value == "bgpt-shared-prior") {
        return r2::CandidatePolicy::BgptSharedPriorOnly;
    }
    if (value == "jax-compress-portable") {
        return r2::CandidatePolicy::JaxCompressPortableOnly;
    }
    throw std::invalid_argument("Invalid --r2-mode");
}

void print_candidate_modes(
    std::ostream& output,
    const std::array<std::uint32_t, r2::kR2BlockModeCount>& candidate_blocks_by_mode) {
    output << " candidate_modes=";
    bool first = true;
    for (std::size_t mode = 0; mode < candidate_blocks_by_mode.size(); ++mode) {
        if (candidate_blocks_by_mode[mode] == 0U) {
            continue;
        }
        if (!first) {
            output << ',';
        }
        output << mode << ':' << candidate_blocks_by_mode[mode];
        first = false;
    }
    if (first) {
        output << "none";
    }
}

void print_r2_stats(const r2::CompressionStats& stats) {
    std::cout << "HZ02 input=" << stats.input_bytes
              << " archive=" << stats.archive_bytes
              << " payload=" << stats.payload_bytes
              << " candidates=" << stats.candidates_evaluated
              << " selected=" << stats.selected_candidate_bytes
              << " oracle=" << stats.oracle_candidate_bytes
              << " oracle_gap=" << stats.oracle_gap_bytes
              << " full_oracle=" << (stats.full_oracle_evaluated ? 1 : 0);
    print_candidate_modes(std::cout, stats.candidate_blocks_by_mode);
    std::cout << " blocks(stored/predictive/zstd/fse/lzma/donor-match/bwt-zstd/bwt-mtf-zstd/bwt-rlt-zstd/x86-bcj-zstd/shuffle-zstd/bitshuffle-zstd/delta-zstd/fastpfor/rans/bcj2-zstd/record-transpose-zstd/jpegls/flac-residual/brotli-text/cmix-word-zstd/neural-lstm/shared-neural-lstm/lstm-compress/delta-of-delta-zstd/bgpt-shared-prior/jax-compress-portable/ppmd7/ppmd8/zpaq/ctw/paq8px-apm/paq8px-record-model/paq8px-linear-prediction/paq8px-similarity/paq8px-similarity-sse/paq8px-generic-sse/paq8px-detected-sse/wavpack/lz4/kanzi-ans/lmic-arithmetic/delta-binary-packed-zstd/fast-ext)="
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
              << stats.blocks_by_mode[10] << '/'
              << stats.blocks_by_mode[11] << '/'
              << stats.blocks_by_mode[12] << '/'
              << stats.blocks_by_mode[13] << '/'
              << stats.blocks_by_mode[14] << '/'
              << stats.blocks_by_mode[15] << '/'
              << stats.blocks_by_mode[16] << '/'
              << stats.blocks_by_mode[17] << '/'
              << stats.blocks_by_mode[18] << '/'
              << stats.blocks_by_mode[19] << '/'
              << stats.blocks_by_mode[20] << '/'
              << stats.blocks_by_mode[21] << '/'
              << stats.blocks_by_mode[22] << '/'
              << stats.blocks_by_mode[23] << '/'
              << stats.blocks_by_mode[24] << '/'
              << stats.blocks_by_mode[25] << '/'
              << stats.blocks_by_mode[26] << '/'
              << stats.blocks_by_mode[27] << '/'
              << stats.blocks_by_mode[28] << '/'
              << stats.blocks_by_mode[29] << '/'
              << stats.blocks_by_mode[30] << '/'
              << stats.blocks_by_mode[31] << '/'
              << stats.blocks_by_mode[32] << '/'
              << stats.blocks_by_mode[33] << '/'
              << stats.blocks_by_mode[34] << '/'
              << stats.blocks_by_mode[35] << '/'
              << stats.blocks_by_mode[36] << '/'
              << stats.blocks_by_mode[37] << '/'
              << stats.blocks_by_mode[38] << '/'
              << stats.blocks_by_mode[39] << '/'
              << stats.blocks_by_mode[40] << '/'
              << stats.blocks_by_mode[41] << '/'
              << stats.blocks_by_mode[42] << '/'
              << stats.blocks_by_mode[43] << '\n';
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
