#include "r2/entropy/rans_backend.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

#include "rans_byte.h"

namespace hz::r2 {
namespace {

constexpr std::uint8_t kScaleBits = 14;
constexpr std::uint32_t kScale = 1U << kScaleBits;
constexpr std::size_t kPayloadPrefixSize = 3;
constexpr std::size_t kEntrySize = 3;
constexpr std::size_t kStateSize = 4;

struct Model {
    std::array<std::uint16_t, 256> frequencies{};
    std::array<std::uint16_t, 256> cumulative{};
    std::array<RansEncSymbol, 256> encoder_symbols{};
    std::array<RansDecSymbol, 256> decoder_symbols{};
    std::array<std::uint8_t, kScale> lookup{};
    std::vector<std::uint8_t> used_symbols;
};

void append_u16_le(std::vector<std::uint8_t>& output,
                   const std::uint16_t value) {
    output.push_back(static_cast<std::uint8_t>(value));
    output.push_back(static_cast<std::uint8_t>(value >> 8U));
}

std::uint16_t read_u16_le(const ByteView input, const std::size_t offset) {
    return static_cast<std::uint16_t>(input[offset]) |
           (static_cast<std::uint16_t>(input[offset + 1U]) << 8U);
}

Model finish_model(std::array<std::uint16_t, 256> frequencies) {
    Model model{};
    model.frequencies = frequencies;
    std::uint32_t cumulative = 0;
    for (std::size_t symbol = 0; symbol < frequencies.size(); ++symbol) {
        const std::uint16_t frequency = frequencies[symbol];
        model.cumulative[symbol] = static_cast<std::uint16_t>(cumulative);
        if (frequency == 0U) {
            continue;
        }
        if (cumulative + frequency > kScale) {
            throw std::runtime_error("rANS normalized frequencies overflow");
        }
        model.used_symbols.push_back(static_cast<std::uint8_t>(symbol));
        RansEncSymbolInit(&model.encoder_symbols[symbol], cumulative, frequency,
                          kScaleBits);
        RansDecSymbolInit(&model.decoder_symbols[symbol], cumulative, frequency);
        for (std::uint32_t slot = cumulative; slot < cumulative + frequency;
             ++slot) {
            model.lookup[slot] = static_cast<std::uint8_t>(symbol);
        }
        cumulative += frequency;
    }
    if (cumulative != kScale || model.used_symbols.empty()) {
        throw std::runtime_error("rANS normalized frequencies are incomplete");
    }
    return model;
}

Model build_model(const ByteView input) {
    if (input.empty()) {
        throw std::invalid_argument("rANS cannot encode an empty block");
    }

    std::array<std::uint32_t, 256> counts{};
    std::size_t used_count = 0;
    for (std::size_t index = 0; index < input.size(); ++index) {
        if (counts[input[index]]++ == 0U) {
            ++used_count;
        }
    }

    std::array<std::uint16_t, 256> frequencies{};
    std::array<std::uint64_t, 256> remainders{};
    const std::uint32_t remaining = kScale - static_cast<std::uint32_t>(used_count);
    std::uint32_t assigned = 0;
    for (std::size_t symbol = 0; symbol < counts.size(); ++symbol) {
        if (counts[symbol] == 0U) {
            continue;
        }
        const std::uint64_t weighted =
            static_cast<std::uint64_t>(counts[symbol]) * remaining;
        const std::uint32_t share = static_cast<std::uint32_t>(weighted / input.size());
        frequencies[symbol] = static_cast<std::uint16_t>(share + 1U);
        remainders[symbol] = weighted % input.size();
        assigned += share;
    }

    std::uint32_t extras = remaining - assigned;
    while (extras != 0U) {
        std::size_t best = 0;
        for (std::size_t symbol = 1; symbol < remainders.size(); ++symbol) {
            if (remainders[symbol] > remainders[best]) {
                best = symbol;
            }
        }
        ++frequencies[best];
        remainders[best] = 0;
        --extras;
    }
    return finish_model(frequencies);
}

Model parse_model(const ByteView payload, std::size_t& stream_offset) {
    if (payload.size() < kPayloadPrefixSize + kEntrySize + kStateSize ||
        payload[0] != kScaleBits) {
        throw std::runtime_error("Invalid rANS payload header");
    }
    const std::uint16_t symbol_count = read_u16_le(payload, 1U);
    if (symbol_count == 0U || symbol_count > 256U ||
        symbol_count > (payload.size() - kPayloadPrefixSize) / kEntrySize) {
        throw std::runtime_error("Invalid rANS frequency table size");
    }
    stream_offset = kPayloadPrefixSize +
        static_cast<std::size_t>(symbol_count) * kEntrySize;
    if (payload.size() - stream_offset < kStateSize) {
        throw std::runtime_error("Truncated rANS state");
    }

    std::array<std::uint16_t, 256> frequencies{};
    std::uint16_t previous_symbol = 0;
    for (std::uint16_t index = 0; index < symbol_count; ++index) {
        const std::size_t offset = kPayloadPrefixSize +
            static_cast<std::size_t>(index) * kEntrySize;
        const std::uint16_t symbol = payload[offset];
        const std::uint16_t frequency = read_u16_le(payload, offset + 1U);
        if (frequency == 0U || (index != 0U && symbol <= previous_symbol) ||
            frequencies[symbol] != 0U) {
            throw std::runtime_error("Invalid rANS frequency entry");
        }
        frequencies[symbol] = frequency;
        previous_symbol = symbol;
    }
    return finish_model(frequencies);
}

void require_available_renormalization(const RansState state,
                                       const RansDecSymbol& symbol,
                                       const std::uint8_t* cursor,
                                       const std::uint8_t* end) {
    std::uint32_t next = static_cast<std::uint32_t>(
        static_cast<std::uint64_t>(symbol.freq) * (state >> kScaleBits) +
        (state & (kScale - 1U)) - symbol.start);
    while (next < RANS_BYTE_L) {
        if (cursor == end) {
            throw std::runtime_error("Truncated rANS payload");
        }
        next = static_cast<std::uint32_t>((next << 8U) | *cursor++);
    }
}

}  // namespace

std::vector<std::uint8_t> RansBackend::encode(const ByteView input) const {
    const Model model = build_model(input);
    std::vector<std::uint8_t> stream(input.size() + 16U);
    std::uint8_t* cursor = stream.data() + stream.size();
    RansState state{};
    RansEncInit(&state);
    for (std::size_t index = input.size(); index-- != 0U;) {
        RansEncPutSymbol(&state, &cursor, &model.encoder_symbols[input[index]]);
    }
    RansEncFlush(&state, &cursor);

    std::vector<std::uint8_t> payload;
    payload.reserve(kPayloadPrefixSize + model.used_symbols.size() * kEntrySize +
                    static_cast<std::size_t>(stream.data() + stream.size() - cursor));
    payload.push_back(kScaleBits);
    append_u16_le(payload, static_cast<std::uint16_t>(model.used_symbols.size()));
    for (const std::uint8_t symbol : model.used_symbols) {
        payload.push_back(symbol);
        append_u16_le(payload, model.frequencies[symbol]);
    }
    payload.insert(payload.end(), cursor, stream.data() + stream.size());
    return payload;
}

std::vector<std::uint8_t> RansBackend::decode(const ByteView payload,
                                               const std::size_t expected_size) const {
    if (expected_size == 0U) {
        throw std::runtime_error("rANS cannot decode an empty block");
    }
    std::size_t stream_offset = 0;
    const Model model = parse_model(payload, stream_offset);
    std::uint8_t* cursor = const_cast<std::uint8_t*>(payload.data() + stream_offset);
    std::uint8_t* const end = const_cast<std::uint8_t*>(payload.data() + payload.size());
    RansState state{};
    RansDecInit(&state, &cursor);

    std::vector<std::uint8_t> output(expected_size);
    for (std::size_t index = 0; index < output.size(); ++index) {
        const std::uint32_t slot = RansDecGet(&state, kScaleBits);
        const std::uint8_t symbol = model.lookup[slot];
        const RansDecSymbol& decoded_symbol = model.decoder_symbols[symbol];
        if (decoded_symbol.freq == 0U) {
            throw std::runtime_error("rANS cumulative lookup is invalid");
        }
        require_available_renormalization(state, decoded_symbol, cursor, end);
        output[index] = symbol;
        RansDecAdvanceSymbol(&state, &cursor, &decoded_symbol, kScaleBits);
    }
    if (cursor != end) {
        throw std::runtime_error("rANS payload has unconsumed bytes");
    }
    return output;
}

std::size_t RansBackend::maximum_payload_size(const std::size_t input_size) {
    constexpr std::size_t kMaximumModelBytes = kPayloadPrefixSize + 256U * kEntrySize;
    constexpr std::size_t kMaximumStreamOverhead = 16U;
    if (input_size > std::numeric_limits<std::size_t>::max() -
                         kMaximumModelBytes - kMaximumStreamOverhead) {
        throw std::overflow_error("rANS payload bound overflow");
    }
    return input_size + kMaximumModelBytes + kMaximumStreamOverhead;
}

}  // namespace hz::r2
