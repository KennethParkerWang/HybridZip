#include "r2/representation/flac_residual_transform.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "r2/archive/r2_archive.h"

extern "C" {
#include "private/fixed.h"
#include "private/lpc.h"
}

namespace hz::r2 {
namespace {

constexpr std::uint8_t kFormatVersion = 1;
constexpr std::uint8_t kPcmBitsPerSample = 16;
constexpr std::uint8_t kAssignmentIndependent = 0;
constexpr std::uint8_t kAssignmentMidSide = 1;
constexpr std::uint8_t kFixedPredictor = 0;
constexpr std::uint8_t kLpcPredictor = 1;
constexpr std::uint8_t kMaximumLpcOrder = 8;
constexpr std::uint8_t kMaximumRiceParameter = 30;
constexpr std::size_t kHeaderSize = 8;

struct BitWriter {
    void write_bits(const std::uint32_t value, const std::uint8_t count) {
        if (count > 32U) {
            throw std::runtime_error("FLAC residual bit count is invalid");
        }
        for (std::uint8_t index = count; index != 0U; --index) {
            const std::uint8_t bit = static_cast<std::uint8_t>(
                (value >> (index - 1U)) & 1U);
            if ((bit_count_ & 7U) == 0U) {
                bytes_.push_back(0U);
            }
            bytes_.back() |= static_cast<std::uint8_t>(
                bit << (7U - static_cast<std::uint8_t>(bit_count_ & 7U)));
            ++bit_count_;
        }
    }

    void write_zeroes(const std::uint32_t count) {
        for (std::uint32_t index = 0; index < count; ++index) {
            write_bits(0U, 1U);
        }
    }

    std::uint32_t bit_count() const noexcept { return bit_count_; }
    const std::vector<std::uint8_t>& bytes() const noexcept { return bytes_; }

private:
    std::vector<std::uint8_t> bytes_;
    std::uint32_t bit_count_ = 0;
};

class BitReader final {
public:
    BitReader(const ByteView input, const std::uint32_t bit_count)
        : input_(input), bit_count_(bit_count) {
        const std::size_t expected_bytes =
            (static_cast<std::size_t>(bit_count_) + 7U) / 8U;
        if (expected_bytes != input_.size()) {
            throw std::runtime_error("FLAC Rice payload length is malformed");
        }
    }

    std::uint32_t read_bits(const std::uint8_t count) {
        if (count > 32U || count > remaining()) {
            throw std::runtime_error("FLAC Rice payload is truncated");
        }
        std::uint32_t value = 0;
        for (std::uint8_t index = 0; index < count; ++index) {
            const std::size_t byte_offset = bit_position_ / 8U;
            const std::uint8_t shift = static_cast<std::uint8_t>(
                7U - (bit_position_ & 7U));
            value = (value << 1U) | ((input_[byte_offset] >> shift) & 1U);
            ++bit_position_;
        }
        return value;
    }

    void require_complete() const {
        if (bit_position_ != bit_count_) {
            throw std::runtime_error("FLAC Rice payload has unconsumed bits");
        }
        if ((bit_count_ & 7U) != 0U && !input_.empty()) {
            const std::uint8_t unused = static_cast<std::uint8_t>(
                8U - (bit_count_ & 7U));
            const std::uint8_t mask = static_cast<std::uint8_t>((1U << unused) - 1U);
            if ((input_[input_.size() - 1U] & mask) != 0U) {
                throw std::runtime_error("FLAC Rice padding is nonzero");
            }
        }
    }

private:
    std::uint32_t remaining() const noexcept { return bit_count_ - bit_position_; }

