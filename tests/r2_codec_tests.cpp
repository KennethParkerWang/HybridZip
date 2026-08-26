#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "codec/decoder.h"
#include "codec/encoder.h"
#include "r2/archive/r2_archive.h"
#include "r2/codec/r2_codec.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <typename Function>
void require_failure(Function&& function, const char* message) {
    try {
        std::forward<Function>(function)();
    } catch (const std::exception&) {
        return;
    }
    throw std::runtime_error(message);
}

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        const auto nonce = std::chrono::high_resolution_clock::now()
                               .time_since_epoch()
                               .count();
        path_ = std::filesystem::temp_directory_path() /
                ("hybridzip-r2-tests-" + std::to_string(nonce));
        if (!std::filesystem::create_directory(path_)) {
            throw std::runtime_error("Failed to create R2 test directory");
        }
    }

    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(path_, ignored);
    }

    const std::filesystem::path& path() const noexcept { return path_; }

private:
    std::filesystem::path path_;
};

void write_bytes(const std::filesystem::path& path,
                 const std::vector<std::uint8_t>& bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
    if (!output) {
        throw std::runtime_error("Failed to write R2 test file");
    }
}

std::vector<std::uint8_t> read_bytes(const std::filesystem::path& path) {
    const std::uintmax_t size = std::filesystem::file_size(path);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    std::ifstream input(path, std::ios::binary);
    input.read(reinterpret_cast<char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
    if (input.gcount() != static_cast<std::streamsize>(bytes.size())) {
        throw std::runtime_error("Failed to read R2 test file");
    }
    return bytes;
}

hz::r2::CompressionStats round_trip(
    const std::filesystem::path& directory,
    const std::string& name,
    const std::vector<std::uint8_t>& source_bytes,
    hz::r2::CompressionOptions options) {
    const std::filesystem::path source = directory / (name + ".input");
    const std::filesystem::path archive = directory / (name + ".hz2");
    const std::filesystem::path decoded = directory / (name + ".decoded");
    write_bytes(source, source_bytes);
    const hz::r2::CompressionStats stats =
        hz::r2::compress_file(source, archive, options);
    require(hz::r2::is_r2_archive(archive),
            "HZ02 archive detection failed");
    hz::r2::decompress_file(archive, decoded);
    require(read_bytes(decoded) == source_bytes,
            "HZ02 round trip was not byte-exact");
    require(stats.input_bytes == source_bytes.size() &&
                stats.archive_bytes == std::filesystem::file_size(archive),
            "HZ02 compression statistics are inconsistent");
    return stats;
}

std::vector<std::uint8_t> pseudo_random_bytes(const std::size_t size) {
    std::vector<std::uint8_t> bytes(size);
    std::uint32_t state = 0xC001D00DU;
    for (std::uint8_t& value : bytes) {
        state ^= state << 13U;
        state ^= state >> 17U;
        state ^= state << 5U;
        value = static_cast<std::uint8_t>(state >> 24U);
    }
    return bytes;
}

void test_empty_and_forced_modes(const std::filesystem::path& directory) {
    hz::r2::CompressionOptions options{};
    options.zstd_level = 3;

    const hz::r2::CompressionStats empty =
        round_trip(directory, "empty", {}, options);
    require(empty.archive_bytes == hz::r2::kR2ArchiveHeaderSize &&
                empty.blocks_by_mode == std::array<std::uint32_t, 43>{},
            "Empty HZ02 archive contract is wrong");

    options.policy = hz::r2::CandidatePolicy::StoredOnly;
    const auto stored = round_trip(
        directory, "stored", pseudo_random_bytes(257), options);
    require(stored.blocks_by_mode[0] == 1,
            "Forced stored mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::ZstdOnly;
    const std::vector<std::uint8_t> repeated(4096, 0x41U);
    const auto zstd = round_trip(directory, "zstd", repeated, options);
    require(zstd.blocks_by_mode[2] == 1,
            "Forced zstd mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::FseOnly;
    const auto fse = round_trip(directory, "fse", repeated, options);
    require(fse.blocks_by_mode[3] == 1,
            "Forced FSE mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::LzmaOnly;
    options.lzma_level = 5;
    const auto lzma = round_trip(directory, "lzma", repeated, options);
    require(lzma.blocks_by_mode[4] == 1,
            "Forced LZMA mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::Lz4Only;
    const auto lz4 = round_trip(directory, "lz4", repeated, options);
    require(lz4.blocks_by_mode[39] == 1,
            "Forced LZ4 mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::KanziAnsOnly;
    const auto kanzi_ans = round_trip(directory, "kanzi-ans", repeated, options);
    require(kanzi_ans.blocks_by_mode[40] == 1,
            "Forced Kanzi ANS mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::LmicArithmeticOnly;
    const auto lmic = round_trip(directory, "lmic-arithmetic", repeated, options);
    require(lmic.blocks_by_mode[41] == 1,
            "Forced LMIC arithmetic mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::DeltaBinaryPackedZstdOnly;
    const auto delta_binary_packed = round_trip(
        directory, "delta-binary-packed-zstd", pseudo_random_bytes(4096), options);
    require(delta_binary_packed.blocks_by_mode[42] == 1,
            "Forced delta-binary-packed+zstd mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::Ppmd7Only;
    const auto ppmd7 = round_trip(directory, "ppmd7", repeated, options);
    require(ppmd7.blocks_by_mode[27] == 1,
            "Forced PPMd7 mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::Ppmd8Only;
    const auto ppmd8 = round_trip(directory, "ppmd8", repeated, options);
    require(ppmd8.blocks_by_mode[28] == 1,
            "Forced PPMd8 mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::ZpaqOnly;
    const auto zpaq = round_trip(directory, "zpaq", repeated, options);
    require(zpaq.blocks_by_mode[29] == 1,
            "Forced ZPAQ mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::CtwOnly;
    const auto ctw = round_trip(directory, "ctw", repeated, options);
    require(ctw.blocks_by_mode[30] == 1,
            "Forced CTW mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::PredictiveV1Only;
    const std::string text = "abracadabra abracadabra";
    const std::vector<std::uint8_t> predictive_input(text.begin(), text.end());
    const auto predictive =
        round_trip(directory, "predictive", predictive_input, options);
    require(predictive.blocks_by_mode[1] == 1,
            "Forced predictive mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::DonorMatchPredictiveOnly;
    const auto donor_match = round_trip(
        directory, "donor-match", predictive_input, options);
    require(donor_match.blocks_by_mode[5] == 1,
            "Forced donor Match predictive mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::Paq8pxApmPredictiveOnly;
    const auto paq8px_apm = round_trip(
        directory, "paq8px-apm", predictive_input, options);
    require(paq8px_apm.blocks_by_mode[31] == 1,
            "Forced PAQ8px APM mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::Paq8pxRecordModelOnly;
    const auto paq8px_record_model = round_trip(
        directory, "paq8px-record-model", predictive_input, options);
    require(paq8px_record_model.blocks_by_mode[32] == 1,
            "Forced PAQ8px RecordModel mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::Paq8pxLinearPredictionOnly;
    const auto paq8px_linear_prediction = round_trip(
        directory, "paq8px-linear-prediction", predictive_input, options);
    require(paq8px_linear_prediction.blocks_by_mode[33] == 1,
            "Forced PAQ8px LinearPredictionModel mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::Paq8pxSimilarityOnly;
    const auto paq8px_similarity = round_trip(
        directory, "paq8px-similarity", predictive_input, options);
    require(paq8px_similarity.blocks_by_mode[34] == 1,
            "Forced PAQ8px SimilarityModel mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::Paq8pxSimilaritySseOnly;
    const auto paq8px_similarity_sse = round_trip(
        directory, "paq8px-similarity-sse", predictive_input, options);
    require(paq8px_similarity_sse.blocks_by_mode[35] == 1,
            "Forced PAQ8px Similarity+SSE mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::Paq8pxGenericSseOnly;
    const auto paq8px_generic_sse = round_trip(
        directory, "paq8px-generic-sse", predictive_input, options);
    require(paq8px_generic_sse.blocks_by_mode[36] == 1,
            "Forced PAQ8px Generic+SSE mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::Paq8pxDetectedSseOnly;
    const auto paq8px_detected_sse = round_trip(
        directory, "paq8px-detected-sse", predictive_input, options);
    require(paq8px_detected_sse.blocks_by_mode[37] == 1,
            "Forced PAQ8px detected specialist+SSE mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::WavpackOnly;
    const auto wavpack = round_trip(
        directory, "wavpack", repeated, options);
    require(wavpack.blocks_by_mode[38] == 1,
            "Forced WavPack mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::NeuralLstmOnly;
    const auto neural = round_trip(directory, "neural-lstm", predictive_input,
                                   options);
    require(neural.blocks_by_mode[21] == 1,
            "Forced neural LSTM mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::SharedNeuralLstmOnly;
    const auto shared_neural = round_trip(directory, "shared-neural-lstm",
                                          predictive_input, options);
    require(shared_neural.blocks_by_mode[22] == 1,
            "Forced shared neural LSTM mode selected another backend");
    const std::filesystem::path shared_archive =
        directory / "shared-neural-lstm.hz2";
    std::vector<std::uint8_t> unknown_model = read_bytes(shared_archive);
    constexpr std::size_t kSharedModelIdOffset =
        hz::r2::kR2ArchiveHeaderSize + hz::r2::kR2BlockHeaderSize +
        hz::r2::kR2BlockChecksumSize;
    unknown_model[kSharedModelIdOffset] ^= 0x01U;
    const std::filesystem::path unknown_model_archive =
        directory / "shared-neural-lstm-unknown.hz2";
    const std::filesystem::path unknown_model_output =
        directory / "shared-neural-lstm-unknown.out";
    write_bytes(unknown_model_archive, unknown_model);
    require_failure(
        [&] { hz::r2::decompress_file(unknown_model_archive,
                                      unknown_model_output); },
        "Unknown shared neural model ID was accepted");
    require(!std::filesystem::exists(unknown_model_output),
            "Unknown shared neural model ID published output");

    options.policy = hz::r2::CandidatePolicy::BgptSharedPriorOnly;
    const auto bgpt_shared = round_trip(directory, "bgpt-shared-prior",
                                        predictive_input, options);
    require(bgpt_shared.blocks_by_mode[25] == 1,
            "Forced bGPT shared-prior mode selected another backend");
    const std::filesystem::path bgpt_archive =
        directory / "bgpt-shared-prior.hz2";
    std::vector<std::uint8_t> unknown_checkpoint = read_bytes(bgpt_archive);
    constexpr std::size_t kBgptCheckpointHashOffset =
        hz::r2::kR2ArchiveHeaderSize + hz::r2::kR2BlockHeaderSize +
        hz::r2::kR2BlockChecksumSize + 4U;
    unknown_checkpoint[kBgptCheckpointHashOffset] ^= 0x01U;
    const std::filesystem::path unknown_checkpoint_archive =
        directory / "bgpt-shared-prior-unknown.hz2";
    const std::filesystem::path unknown_checkpoint_output =
        directory / "bgpt-shared-prior-unknown.out";
    write_bytes(unknown_checkpoint_archive, unknown_checkpoint);
    require_failure(
        [&] { hz::r2::decompress_file(unknown_checkpoint_archive,
                                      unknown_checkpoint_output); },
        "Unknown bGPT checkpoint hash was accepted");
    require(!std::filesystem::exists(unknown_checkpoint_output),
            "Unknown bGPT checkpoint hash published output");

    options.policy = hz::r2::CandidatePolicy::JaxCompressPortableOnly;
    const auto jax_portable = round_trip(directory, "jax-compress-portable",
                                         predictive_input, options);
    require(jax_portable.blocks_by_mode[26] == 1,
            "Forced jax-compress portable mode selected another backend");
    const std::filesystem::path jax_archive =
        directory / "jax-compress-portable.hz2";
    std::vector<std::uint8_t> unknown_jax_profile = read_bytes(jax_archive);
    constexpr std::size_t kJaxProfileHashOffset =
        hz::r2::kR2ArchiveHeaderSize + hz::r2::kR2BlockHeaderSize +
        hz::r2::kR2BlockChecksumSize + 4U + 20U;
    unknown_jax_profile[kJaxProfileHashOffset] ^= 0x01U;
    const std::filesystem::path unknown_jax_profile_archive =
        directory / "jax-compress-portable-unknown.hz2";
    const std::filesystem::path unknown_jax_profile_output =
        directory / "jax-compress-portable-unknown.out";
    write_bytes(unknown_jax_profile_archive, unknown_jax_profile);
    require_failure(
        [&] { hz::r2::decompress_file(unknown_jax_profile_archive,
                                      unknown_jax_profile_output); },
        "Unknown jax-compress portable profile hash was accepted");
    require(!std::filesystem::exists(unknown_jax_profile_output),
            "Unknown jax-compress portable profile hash published output");

    options.policy = hz::r2::CandidatePolicy::BwtZstdOnly;
    const auto bwt_zstd = round_trip(directory, "bwt-zstd", repeated, options);
    require(bwt_zstd.blocks_by_mode[6] == 1,
            "Forced BWT+zstd mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::BwtMtfZstdOnly;
    const auto bwt_mtf_zstd = round_trip(directory, "bwt-mtf-zstd", repeated, options);
    require(bwt_mtf_zstd.blocks_by_mode[7] == 1,
            "Forced BWT+MTF+zstd mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::BwtRltZstdOnly;
    const std::string rlt_text_unit =
        "the quick brown fox jumps over the lazy dog. ";
    std::vector<std::uint8_t> rlt_input;
    for (std::size_t index = 0; index < 256; ++index) {
        rlt_input.insert(rlt_input.end(), rlt_text_unit.begin(),
                         rlt_text_unit.end());
    }
    const auto bwt_rlt_zstd = round_trip(directory, "bwt-rlt-zstd", rlt_input, options);
    require(bwt_rlt_zstd.blocks_by_mode[8] == 1,
            "Forced BWT+RLT+zstd mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::X86BcjZstdOnly;
    const auto x86_bcj_zstd = round_trip(directory, "x86-bcj-zstd", repeated, options);
    require(x86_bcj_zstd.blocks_by_mode[9] == 1,
            "Forced x86 BCJ+zstd mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::ShuffleZstdOnly;
    const auto shuffle_zstd = round_trip(directory, "shuffle-zstd", repeated, options);
    require(shuffle_zstd.blocks_by_mode[10] == 1,
            "Forced shuffle+zstd mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::BitshuffleZstdOnly;
    const auto bitshuffle_zstd = round_trip(directory, "bitshuffle-zstd", repeated, options);
    require(bitshuffle_zstd.blocks_by_mode[11] == 1,
            "Forced bitshuffle+zstd mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::DeltaZstdOnly;
    const auto delta_zstd = round_trip(directory, "delta-zstd", repeated, options);
    require(delta_zstd.blocks_by_mode[12] == 1,
            "Forced delta+zstd mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::DeltaOfDeltaZstdOnly;
    const auto delta_of_delta = round_trip(
        directory, "delta-of-delta-zstd", repeated, options);
    require(delta_of_delta.blocks_by_mode[24] == 1,
            "Forced delta-of-delta+zstd mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::FastPforOnly;
    const auto fastpfor = round_trip(
        directory, "fastpfor", pseudo_random_bytes(4096), options);
    require(fastpfor.blocks_by_mode[13] == 1,
            "Forced FastPFOR mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::RansOnly;
    const auto rans = round_trip(
        directory, "rans", pseudo_random_bytes(4096), options);
    require(rans.blocks_by_mode[14] == 1,
            "Forced rANS mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::Bcj2ZstdOnly;
    const auto bcj2 = round_trip(
        directory, "bcj2", pseudo_random_bytes(4096), options);
    require(bcj2.blocks_by_mode[15] == 1,
            "Forced BCJ2 mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::RecordTransposeZstdOnly;
    const auto record_transpose = round_trip(
        directory, "record-transpose", pseudo_random_bytes(4096), options);
    require(record_transpose.blocks_by_mode[16] == 1,
            "Forced record transpose mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::JpegLsOnly;
    const auto jpeg_ls = round_trip(
        directory, "jpegls", pseudo_random_bytes(4096), options);
    require(jpeg_ls.blocks_by_mode[17] == 1,
            "Forced JPEG-LS mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::FlacResidualOnly;
    const auto flac_residual = round_trip(
        directory, "flac-residual", pseudo_random_bytes(4096), options);
    require(flac_residual.blocks_by_mode[18] == 1,
            "Forced FLAC residual mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::BrotliTextOnly;
    const auto brotli_text = round_trip(
        directory, "brotli-text", predictive_input, options);
    require(brotli_text.blocks_by_mode[19] == 1,
            "Forced Brotli text mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::CmixWordDictionaryZstdOnly;
    const auto cmix_word = round_trip(
        directory, "cmix-word-zstd", predictive_input, options);
    require(cmix_word.blocks_by_mode[20] == 1,
            "Forced cmix word dictionary mode selected another backend");
}

void test_auto_selection(const std::filesystem::path& directory) {
    hz::r2::CompressionOptions options{};
    options.policy = hz::r2::CandidatePolicy::Auto;
    options.zstd_level = 3;

    const std::vector<std::uint8_t> repeated(4096, 0x5AU);
    const auto compressible =
        round_trip(directory, "auto-compressible", repeated, options);
    require(compressible.blocks_by_mode[0] == 0 &&
                compressible.blocks_by_mode[1] +
                        compressible.blocks_by_mode[2] +
                        compressible.blocks_by_mode[3] +
                        compressible.blocks_by_mode[4] +
                        compressible.blocks_by_mode[5] +
                        compressible.blocks_by_mode[6] +
                        compressible.blocks_by_mode[7] +
                        compressible.blocks_by_mode[8] +
                        compressible.blocks_by_mode[9] +
                        compressible.blocks_by_mode[10] +
                        compressible.blocks_by_mode[11] +
                        compressible.blocks_by_mode[12] +
                        compressible.blocks_by_mode[13] +
                        compressible.blocks_by_mode[14] +
                        compressible.blocks_by_mode[15] +
                        compressible.blocks_by_mode[16] +
                        compressible.blocks_by_mode[17] +
                        compressible.blocks_by_mode[18] +
                        compressible.blocks_by_mode[19] +
                        compressible.blocks_by_mode[20] +
                        compressible.blocks_by_mode[21] +
                        compressible.blocks_by_mode[22] +
                        compressible.blocks_by_mode[23] +
                        compressible.blocks_by_mode[24] +
                        compressible.blocks_by_mode[25] +
                        compressible.blocks_by_mode[26] +
                        compressible.blocks_by_mode[27] +
                        compressible.blocks_by_mode[28] +
                        compressible.blocks_by_mode[29] +
                        compressible.blocks_by_mode[30] +
                        compressible.blocks_by_mode[31] +
                        compressible.blocks_by_mode[32] +
                        compressible.blocks_by_mode[33] +
                        compressible.blocks_by_mode[34] +
                        compressible.blocks_by_mode[35] +
                        compressible.blocks_by_mode[36] +
                        compressible.blocks_by_mode[37] +
                        compressible.blocks_by_mode[38] +
                        compressible.blocks_by_mode[39] ==
                    1,
            "Auto mode selected stored for compressible repeated data");

    const auto incompressible = round_trip(
        directory, "auto-incompressible", pseudo_random_bytes(2048), options);
    require(incompressible.blocks_by_mode[0] == 1,
            "Auto mode did not select stored for high-entropy data");
}

void test_corrupt_archives(const std::filesystem::path& directory) {
    const std::vector<std::uint8_t> source_bytes =
        pseudo_random_bytes(128);
    const std::filesystem::path source = directory / "corrupt.input";
    const std::filesystem::path valid = directory / "corrupt-valid.hz2";
    write_bytes(source, source_bytes);
    hz::r2::CompressionOptions options{};
    options.policy = hz::r2::CandidatePolicy::StoredOnly;
    hz::r2::compress_file(source, valid, options);
    const std::vector<std::uint8_t> valid_bytes = read_bytes(valid);
    require(valid_bytes.size() == hz::r2::kR2ArchiveHeaderSize +
                                      hz::r2::kR2BlockHeaderSize +
                                      hz::r2::kR2BlockChecksumSize +
                                      source_bytes.size(),
            "Stored HZ02 archive accounting is wrong");

    auto expect_rejected = [&](std::vector<std::uint8_t> bytes,
                               const std::string& name) {
        const std::filesystem::path archive = directory / (name + ".hz2");
        const std::filesystem::path output = directory / (name + ".out");
        write_bytes(archive, bytes);
        require_failure([&] { hz::r2::decompress_file(archive, output); },
                        "Malformed HZ02 archive was accepted");
        require(!std::filesystem::exists(output),
                "Failed HZ02 decode published partial output");
    };

    std::vector<std::uint8_t> truncated = valid_bytes;
    truncated.pop_back();
    expect_rejected(std::move(truncated), "truncated");

    std::vector<std::uint8_t> trailing = valid_bytes;
    trailing.push_back(0);
    expect_rejected(std::move(trailing), "trailing");

    std::vector<std::uint8_t> invalid_mode = valid_bytes;
    invalid_mode[hz::r2::kR2ArchiveHeaderSize] = 0x7FU;
    expect_rejected(std::move(invalid_mode), "invalid-mode");

    std::vector<std::uint8_t> invalid_checksum = valid_bytes;
    invalid_checksum[hz::r2::kR2ArchiveHeaderSize +
                     hz::r2::kR2BlockHeaderSize] ^= 0x01U;
    expect_rejected(std::move(invalid_checksum), "invalid-checksum");

    std::vector<std::uint8_t> corrupt_payload = valid_bytes;
    corrupt_payload.back() ^= 0x80U;
    expect_rejected(std::move(corrupt_payload), "corrupt-payload");

    hz::r2::CompressionOptions bwt_options{};
    bwt_options.policy = hz::r2::CandidatePolicy::BwtZstdOnly;
    const std::filesystem::path bwt_archive = directory / "bwt-valid.hz2";
    const std::filesystem::path bwt_input = directory / "bwt.input";
    const std::string bwt_text = "banana_bandana::HybridZip::";
    std::vector<std::uint8_t> bwt_bytes;
    for (std::size_t index = 0; index < 32; ++index) {
        bwt_bytes.insert(bwt_bytes.end(), bwt_text.begin(), bwt_text.end());
    }
    write_bytes(bwt_input, bwt_bytes);
    hz::r2::compress_file(bwt_input, bwt_archive, bwt_options);
    std::vector<std::uint8_t> invalid_primary = read_bytes(bwt_archive);
    invalid_primary[hz::r2::kR2ArchiveHeaderSize +
                    hz::r2::kR2BlockHeaderSize +
                    hz::r2::kR2BlockChecksumSize] = 0;
    expect_rejected(std::move(invalid_primary), "invalid-bwt-primary");
}

void test_hz01_regression(const std::filesystem::path& directory) {
    const std::string text = "HybridZip HZ01 regression\n";
    const std::vector<std::uint8_t> expected(text.begin(), text.end());
    const std::filesystem::path source = directory / "v1.input";
    const std::filesystem::path archive = directory / "v1.hz";
    const std::filesystem::path decoded = directory / "v1.decoded";
    write_bytes(source, expected);
    hz::compress_file(source, archive);
    require(!hz::r2::is_r2_archive(archive),
            "HZ01 archive was misidentified as HZ02");
    hz::decompress_file(archive, decoded);
    require(read_bytes(decoded) == expected,
            "HZ01 regression round trip failed");
}

}  // namespace

int main() {
    try {
        const TemporaryDirectory temporary;
        test_empty_and_forced_modes(temporary.path());
        test_auto_selection(temporary.path());
        test_corrupt_archives(temporary.path());
        test_hz01_regression(temporary.path());
        std::cout << "r2_codec_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "r2_codec_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
