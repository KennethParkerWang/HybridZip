#include "r2/entropy/zstd_backend.h"

#include <memory>
#include <stdexcept>
#include <string>

#include "zstd.h"

namespace hz::r2 {
namespace {

struct ContextDeleter {
    void operator()(ZSTD_CCtx* context) const noexcept {
        ZSTD_freeCCtx(context);
    }
};

void require_zstd_success(const std::size_t result, const char* operation) {
    if (ZSTD_isError(result) != 0U) {
        throw std::runtime_error(std::string(operation) + ": " +
                                 ZSTD_getErrorName(result));
    }
}

}  // namespace

std::vector<std::uint8_t> ZstdBackend::encode(const ByteView input) const {
    std::vector<std::uint8_t> output(maximum_payload_size(input.size()));
    std::unique_ptr<ZSTD_CCtx, ContextDeleter> context(ZSTD_createCCtx());
    if (!context) {
        throw std::bad_alloc();
    }

    require_zstd_success(
        ZSTD_CCtx_setParameter(context.get(), ZSTD_c_compressionLevel,
                               compression_level_),
        "Failed to set zstd compression level");
    require_zstd_success(
        ZSTD_CCtx_setParameter(context.get(), ZSTD_c_checksumFlag,
                               include_checksum_ ? 1 : 0),
        "Failed to set zstd block checksum flag");
    require_zstd_success(
        ZSTD_CCtx_setParameter(context.get(), ZSTD_c_contentSizeFlag,
                               include_content_size_ ? 1 : 0),
        "Failed to set zstd content size flag");
    require_zstd_success(
        ZSTD_CCtx_setParameter(context.get(), ZSTD_c_dictIDFlag,
                               include_dict_id_ ? 1 : 0),
        "Failed to set zstd dictionary ID flag");
    require_zstd_success(
        ZSTD_CCtx_setParameter(context.get(), ZSTD_c_nbWorkers, 0),
        "Failed to force single-threaded zstd");

    const std::size_t written = ZSTD_compress2(
        context.get(), output.data(), output.size(), input.data(), input.size());
    require_zstd_success(written, "zstd compression failed");
    output.resize(written);
    return output;
}

std::vector<std::uint8_t> ZstdBackend::decode(
    const ByteView payload,
    const std::size_t expected_size) const {
    std::vector<std::uint8_t> output(expected_size);
    const std::size_t written = ZSTD_decompress(
        output.data(), output.size(), payload.data(), payload.size());
    require_zstd_success(written, "zstd decompression failed");
    if (written != expected_size) {
        throw std::runtime_error("zstd output size does not match metadata");
    }
    return output;
}

std::size_t ZstdBackend::maximum_payload_size(const std::size_t input_size) {
    return ZSTD_compressBound(input_size);
}

}  // namespace hz::r2
