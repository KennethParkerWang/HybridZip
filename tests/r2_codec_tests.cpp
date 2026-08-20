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
                empty.blocks_by_mode == std::array<std::uint32_t, 8>{},
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

    options.policy = hz::r2::CandidatePolicy::BwtZstdOnly;
    const auto bwt_zstd = round_trip(directory, "bwt-zstd", repeated, options);
    require(bwt_zstd.blocks_by_mode[6] == 1,
            "Forced BWT+zstd mode selected another backend");

    options.policy = hz::r2::CandidatePolicy::BwtMtfZstdOnly;
    const auto bwt_mtf_zstd = round_trip(directory, "bwt-mtf-zstd", repeated, options);
    require(bwt_mtf_zstd.blocks_by_mode[7] == 1,
            "Forced BWT+MTF+zstd mode selected another backend");
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
                        compressible.blocks_by_mode[7] ==
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
    write_bytes(bwt_input, std::vector<std::uint8_t>(512, 0x42U));
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
