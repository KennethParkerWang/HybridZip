#pragma once

#include <cstdint>
#include <memory>

#include "r2/experts/expert.h"

namespace hz::cmix {
class MatchCore;
struct MatchConfig;
}  // namespace hz::cmix

namespace hz::r2 {

class CmixMatchExpert final : public IExpert {
public:
    static constexpr std::uint32_t kProbabilityScale = 1U << 24U;

    CmixMatchExpert();
    explicit CmixMatchExpert(const cmix::MatchConfig& config);
    ~CmixMatchExpert() override;

    CmixMatchExpert(const CmixMatchExpert&) = delete;
    CmixMatchExpert& operator=(const CmixMatchExpert&) = delete;

    const char* name() const noexcept override;
    ExpertEvidence predict(const ExpertContext& context) override;
    void observe(std::uint8_t actual,
                 const ExpertContext& context) override;
    void reset_block(const ExpertContext& context) override;

    bool prediction_pending() const noexcept;
    const cmix::MatchCore& core() const noexcept;

private:
    static bool same_context(const ExpertContext& left,
                             const ExpertContext& right) noexcept;

    std::unique_ptr<cmix::MatchCore> core_;
    bool prediction_pending_ = false;
    ExpertContext predicted_context_{};
};

}  // namespace hz::r2
