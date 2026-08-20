#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/experts/expert.h"
#include "r2/match/paq8px_match_service.h"

namespace hz::r2 {

class Paq8pxMatchExpert final : public IExpert {
public:
    explicit Paq8pxMatchExpert(
        std::uint8_t hash_bits = Paq8pxMatchService::kDefaultHashBits);

    const char* name() const noexcept override;
    ExpertEvidence predict(const ExpertContext& context) override;
    void observe(std::uint8_t actual,
                 const ExpertContext& context) override;
    void reset_block(const ExpertContext& context) override;

    std::size_t allocation_bytes() const noexcept {
        return service_.allocation_bytes();
    }
    std::size_t prefix_size() const noexcept { return history_.size(); }

private:
    static bool same_context(const ExpertContext& left,
                             const ExpertContext& right) noexcept;

    Paq8pxMatchService service_;
    std::vector<std::uint8_t> history_;
    bool prediction_pending_ = false;
    ExpertContext predicted_context_{};
};

}  // namespace hz::r2
