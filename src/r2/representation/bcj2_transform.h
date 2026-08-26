#pragma once

#include <cstddef>
#include <vector>

#include "r2/representation/transform.h"

namespace hz::r2 {

class Bcj2Transform final {
public:
    static constexpr std::size_t kSideInformationSize = 16;

    TransformResult forward(ByteView input) const;
    std::vector<std::uint8_t> inverse(ByteView transformed,
                                      ByteView side_information,
                                      std::size_t expected_size) const;
};

}  // namespace hz::r2
