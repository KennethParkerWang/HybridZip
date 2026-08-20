#include "r2/lz/zstd_sequence_service.h"

#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#define ZSTD_STATIC_LINKING_ONLY
#include "zstd.h"

namespace hz::r2 {
namespace {

struct ContextDeleter {
    void operator()(ZSTD_CCtx* context) const noexcept {
        ZSTD_freeCCtx(context);
    }
};

using ContextPtr = std::unique_ptr<ZSTD_CCtx, ContextDeleter>;

void require_zstd_success(const std::size_t result, const char* operation) {
    if (ZSTD_isError(result) != 0U) {
        throw std::runtime_error(std::string(operation) + ": " +
                                 ZSTD_getErrorName(result));
    }
}

void validate_input(const ByteView input) {
    if (!input.empty() && input.data() == nullptr) {
        throw std::invalid_argument(
            "zstd sequence input has a null data pointer");
    }
    const std::size_t bound = ZSTD_compressBound(input.size());
    if (ZSTD_isError(bound) != 0U) {
        throw std::length_error(
            std::string("zstd sequence input size is unsupported: ") +
            ZSTD_getErrorName(bound));
    }
}

ContextPtr make_context(const int compression_level) {
    ContextPtr context(ZSTD_createCCtx());
    if (!context) {
        throw std::bad_alloc();
    }
    require_zstd_success(
        ZSTD_CCtx_setParameter(context.get(), ZSTD_c_compressionLevel,
                               compression_level),
        "failed to set zstd sequence compression level");
    require_zstd_success(
        ZSTD_CCtx_setParameter(context.get(), ZSTD_c_nbWorkers, 0),
        "failed to force single-threaded zstd sequence generation");
    require_zstd_success(
        ZSTD_CCtx_setParameter(context.get(), ZSTD_c_targetCBlockSize, 0),
        "failed to disable zstd target compressed block sizing");
    return context;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

std::size_t call_generate_sequences(ZSTD_CCtx* context,
                                    ZSTD_Sequence* output,
                                    const std::size_t capacity,
                                    const ByteView input) {
    return ZSTD_generateSequences(context, output, capacity, input.data(),
                                  input.size());
}

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

LzParseResult convert_sequences(const ByteView input,
                                const std::size_t sequence_bound,
                                const std::vector<ZSTD_Sequence>& raw) {
    LzParseResult result{};
    result.input_size = input.size();
    result.sequence_bound = sequence_bound;
    result.raw_sequence_count = raw.size();
    result.block_delimiters_preserved = true;
    result.sequences.reserve(raw.size());

    std::size_t cursor = 0;
    std::size_t block_index = 0;
    for (std::size_t index = 0; index < raw.size(); ++index) {
        const ZSTD_Sequence& source = raw[index];
        if (source.rep > 3U) {
            throw std::logic_error("zstd sequence repcode is outside [0, 3]");
        }
        if (source.litLength > input.size() - cursor) {
            throw std::logic_error(
                "zstd sequence literal length exceeds the input cursor");
        }

        LzParseSequence sequence{};
        sequence.sequence_index = index;
        sequence.block_index = block_index;
        sequence.cursor_begin = cursor;
        sequence.literal_length = source.litLength;
        sequence.match_length = source.matchLength;
        sequence.offset = source.offset;
        sequence.distance = source.offset;
        sequence.rep = source.rep;
        sequence.literal_end = cursor + source.litLength;
        sequence.block_delimiter =
            source.offset == 0U && source.matchLength == 0U;

        if (sequence.block_delimiter) {
            sequence.match_source_begin = sequence.literal_end;
            sequence.match_end = sequence.literal_end;
            ++result.block_count;
            ++block_index;
        } else {
            if (source.offset == 0U || source.matchLength == 0U) {
                throw std::logic_error(
                    "zstd sequence has an incomplete match description");
            }
            if (source.offset > sequence.literal_end) {
                throw std::logic_error(
                    "zstd sequence match offset precedes available history");
            }
            if (source.matchLength > input.size() - sequence.literal_end) {
                throw std::logic_error(
                    "zstd sequence match length exceeds the input cursor");
            }
            sequence.match_source_begin =
                sequence.literal_end - source.offset;
            sequence.match_end = sequence.literal_end + source.matchLength;

            for (std::size_t i = 0; i < source.matchLength; ++i) {
                const std::size_t match_source =
                    sequence.match_source_begin + i;
                const std::size_t match_target = sequence.literal_end + i;
                if (input[match_source] != input[match_target]) {
                    throw std::logic_error(
                        "zstd sequence match does not reconstruct the input");
                }
            }
        }

        result.literal_bytes += source.litLength;
        result.match_bytes += source.matchLength;
        cursor = sequence.match_end;
        result.sequences.push_back(sequence);
    }

    if (cursor != input.size()) {
        throw std::logic_error(
            "zstd sequence cursor does not consume the complete input");
    }
    if (!input.empty() &&
        (result.sequences.empty() ||
         !result.sequences.back().block_delimiter)) {
        throw std::logic_error(
            "zstd sequence output lacks a final block delimiter");
    }
    result.consumed_size = cursor;
    return result;
}

}  // namespace

ZstdSequenceService::ZstdSequenceService(const ZstdSequenceConfig config)
    : config_(config) {
    if (config_.compression_level < minimum_compression_level() ||
        config_.compression_level > maximum_compression_level()) {
        throw std::invalid_argument(
            "zstd sequence compression level is outside donor bounds");
    }
}

const char* ZstdSequenceService::name() const noexcept {
    return "zstd-generate-sequences-candidate";
}

LzParseStability ZstdSequenceService::stability() const noexcept {
    return LzParseStability::CandidateInstrumentationOnly;
}

const char* ZstdSequenceService::stability_notice() const noexcept {
    return "zstd 1.6.0 ZSTD_generateSequences is deprecated, debug-only, "
           "not guaranteed to succeed, and not production-stable";
}

LzParseResult ZstdSequenceService::parse(const ByteView input) const {
    validate_input(input);
    return parse_with_sequence_capacity(input, sequence_bound(input.size()));
}

LzParseResult ZstdSequenceService::parse_with_sequence_capacity(
    const ByteView input,
    const std::size_t sequence_capacity) const {
    validate_input(input);
    const std::size_t bound = sequence_bound(input.size());
    if (sequence_capacity > bound) {
        throw std::invalid_argument(
            "zstd sequence capacity exceeds ZSTD_sequenceBound");
    }
    if (sequence_capacity >
        std::vector<ZSTD_Sequence>{}.max_size()) {
        throw std::length_error("zstd sequence capacity is not allocatable");
    }

    std::vector<ZSTD_Sequence> raw(sequence_capacity);
    ContextPtr context = make_context(config_.compression_level);
    const std::size_t generated = call_generate_sequences(
        context.get(), raw.data(), raw.size(), input);
    require_zstd_success(generated,
                         "zstd debug sequence generation failed");
    if (generated > sequence_capacity || generated > bound) {
        throw std::logic_error(
            "zstd sequence count exceeds its declared capacity or bound");
    }
    raw.resize(generated);
    return convert_sequences(input, bound, raw);
}

std::size_t ZstdSequenceService::sequence_bound(
    const std::size_t input_size) noexcept {
    return ZSTD_sequenceBound(input_size);
}

int ZstdSequenceService::minimum_compression_level() noexcept {
    return ZSTD_minCLevel();
}

int ZstdSequenceService::maximum_compression_level() noexcept {
    return ZSTD_maxCLevel();
}

}  // namespace hz::r2
