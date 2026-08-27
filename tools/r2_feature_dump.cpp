#include <array>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "r2/core/byte_view.h"
#include "r2/routing/block_features.h"
#include "r2/routing/mode_ranker.h"

namespace {

constexpr std::string_view kOffsetPrefix = "--offset=";
constexpr std::string_view kLengthPrefix = "--length=";

void print_usage(std::ostream& output) {
    output << "Usage: hz_r2_feature_dump <input> [--offset=<bytes>] "
              "[--length=<bytes>]\n";
}

std::size_t parse_size(const std::string_view text, const char* name) {
    if (text.empty()) {
        throw std::invalid_argument(std::string(name) + " cannot be empty");
    }
    std::size_t parsed_characters = 0U;
    unsigned long long value = 0U;
    try {
        value = std::stoull(std::string(text), &parsed_characters, 10);
    } catch (const std::exception&) {
        throw std::invalid_argument(std::string("Invalid ") + name + ": " +
                                    std::string(text));
    }
    if (parsed_characters != text.size() ||
        value > std::numeric_limits<std::size_t>::max()) {
        throw std::invalid_argument(std::string("Invalid ") + name + ": " +
                                    std::string(text));
    }
    return static_cast<std::size_t>(value);
}

const char* block_class_name(const hz::r2::BlockClass value) {
    switch (value) {
    case hz::r2::BlockClass::Text:
        return "text";
    case hz::r2::BlockClass::X86:
        return "x86";
    case hz::r2::BlockClass::Numeric:
        return "numeric";
    case hz::r2::BlockClass::Generic:
        return "generic";
    }
    return "unknown";
}

std::vector<std::uint8_t> read_slice(const std::filesystem::path& path,
                                     const std::size_t offset,
                                     const std::size_t length) {
    std::error_code error;
    const std::uintmax_t file_bytes = std::filesystem::file_size(path, error);
    if (error) {
        throw std::runtime_error("Unable to read input size: " + path.string());
    }
    if (file_bytes > std::numeric_limits<std::size_t>::max() ||
        offset > static_cast<std::size_t>(file_bytes) ||
        length > static_cast<std::size_t>(file_bytes) - offset ||
        length > static_cast<std::size_t>(
            std::numeric_limits<std::streamsize>::max())) {
        throw std::invalid_argument("Requested input slice is outside the file");
    }

    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Unable to open input: " + path.string());
    }
    input.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!input) {
        throw std::runtime_error("Unable to seek input: " + path.string());
    }
    std::vector<std::uint8_t> bytes(length);
    if (length != 0U) {
        input.read(reinterpret_cast<char*>(bytes.data()),
                   static_cast<std::streamsize>(length));
        if (input.gcount() != static_cast<std::streamsize>(length)) {
            throw std::runtime_error("Unable to read requested input slice");
        }
    }
    return bytes;
}

template <typename Value>
void write_json_array(std::ostream& output, const std::vector<Value>& values) {
    output << '[';
    for (std::size_t index = 0U; index < values.size(); ++index) {
        if (index != 0U) output << ',';
        output << values[index];
    }
    output << ']';
}

template <typename Value, std::size_t Size>
void write_json_array(std::ostream& output,
                      const std::array<Value, Size>& values) {
    output << '[';
    for (std::size_t index = 0U; index < values.size(); ++index) {
        if (index != 0U) output << ',';
        output << values[index];
    }
    output << ']';
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 2 && std::string_view(argv[1]) == "--help") {
            print_usage(std::cout);
            return 0;
        }
        if (argc < 2) {
            print_usage(std::cerr);
            return 2;
        }

        const std::filesystem::path input_path(argv[1]);
        std::size_t offset = 0U;
        std::size_t length = 0U;
        bool length_given = false;
        for (int index = 2; index < argc; ++index) {
            const std::string_view argument(argv[index]);
            if (argument.substr(0U, kOffsetPrefix.size()) == kOffsetPrefix) {
                offset = parse_size(argument.substr(kOffsetPrefix.size()), "offset");
            } else if (argument.substr(0U, kLengthPrefix.size()) == kLengthPrefix) {
                if (length_given) {
                    throw std::invalid_argument("--length was specified more than once");
                }
                length = parse_size(argument.substr(kLengthPrefix.size()), "length");
                length_given = true;
            } else {
                throw std::invalid_argument("Unknown argument: " +
                                            std::string(argument));
            }
        }

        std::error_code error;
        const std::uintmax_t file_bytes = std::filesystem::file_size(input_path, error);
        if (error || file_bytes > std::numeric_limits<std::size_t>::max() ||
            offset > static_cast<std::size_t>(file_bytes)) {
            throw std::invalid_argument("Input path or offset is invalid");
        }
        if (!length_given) {
            length = static_cast<std::size_t>(file_bytes) - offset;
        }

        const std::vector<std::uint8_t> input = read_slice(input_path, offset, length);
        const hz::r2::BlockFeaturesV1 features =
            hz::r2::extract_block_features(hz::r2::ByteView(input));
        const hz::r2::FixedPointRankerModelV1& ranker =
            hz::r2::fixed_point_ranker_model_v1();
        if (!hz::r2::fixed_point_ranker_model_v1_valid()) {
            throw std::logic_error("Fixed-point ranker model checksum is invalid");
        }
        const std::vector<hz::r2::BlockMode> shortlist =
            hz::r2::rank_modes_k8(features);
        std::vector<unsigned int> shortlist_ids;
        shortlist_ids.reserve(shortlist.size());
        for (const hz::r2::BlockMode mode : shortlist) {
            shortlist_ids.push_back(static_cast<unsigned int>(mode));
        }

        std::cout << "{\"schema\":\"r2-block-features-v1\""
                  << ",\"input_offset_bytes\":" << offset
                  << ",\"input_bytes\":" << input.size()
                  << ",\"block_class\":\"" << block_class_name(features.classify())
                  << "\",\"feature_values\":";
        write_json_array(std::cout, features.values);
        std::cout << ",\"auto_k8_mode_ids\":";
        write_json_array(std::cout, shortlist_ids);
        std::cout << ",\"ranker_model\":{\"version\":" << ranker.version
                  << ",\"crc32\":" << ranker.crc32
                  << ",\"sha256\":\""
                  << hz::r2::fixed_point_ranker_model_v1_sha256_hex()
                  << "\"}}\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "hz_r2_feature_dump: " << error.what() << '\n';
        return 1;
    }
}
