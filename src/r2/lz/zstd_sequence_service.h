#pragma once

#include <cstddef>

#include "r2/lz/lz_parse_service.h"

namespace hz::r2 {

struct ZstdSequenceConfig {
    int compression_level = 19;
};

// This adapter intentionally exposes zstd 1.6.0's deprecated debug sequence
// collector. It is candidate/instrumentation data, not a production-stable
// parser contract. Every zstd block delimiter is preserved as a sequence.
class ZstdSequenceService final : public ILzParseService {
public:
    explicit ZstdSequenceService(ZstdSequenceConfig config = {});

    const char* name() const noexcept override;
    LzParseStability stability() const noexcept override;
    const char* stability_notice() const noexcept override;
    LzParseResult parse(ByteView input) const override;

    // Diagnostic entry point for exercising the donor's capacity failure.
    // Capacity is a count of zstd sequence records, not a byte count.
    LzParseResult parse_with_sequence_capacity(
        ByteView input,
        std::size_t sequence_capacity) const;

    const ZstdSequenceConfig& config() const noexcept { return config_; }

    static std::size_t sequence_bound(std::size_t input_size) noexcept;
    static int minimum_compression_level() noexcept;
    static int maximum_compression_level() noexcept;

private:
    ZstdSequenceConfig config_;
};

}  // namespace hz::r2
