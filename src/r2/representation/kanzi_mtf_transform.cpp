#include "r2/representation/kanzi_mtf_transform.h"

#include <limits>
#include <stdexcept>

#include "SliceArray.hpp"
#include "transform/SBRT.hpp"

namespace hz::r2 {
namespace {
std::vector<std::uint8_t> apply(ByteView input, bool inverse) {
    if (input.empty() || input.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument("Kanzi MTF requires 1..INT_MAX bytes");
    }
    std::vector<std::uint8_t> output(input.size());
    auto* source = reinterpret_cast<kanzi::byte*>(const_cast<std::uint8_t*>(input.data()));
    auto* destination = reinterpret_cast<kanzi::byte*>(output.data());
    kanzi::SliceArray<kanzi::byte> src(source, static_cast<int>(input.size()));
    kanzi::SliceArray<kanzi::byte> dst(destination, static_cast<int>(output.size()));
    kanzi::SBRT mtf(kanzi::SBRT::MODE_MTF);
    const bool ok = inverse ? mtf.inverse(src, dst, static_cast<int>(input.size()))
                            : mtf.forward(src, dst, static_cast<int>(input.size()));
    if (!ok || src._index != static_cast<int>(input.size()) || dst._index != static_cast<int>(output.size())) {
        throw std::runtime_error("Kanzi SBRT MTF did not consume its full block");
    }
    return output;
}
}  // namespace

bool KanziMtfTransform::applicable(ByteView input, const StructureFeatures&) const {
    return !input.empty() && input.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max());
}
TransformResult KanziMtfTransform::forward(ByteView input) const { return {apply(input, false), {}}; }
std::vector<std::uint8_t> KanziMtfTransform::inverse(ByteView input, ByteView side) const {
    if (!side.empty()) throw std::runtime_error("Kanzi MTF has no side information");
    return apply(input, true);
}
}  // namespace hz::r2
