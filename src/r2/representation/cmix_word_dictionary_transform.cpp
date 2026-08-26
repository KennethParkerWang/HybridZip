#include "r2/representation/cmix_word_dictionary_transform.h"

#include <cstdio>
#include <limits>
#include <stdexcept>
#include <utility>

#include "r2/representation/cmix_dictionary_data.h"

#include "dictionary.h"

namespace hz::r2 {
namespace {

class TemporaryFile final {
public:
    TemporaryFile() : file_(std::tmpfile()) {
        if (file_ == nullptr) {
            throw std::runtime_error("cmix dictionary temporary file creation failed");
        }
    }

    ~TemporaryFile() {
        if (file_ != nullptr) {
            std::fclose(file_);
        }
    }

    TemporaryFile(const TemporaryFile&) = delete;
    TemporaryFile& operator=(const TemporaryFile&) = delete;

    std::FILE* get() const noexcept { return file_; }

    void write(const ByteView bytes) const {
        if (!bytes.empty() &&
            std::fwrite(bytes.data(), 1U, bytes.size(), file_) != bytes.size()) {
            throw std::runtime_error("cmix dictionary temporary file write failed");
        }
        if (std::fflush(file_) != 0 || std::fseek(file_, 0, SEEK_SET) != 0) {
            throw std::runtime_error("cmix dictionary temporary file rewind failed");
        }
    }

    std::vector<std::uint8_t> read_all(const std::size_t maximum_size) const {
        if (std::fflush(file_) != 0 || std::fseek(file_, 0, SEEK_END) != 0) {
            throw std::runtime_error("cmix dictionary temporary file seek failed");
        }
        const long end = std::ftell(file_);
        if (end < 0 || static_cast<std::size_t>(end) > maximum_size ||
            std::fseek(file_, 0, SEEK_SET) != 0) {
            throw std::runtime_error("cmix dictionary transform size is invalid");
        }
        std::vector<std::uint8_t> bytes(static_cast<std::size_t>(end));
        if (!bytes.empty() &&
            std::fread(bytes.data(), 1U, bytes.size(), file_) != bytes.size()) {
            throw std::runtime_error("cmix dictionary temporary file read failed");
        }
        return bytes;
    }

private:
    std::FILE* file_ = nullptr;
};

void seed_dictionary(TemporaryFile& dictionary) {
    dictionary.write(ByteView(cmix_dictionary_data(), cmix_dictionary_size()));
}

}  // namespace

bool CmixWordDictionaryTransform::applicable(const ByteView input) const noexcept {
    return !input.empty();
}

TransformResult CmixWordDictionaryTransform::forward(const ByteView input) const {
    if (!applicable(input)) {
        throw std::invalid_argument("cmix dictionary requires a non-empty block");
    }
    TemporaryFile dictionary;
    seed_dictionary(dictionary);
    TemporaryFile source;
    TemporaryFile transformed;
    source.write(input);

    preprocessor::Dictionary donor(dictionary.get(), true, false);
    donor.Encode(source.get(), static_cast<int>(input.size()), transformed.get());

    TransformResult result{};
    result.bytes = transformed.read_all(maximum_transformed_size(input.size()));
    if (result.bytes.empty()) {
        throw std::runtime_error("cmix dictionary produced an empty transform");
    }
    return result;
}

std::vector<std::uint8_t> CmixWordDictionaryTransform::inverse(
    const ByteView transformed, const std::size_t expected_size) const {
    if (transformed.empty() || expected_size == 0 ||
        transformed.size() > maximum_transformed_size(expected_size)) {
        throw std::runtime_error("cmix dictionary transform violates HZ02 bounds");
    }
    TemporaryFile dictionary;
    seed_dictionary(dictionary);
    TemporaryFile source;
    source.write(transformed);

    preprocessor::Dictionary donor(dictionary.get(), false, true);
    std::vector<std::uint8_t> decoded;
    decoded.reserve(expected_size);
    for (std::size_t index = 0; index < expected_size; ++index) {
        const unsigned char value = donor.Decode(source.get());
        if (std::ferror(source.get()) != 0 || std::feof(source.get()) != 0) {
            throw std::runtime_error("cmix dictionary transform is truncated");
        }
        decoded.push_back(value);
    }

    if (donor.HasPendingOutput() || std::ferror(source.get()) != 0 ||
        std::fgetc(source.get()) != EOF || std::ferror(source.get()) != 0 ||
        std::feof(source.get()) == 0) {
        throw std::runtime_error(
            "cmix dictionary transform has trailing input or output");
    }
    return decoded;
}

std::size_t CmixWordDictionaryTransform::maximum_transformed_size(
    const std::size_t input_size) {
    if (input_size > std::numeric_limits<std::size_t>::max() / 2U) {
        throw std::runtime_error("cmix dictionary transform bound overflow");
    }
    return input_size * 2U;
}

}  // namespace hz::r2