    ByteView input_;
    std::uint32_t bit_count_ = 0;
    std::uint32_t bit_position_ = 0;
};

struct ChannelModel {
    std::uint8_t kind = kFixedPredictor;
    std::uint8_t order = 0;
    std::uint8_t shift = 0;
    std::uint8_t rice_parameter = 0;
    std::vector<std::int32_t> coefficients;
    std::vector<std::int32_t> warmup;
    std::vector<std::int32_t> residual;
    std::vector<std::uint8_t> rice_bytes;
    std::uint32_t rice_bits = 0;
};

void append_u32_le(std::vector<std::uint8_t>& output,
                   const std::uint32_t value) {
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        output.push_back(static_cast<std::uint8_t>(value >> shift));
    }
}

void append_i16_le(std::vector<std::uint8_t>& output,
                   const std::int16_t value) {
    const std::uint16_t bits = static_cast<std::uint16_t>(value);
    output.push_back(static_cast<std::uint8_t>(bits));
    output.push_back(static_cast<std::uint8_t>(bits >> 8U));
}

void append_i32_le(std::vector<std::uint8_t>& output,
                   const std::int32_t value) {
    append_u32_le(output, static_cast<std::uint32_t>(value));
}

std::uint32_t read_u32_le(const ByteView input, const std::size_t offset) {
    if (offset > input.size() || input.size() - offset < 4U) {
        throw std::runtime_error("FLAC residual metadata is truncated");
    }
    return static_cast<std::uint32_t>(input[offset]) |
           (static_cast<std::uint32_t>(input[offset + 1U]) << 8U) |
           (static_cast<std::uint32_t>(input[offset + 2U]) << 16U) |
           (static_cast<std::uint32_t>(input[offset + 3U]) << 24U);
}

std::int32_t read_i32_le(const ByteView input, const std::size_t offset) {
    return static_cast<std::int32_t>(read_u32_le(input, offset));
}

std::uint32_t zigzag_encode(const std::int32_t value) noexcept {
    const std::int64_t widened = value;
    return static_cast<std::uint32_t>(
        widened >= 0 ? widened * 2 : -widened * 2 - 1);
}

std::int32_t zigzag_decode(const std::uint32_t value) noexcept {
    const std::int64_t decoded = (value & 1U) == 0U
        ? static_cast<std::int64_t>(value >> 1U)
        : -static_cast<std::int64_t>((value >> 1U) + 1U);
    return static_cast<std::int32_t>(decoded);
}

std::uint64_t rice_bit_count(const std::vector<std::int32_t>& residual,
                             const std::uint8_t parameter) noexcept {
    std::uint64_t total = 0;
    for (const std::int32_t value : residual) {
        const std::uint32_t code = zigzag_encode(value);
        total += static_cast<std::uint64_t>(code >> parameter) + 1U + parameter;
    }
    return total;
}

void encode_rice(ChannelModel& model) {
    std::uint8_t best_parameter = 0;
    std::uint64_t best_bits = std::numeric_limits<std::uint64_t>::max();
    for (std::uint8_t parameter = 0; parameter <= kMaximumRiceParameter;
         ++parameter) {
        const std::uint64_t bits = rice_bit_count(model.residual, parameter);
        if (bits < best_bits) {
            best_bits = bits;
            best_parameter = parameter;
        }
    }
    if (best_bits > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("FLAC Rice stream exceeds HZ02 framing limits");
    }

    BitWriter writer;
    for (const std::int32_t value : model.residual) {
        const std::uint32_t code = zigzag_encode(value);
        writer.write_zeroes(code >> best_parameter);
        writer.write_bits(1U, 1U);
        if (best_parameter != 0U) {
            writer.write_bits(code & ((1U << best_parameter) - 1U),
                              best_parameter);
        }
    }
    if (writer.bit_count() != best_bits) {
        throw std::runtime_error("FLAC Rice bit accounting disagrees with writer");
    }
    model.rice_parameter = best_parameter;
    model.rice_bits = static_cast<std::uint32_t>(best_bits);
    model.rice_bytes = writer.bytes();
}

std::vector<std::int32_t> decode_rice(const ByteView payload,
                                      const std::uint32_t bit_count,
                                      const std::uint8_t parameter,
                                      const std::size_t value_count) {
    if (parameter > kMaximumRiceParameter) {
        throw std::runtime_error("FLAC Rice parameter is invalid");
    }
    BitReader reader(payload, bit_count);
    std::vector<std::int32_t> residual;
    residual.reserve(value_count);
    for (std::size_t index = 0; index < value_count; ++index) {
        std::uint32_t quotient = 0;
        while (reader.read_bits(1U) == 0U) {
            if (quotient == (std::numeric_limits<std::uint32_t>::max() >> parameter)) {
                throw std::runtime_error("FLAC Rice quotient exceeds the supported bound");
            }
            ++quotient;
        }
        const std::uint32_t remainder = parameter == 0U
            ? 0U : reader.read_bits(parameter);
        residual.push_back(zigzag_decode((quotient << parameter) | remainder));
    }
    reader.require_complete();
    return residual;
}

std::vector<std::int32_t> padded_samples(const std::vector<std::int32_t>& samples,
                                         const std::uint8_t history) {
    std::vector<std::int32_t> padded(4U + samples.size(), 0);
    if (history > 4U) {
        throw std::runtime_error("FLAC fixed history is invalid");
    }
    std::copy(samples.begin(), samples.end(), padded.begin() + 4U);
    return padded;
}

ChannelModel make_fixed_model(const std::vector<std::int32_t>& samples) {
    if (samples.empty()) {
        throw std::runtime_error("FLAC channel has no samples");
    }
    std::vector<std::int32_t> padded = padded_samples(samples, 4U);
    std::array<float, FLAC__MAX_FIXED_ORDER + 1U> bits{};
    const std::uint32_t selected_order = FLAC__fixed_compute_best_predictor_wide(
        padded.data() + 4U, static_cast<std::uint32_t>(samples.size()), bits.data());
    if (selected_order > FLAC__MAX_FIXED_ORDER || selected_order > samples.size()) {
        throw std::runtime_error("libFLAC selected an invalid fixed predictor");
    }

    ChannelModel model{};
    model.kind = kFixedPredictor;
    model.order = static_cast<std::uint8_t>(selected_order);
    model.warmup.assign(samples.begin(), samples.begin() + selected_order);
    model.residual.resize(samples.size() - selected_order);
    FLAC__fixed_compute_residual_wide(
        padded.data() + 4U + selected_order,
        static_cast<std::uint32_t>(model.residual.size()), selected_order,
        model.residual.data());
    encode_rice(model);
    return model;
}

std::optional<ChannelModel> make_lpc_model(
    const std::vector<std::int32_t>& samples) {
    if (samples.size() <= kMaximumLpcOrder) {
        return std::nullopt;
    }
    std::vector<FLAC__real> floating(samples.begin(), samples.end());
    std::array<double, kMaximumLpcOrder + 1U> autocorrelation{};
    FLAC__lpc_compute_autocorrelation(floating.data(),
                                      static_cast<std::uint32_t>(floating.size()),
                                      kMaximumLpcOrder + 1U,
                                      autocorrelation.data());
    if (autocorrelation[0] <= 0.0) {
        return std::nullopt;
    }
    std::array<std::array<FLAC__real, FLAC__MAX_LPC_ORDER>,
               FLAC__MAX_LPC_ORDER> coefficients{};
    std::array<double, FLAC__MAX_LPC_ORDER> errors{};
    std::uint32_t max_order = kMaximumLpcOrder;
    FLAC__lpc_compute_lp_coefficients(
        autocorrelation.data(), &max_order,
        reinterpret_cast<FLAC__real (*)[FLAC__MAX_LPC_ORDER]>(coefficients.data()),
        errors.data());

    std::optional<ChannelModel> best;
    for (std::uint32_t order = 1; order <= max_order; ++order) {
        ChannelModel model{};
        model.kind = kLpcPredictor;
        model.order = static_cast<std::uint8_t>(order);
        model.coefficients.resize(order);
        int shift = 0;
        if (FLAC__lpc_quantize_coefficients(coefficients[order - 1U].data(), order,
                                             12U, model.coefficients.data(),
                                             &shift) != 0 ||
            shift < 0 || shift > 31) {
            continue;
        }
        model.shift = static_cast<std::uint8_t>(shift);
        model.warmup.assign(samples.begin(), samples.begin() + order);
        model.residual.resize(samples.size() - order);
        std::vector<std::int32_t> padded = padded_samples(samples, 4U);
        FLAC__lpc_compute_residual_from_qlp_coefficients(
            padded.data() + 4U + order,
            static_cast<std::uint32_t>(model.residual.size()),
            model.coefficients.data(), order, shift, model.residual.data());
        encode_rice(model);
        if (!best.has_value() || model.rice_bytes.size() < best->rice_bytes.size() ||
            (model.rice_bytes.size() == best->rice_bytes.size() &&
             model.rice_bits < best->rice_bits)) {
            best = std::move(model);
        }
    }
    return best;
}

ChannelModel choose_channel_model(const std::vector<std::int32_t>& samples) {
    ChannelModel best = make_fixed_model(samples);
    const std::optional<ChannelModel> lpc = make_lpc_model(samples);
    if (lpc.has_value() &&
        (lpc->rice_bytes.size() < best.rice_bytes.size() ||
         (lpc->rice_bytes.size() == best.rice_bytes.size() &&
          lpc->rice_bits < best.rice_bits))) {
        best = *lpc;
    }
    return best;
}

std::int32_t floor_divide_by_two(const std::int32_t value) noexcept {
    return value >= 0 ? value / 2 : -static_cast<std::int32_t>((-
        static_cast<std::int64_t>(value) + 1) / 2);
}

struct EncodedCandidate {
    std::uint8_t channels = 0;
    std::uint8_t assignment = kAssignmentIndependent;
    std::uint32_t frames = 0;
    std::vector<ChannelModel> models;
    std::vector<std::uint8_t> payload;
    std::vector<std::uint8_t> metadata;
};

EncodedCandidate encode_planes(const std::vector<std::vector<std::int32_t>>& planes,
                               const std::uint8_t assignment) {
    if (planes.empty() || planes.size() > 2U || planes[0].empty()) {
        throw std::runtime_error("FLAC PCM channel geometry is invalid");
    }
    const std::size_t frames = planes[0].size();
    if (frames > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("FLAC PCM frame count exceeds HZ02 limits");
    }
    EncodedCandidate candidate{};
    candidate.channels = static_cast<std::uint8_t>(planes.size());
    candidate.assignment = assignment;
    candidate.frames = static_cast<std::uint32_t>(frames);
    candidate.metadata = {kFormatVersion, candidate.channels, assignment,
                          kPcmBitsPerSample};
    append_u32_le(candidate.metadata, candidate.frames);

    for (const auto& plane : planes) {
        if (plane.size() != frames) {
            throw std::runtime_error("FLAC PCM planes have unequal lengths");
        }
        ChannelModel model = choose_channel_model(plane);
        candidate.metadata.push_back(model.kind);
        candidate.metadata.push_back(model.order);
        candidate.metadata.push_back(model.shift);
        candidate.metadata.push_back(model.rice_parameter);
        append_u32_le(candidate.metadata, model.rice_bits);
        for (const std::int32_t sample : model.warmup) {
            append_i32_le(candidate.metadata, sample);
        }
        if (model.kind == kLpcPredictor) {
            for (const std::int32_t coefficient : model.coefficients) {
                append_i32_le(candidate.metadata, coefficient);
            }
        }
        candidate.payload.insert(candidate.payload.end(), model.rice_bytes.begin(),
                                 model.rice_bytes.end());
        candidate.models.push_back(std::move(model));
    }
    return candidate;
}

std::vector<std::int32_t> decode_plane(const ChannelModel& model,
                                       const std::vector<std::int32_t>& residual,
                                       const std::uint32_t frames) {
    if (model.warmup.size() != model.order || residual.size() != frames - model.order) {
        throw std::runtime_error("FLAC residual model geometry is invalid");
    }
    std::vector<std::int32_t> padded(4U + frames, 0);
    for (std::size_t index = 0; index < model.warmup.size(); ++index) {
        padded[4U + index] = model.warmup[index];
    }
    std::int32_t* const output = padded.data() + 4U + model.order;
    if (model.kind == kFixedPredictor) {
        FLAC__fixed_restore_signal_wide(residual.data(),
                                        static_cast<std::uint32_t>(residual.size()),
                                        model.order, output);
    } else if (model.kind == kLpcPredictor) {
        FLAC__lpc_restore_signal(residual.data(),
                                 static_cast<std::uint32_t>(residual.size()),
                                 model.coefficients.data(), model.order,
                                 model.shift, output);
    } else {
        throw std::runtime_error("FLAC predictor kind is invalid");
    }
    std::vector<std::int32_t> result(padded.begin() + 4U, padded.end());
    return result;
}

}  // namespace

