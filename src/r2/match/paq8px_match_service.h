#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "r2/match/match_service.h"

namespace hz::paq8px {
class MatchCore;
}

namespace hz::r2 {

class Paq8pxMatchService final : public IMatchService {
public:
    static constexpr std::uint8_t kDefaultHashBits = 20;
    static constexpr std::size_t kMaximumCandidates = 4;

    explicit Paq8pxMatchService(
        std::uint8_t hash_bits = kDefaultHashBits);
    ~Paq8pxMatchService() override;

    Paq8pxMatchService(const Paq8pxMatchService&) = delete;
    Paq8pxMatchService& operator=(const Paq8pxMatchService&) = delete;
    Paq8pxMatchService(Paq8pxMatchService&&) noexcept;
    Paq8pxMatchService& operator=(Paq8pxMatchService&&) noexcept;

    const char* name() const noexcept override;
    void reset(std::uint64_t seed) override;
    std::vector<MatchCandidate> find(
        ByteView history,
        std::size_t position,
        std::size_t maximum_candidates) override;

    std::size_t allocation_bytes() const noexcept;
    std::size_t position() const noexcept { return position_; }
    const paq8px::MatchCore& core() const noexcept;

private:
    std::vector<MatchCandidate> truncate_cache(
        std::size_t maximum_candidates) const;

    std::unique_ptr<paq8px::MatchCore> core_;
    std::size_t position_ = 0;
    std::vector<MatchCandidate> cached_candidates_;
};

}  // namespace hz::r2
