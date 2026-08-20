#pragma once

#include <cstddef>
#include <cstdint>

#include "r2/lz/lz_parse_service.h"

namespace hz::r2 {

struct LzmaMatchFinderConfig {
    std::uint32_t history_size = 65536;
    std::uint32_t maximum_match_length = 273;
    std::uint32_t cut_value = 32;
};

// Direct adapter for the public-domain 7-Zip LzFind binary-tree matcher.
// The service exposes a deterministic greedy parse; it does not claim to be
// the LZMA encoder's optimal parser or entropy stream.
class LzmaMatchFinderService final : public ILzParseService {
public:
    explicit LzmaMatchFinderService(LzmaMatchFinderConfig config = {});

    const char* name() const noexcept override;
    LzParseStability stability() const noexcept override;
    const char* stability_notice() const noexcept override;
    LzParseResult parse(ByteView input) const override;

    const LzmaMatchFinderConfig& config() const noexcept { return config_; }

private:
    LzmaMatchFinderConfig config_;
};

}  // namespace hz::r2