bool FlacResidualTransform::applicable(const ByteView input) const noexcept {
    return input.size() >= 64U && input.size() <= kR2MaximumBlockSize &&
           (input.size() & 1U) == 0U;
}

TransformResult FlacResidualTransform::forward(const ByteView input) const {
    if (!applicable(input)) {
        throw std::invalid_argument(
            "FLAC residual requires at least 32 signed 16-bit PCM samples");
    }
    std::vector<std::int32_t> mono;
    mono.reserve(input.size() / 2U);
    for (std::size_t offset = 0; offset < input.size(); offset += 2U) {
        const std::uint16_t raw = static_cast<std::uint16_t>(input[offset]) |
            (static_cast<std::uint16_t>(input[offset + 1U]) << 8U);
        mono.push_back(static_cast<std::int16_t>(raw));
    }
    EncodedCandidate best = encode_planes({mono}, kAssignmentIndependent);

    if ((input.size() & 3U) == 0U) {
        std::vector<std::int32_t> left;
        std::vector<std::int32_t> right;
        left.reserve(input.size() / 4U);
        right.reserve(input.size() / 4U);
        for (std::size_t offset = 0; offset < input.size(); offset += 4U) {
            const std::uint16_t left_raw = static_cast<std::uint16_t>(input[offset]) |
                (static_cast<std::uint16_t>(input[offset + 1U]) << 8U);
            const std::uint16_t right_raw = static_cast<std::uint16_t>(input[offset + 2U]) |
                (static_cast<std::uint16_t>(input[offset + 3U]) << 8U);
            left.push_back(static_cast<std::int16_t>(left_raw));
            right.push_back(static_cast<std::int16_t>(right_raw));
        }
        EncodedCandidate independent = encode_planes({left, right}, kAssignmentIndependent);
        std::vector<std::int32_t> mid(left.size());
        std::vector<std::int32_t> side(left.size());
        for (std::size_t index = 0; index < left.size(); ++index) {
            mid[index] = floor_divide_by_two(left[index] + right[index]);
            side[index] = left[index] - right[index];
        }
        EncodedCandidate mid_side = encode_planes({mid, side}, kAssignmentMidSide);
        if (independent.payload.size() + independent.metadata.size() <
            best.payload.size() + best.metadata.size()) {
            best = std::move(independent);
        }
        if (mid_side.payload.size() + mid_side.metadata.size() <
            best.payload.size() + best.metadata.size()) {
            best = std::move(mid_side);
        }
    }
    if (best.payload.empty() || best.payload.size() > maximum_payload_size(input.size())) {
        throw std::runtime_error("FLAC residual payload exceeds HZ02 framing bounds");
    }
    return TransformResult{std::move(best.payload), std::move(best.metadata)};
}

