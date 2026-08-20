#pragma once

#include <cstdint>
#include <vector>

#include "r2/representation/transform.h"

namespace hz::r2 {

struct ExpertTelemetry {
    double recent_log_loss_16 = 0.0;
    double recent_log_loss_256 = 0.0;
    double recent_log_loss_4096 = 0.0;
    double entropy = 0.0;
    double disagreement = 0.0;
    std::uint32_t age = 0;
};

class IActivationRouter {
public:
    virtual ~IActivationRouter() = default;

    virtual std::vector<bool> active_experts(
        const StructureFeatures& structure,
        const std::vector<ExpertTelemetry>& telemetry) = 0;
};

}  // namespace hz::r2

