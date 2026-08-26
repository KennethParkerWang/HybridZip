#include "r2/entropy/bgpt_shared_prior_backend.h"

#include <sstream>
#include <stdexcept>
#include <string>

#include "core/cdf.h"
#include "core/types.h"
#include "entropy/arithmetic_codec.h"
#include "r2/entropy/bgpt_shared_prior_data.h"

namespace hz::r2 {
namespace {

hz::Cdf cdf_for_context(const std::size_t context) {
    if (context >= kBgptSharedPriorContextCount) {
        throw std::logic_error("bGPT shared-prior context is invalid");
    }
    const std::uint16_t* const frequencies =
        bgpt_shared_prior_frequencies() + context * hz::kAlphabet;
    hz::Cdf cdf{};
    for (std::size_t symbol = 0; symbol < hz::kAlphabet; ++symbol) {
        cdf.v[symbol + 1U] = cdf.v[symbol] +
            static_cast<std::uint32_t>(frequencies[symbol]) * 256U;
    }
    hz::validate_cdf(cdf);
    return cdf;
}

std::vector<std::uint8_t> as_bytes(const std::string& value) {
    return {reinterpret_cast<const std::uint8_t*>(value.data()),
            reinterpret_cast<const std::uint8_t*>(value.data()) + value.size()};
}

}  // namespace

std::vector<std::uint8_t> BgptSharedPriorBackend::encode(
    const ByteView input) const {
    std::ostringstream encoded(std::ios::out | std::ios::binary);
    std::size_t context = kBgptSharedPriorStartContext;
    {
        hz::ArithmeticEncoderStream coder(encoded);
        for (std::size_t index = 0; index < input.size(); ++index) {
            coder.write_symbol(cdf_for_context(context), input[index]);
            context = input[index];
        }
        coder.finish();
    }
    return as_bytes(encoded.str());
}

std::vector<std::uint8_t> BgptSharedPriorBackend::decode(
    const ByteView payload, const std::size_t expected_size) const {
    const std::string bytes(reinterpret_cast<const char*>(payload.data()),
                            payload.size());
    std::istringstream encoded(bytes, std::ios::in | std::ios::binary);
    hz::ArithmeticDecoderStream coder(encoded);
    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    std::size_t context = kBgptSharedPriorStartContext;
    for (std::size_t index = 0; index < expected_size; ++index) {
        const std::uint8_t symbol = coder.read_symbol(cdf_for_context(context));
        output.push_back(symbol);
        context = symbol;
    }
    return output;
}

}  // namespace hz::r2