std::vector<std::uint8_t> FlacResidualTransform::inverse(
    const ByteView payload, const ByteView side_information,
    const std::size_t expected_size) const {
    if (side_information.size() < kHeaderSize || side_information[0] != kFormatVersion ||
        side_information[3] != kPcmBitsPerSample || expected_size < 64U ||
        (expected_size & 1U) != 0U) {
        throw std::runtime_error("FLAC residual metadata is malformed");
    }
    const std::uint8_t channels = side_information[1];
    const std::uint8_t assignment = side_information[2];
    const std::uint32_t frames = read_u32_le(side_information, 4U);
    if ((channels != 1U && channels != 2U) ||
        (channels == 1U && assignment != kAssignmentIndependent) ||
        (channels == 2U && assignment != kAssignmentIndependent &&
         assignment != kAssignmentMidSide) ||
        frames == 0U || static_cast<std::size_t>(frames) * channels * 2U != expected_size ||
        payload.empty() || payload.size() > maximum_payload_size(expected_size)) {
        throw std::runtime_error("FLAC residual PCM geometry is invalid");
    }

    std::size_t metadata_offset = kHeaderSize;
    std::size_t payload_offset = 0;
    std::vector<std::vector<std::int32_t>> planes;
    planes.reserve(channels);
    for (std::uint8_t channel = 0; channel < channels; ++channel) {
        if (metadata_offset > side_information.size() ||
            side_information.size() - metadata_offset < 8U) {
            throw std::runtime_error("FLAC residual channel metadata is truncated");
        }
        ChannelModel model{};
        model.kind = side_information[metadata_offset++];
        model.order = side_information[metadata_offset++];
        model.shift = side_information[metadata_offset++];
        model.rice_parameter = side_information[metadata_offset++];
        model.rice_bits = read_u32_le(side_information, metadata_offset);
        metadata_offset += 4U;
        const bool fixed = model.kind == kFixedPredictor;
        const bool lpc = model.kind == kLpcPredictor;
        if ((!fixed && !lpc) || (fixed && model.order > FLAC__MAX_FIXED_ORDER) ||
            (lpc && (model.order == 0U || model.order > kMaximumLpcOrder)) ||
            (fixed && model.shift != 0U) || (lpc && model.shift > 31U) ||
            model.order > frames || model.rice_parameter > kMaximumRiceParameter) {
            throw std::runtime_error("FLAC residual predictor metadata is invalid");
        }
        const std::size_t warmup_bytes = static_cast<std::size_t>(model.order) * 4U;
        const std::size_t coefficient_bytes = lpc
            ? static_cast<std::size_t>(model.order) * 4U : 0U;
        if (metadata_offset > side_information.size() ||
            side_information.size() - metadata_offset < warmup_bytes + coefficient_bytes) {
            throw std::runtime_error("FLAC residual warmup metadata is truncated");
        }
        model.warmup.reserve(model.order);
        for (std::uint8_t index = 0; index < model.order; ++index) {
            model.warmup.push_back(read_i32_le(side_information, metadata_offset));
            metadata_offset += 4U;
        }
        if (lpc) {
            model.coefficients.reserve(model.order);
            for (std::uint8_t index = 0; index < model.order; ++index) {
                model.coefficients.push_back(read_i32_le(side_information, metadata_offset));
                metadata_offset += 4U;
            }
        }
        const std::size_t rice_bytes =
            (static_cast<std::size_t>(model.rice_bits) + 7U) / 8U;
        if (rice_bytes > payload.size() - payload_offset) {
            throw std::runtime_error("FLAC Rice stream exceeds payload bounds");
        }
        const std::vector<std::int32_t> residual = decode_rice(
            ByteView(payload.data() + payload_offset, rice_bytes), model.rice_bits,
            model.rice_parameter, static_cast<std::size_t>(frames) - model.order);
        payload_offset += rice_bytes;
        planes.push_back(decode_plane(model, residual, frames));
    }
    if (metadata_offset != side_information.size() || payload_offset != payload.size()) {
        throw std::runtime_error("FLAC residual framing has trailing bytes");
    }

    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    for (std::size_t frame = 0; frame < frames; ++frame) {
        std::int32_t first = planes[0][frame];
        std::int32_t second = channels == 2U ? planes[1][frame] : 0;
        if (assignment == kAssignmentMidSide) {
            first += (second + (second & 1)) / 2;
            second = first - second;
        }
        if (first < std::numeric_limits<std::int16_t>::min() ||
            first > std::numeric_limits<std::int16_t>::max() ||
            (channels == 2U && (second < std::numeric_limits<std::int16_t>::min() ||
                                second > std::numeric_limits<std::int16_t>::max()))) {
            throw std::runtime_error("FLAC channel decorrelation exceeds 16-bit PCM");
        }
        append_i16_le(output, static_cast<std::int16_t>(first));
        if (channels == 2U) {
            append_i16_le(output, static_cast<std::int16_t>(second));
        }
    }
    return output;
}

std::size_t FlacResidualTransform::maximum_payload_size(
    const std::size_t input_size) {
    if (input_size > (std::numeric_limits<std::size_t>::max() - 4096U) / 18U) {
        throw std::runtime_error("FLAC residual payload bound overflow");
    }
    return input_size * 18U + 4096U;
}

}  // namespace hz::r2
