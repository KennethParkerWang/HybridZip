#pragma once

#include "r2/core/byte_view.h"
#include "r2/representation/transform.h"

namespace hz::r2 {

// Deterministic, input-only features used before representation selection.
class StructureAnalyzer {
public:
    StructureFeatures analyze(ByteView input) const;
};

}  // namespace hz::r2
