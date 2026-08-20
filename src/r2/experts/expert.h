#pragma once

#include <cstdint>

#include "r2/core/evidence.h"

namespace hz::r2 {

class IExpert {
public:
    virtual ~IExpert() = default;

    virtual const char* name() const noexcept = 0;
    virtual ExpertEvidence predict(const ExpertContext& context) = 0;
    virtual void observe(std::uint8_t actual,
                         const ExpertContext& context) = 0;
    virtual void reset_block(const ExpertContext& context) = 0;
};

}  // namespace hz::r2

