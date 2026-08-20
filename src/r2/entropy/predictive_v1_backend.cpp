#include "r2/entropy/predictive_v1_backend.h"

#include <sstream>
#include <string>

#include "codec/model_pipeline.h"
#include "core/profile.h"
#include "entropy/arithmetic_codec.h"

namespace hz::r2 {

std::vector<std::uint8_t> PredictiveV1Backend::encode(
    const ByteView input) const {
    const Profile profile = make_profile_v1();
    ModelPipeline pipeline(profile);
    pipeline.reset(model_seed_);

    std::ostringstream encoded(std::ios::out | std::ios::binary);
    {
        ArithmeticEncoderStream coder(encoded);
        for (std::size_t index = 0; index < input.size(); ++index) {
            const Cdf& cdf = pipeline.predict_cdf();
            coder.write_symbol(cdf, input[index]);
            pipeline.observe(input[index]);
        }
        coder.finish();
    }

    const std::string bytes = encoded.str();
    return {reinterpret_cast<const std::uint8_t*>(bytes.data()),
            reinterpret_cast<const std::uint8_t*>(bytes.data()) + bytes.size()};
}

std::vector<std::uint8_t> PredictiveV1Backend::decode(
    const ByteView payload,
    const std::size_t expected_size) const {
    const std::string bytes(
        reinterpret_cast<const char*>(payload.data()), payload.size());
    std::istringstream encoded(bytes, std::ios::in | std::ios::binary);

    const Profile profile = make_profile_v1();
    ModelPipeline pipeline(profile);
    pipeline.reset(model_seed_);
    ArithmeticDecoderStream coder(encoded);

    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    for (std::size_t index = 0; index < expected_size; ++index) {
        const Cdf& cdf = pipeline.predict_cdf();
        const std::uint8_t symbol = coder.read_symbol(cdf);
        output.push_back(symbol);
        pipeline.observe(symbol);
    }
    return output;
}

}  // namespace hz::r2

