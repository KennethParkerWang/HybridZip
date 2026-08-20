#include "r2/representation/kanzi_rlt_transform.h"

#include <limits>
#include <stdexcept>

#include "SliceArray.hpp"
#include "transform/RLT.hpp"

namespace hz::r2 {
std::optional<std::vector<std::uint8_t>> KanziRltTransform::forward_if_smaller(ByteView input) const {
    if (input.size() < 16 || input.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) return std::nullopt;
    std::vector<std::uint8_t> output(input.size());
    auto* source = reinterpret_cast<kanzi::byte*>(const_cast<std::uint8_t*>(input.data()));
    kanzi::SliceArray<kanzi::byte> src(source, static_cast<int>(input.size()));
    kanzi::SliceArray<kanzi::byte> dst(reinterpret_cast<kanzi::byte*>(output.data()), static_cast<int>(output.size()));
    kanzi::RLT rlt;
    if (!rlt.forward(src, dst, static_cast<int>(input.size())) || src._index != static_cast<int>(input.size()) || dst._index >= src._index) return std::nullopt;
    output.resize(static_cast<std::size_t>(dst._index));
    return output;
}

std::vector<std::uint8_t> KanziRltTransform::inverse(ByteView input, std::size_t output_size) const {
    if (input.empty() || output_size == 0 || output_size > static_cast<std::size_t>(std::numeric_limits<int>::max())) throw std::invalid_argument("Invalid Kanzi RLT sizes");
    std::vector<std::uint8_t> output(output_size);
    auto* source = reinterpret_cast<kanzi::byte*>(const_cast<std::uint8_t*>(input.data()));
    kanzi::SliceArray<kanzi::byte> src(source, static_cast<int>(input.size()));
    kanzi::SliceArray<kanzi::byte> dst(reinterpret_cast<kanzi::byte*>(output.data()), static_cast<int>(output.size()));
    kanzi::RLT rlt;
    if (!rlt.inverse(src, dst, static_cast<int>(input.size())) || src._index != static_cast<int>(input.size()) || dst._index != static_cast<int>(output.size())) throw std::runtime_error("Kanzi RLT inverse failed");
    return output;
}
}  // namespace hz::r2
