#include "r2/entropy/ctw_backend.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

namespace hz::r2 {
namespace {

constexpr std::array<std::uint8_t, 4> kMagic{'H', 'Z', 'C', '1'};
constexpr std::uint8_t kVersion = 1;
constexpr std::uint8_t kFlags = 1;  // raw length and encoded bit length present
constexpr unsigned kProbabilityPrecision = 12;
constexpr unsigned kDelayWidth = 64;
constexpr std::uint64_t kProbabilityScale = 1ULL << kProbabilityPrecision;
constexpr std::size_t kMaximumExpansionFactor = 32;
constexpr std::size_t kCoderTerminationBits =
    kDelayWidth + kProbabilityPrecision + 1U;

std::size_t checked_add(const std::size_t left,
                        const std::size_t right,
                        const char* message) {
    if (left > std::numeric_limits<std::size_t>::max() - right) {
        throw std::length_error(message);
    }
    return left + right;
}

std::size_t checked_multiply(const std::size_t left,
                             const std::size_t right,
                             const char* message) {
    if (left != 0 && right > std::numeric_limits<std::size_t>::max() / left) {
        throw std::length_error(message);
    }
    return left * right;
}

std::uint32_t crc32(const ByteView input) noexcept {
    std::uint32_t checksum = 0xFFFFFFFFU;
    for (std::size_t index = 0; index < input.size(); ++index) {
        checksum ^= input[index];
        for (unsigned bit = 0; bit < 8U; ++bit) {
            const std::uint32_t mask =
                0U - static_cast<std::uint32_t>(checksum & 1U);
            checksum = (checksum >> 1U) ^ (0xEDB88320U & mask);
        }
    }
    return ~checksum;
}

void write_u32_le(std::vector<std::uint8_t>& output,
                  const std::size_t offset,
                  const std::uint32_t value) noexcept {
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        output[offset + shift / 8U] =
            static_cast<std::uint8_t>(value >> shift);
    }
}

void write_u64_le(std::vector<std::uint8_t>& output,
                  const std::size_t offset,
                  const std::uint64_t value) noexcept {
    for (unsigned shift = 0; shift < 64U; shift += 8U) {
        output[offset + shift / 8U] =
            static_cast<std::uint8_t>(value >> shift);
    }
}

std::uint32_t read_u32_le(const ByteView input,
                          const std::size_t offset) noexcept {
    std::uint32_t value = 0;
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        value |= static_cast<std::uint32_t>(input[offset + shift / 8U])
                 << shift;
    }
    return value;
}

std::uint64_t read_u64_le(const ByteView input,
                          const std::size_t offset) noexcept {
    std::uint64_t value = 0;
    for (unsigned shift = 0; shift < 64U; shift += 8U) {
        value |= static_cast<std::uint64_t>(input[offset + shift / 8U])
                 << shift;
    }
    return value;
}

struct Node {
    double log_prob = 0.0;
    std::uint32_t zeros = 0;
    std::uint32_t ones = 0;
    double log_kt = 0.0;
    std::unique_ptr<Node> left;   // suffix ending in one
    std::unique_ptr<Node> right;  // suffix ending in zero
};

struct Snapshot {
    Node* node = nullptr;
    double log_prob = 0.0;
    std::uint32_t zeros = 0;
    std::uint32_t ones = 0;
    double log_kt = 0.0;
    bool is_new = false;
    bool new_left = false;
};

double log_add_exp(const double x, const double y) {
    const double delta = x - y;
    if (delta > 0.0) {
        return x + std::log1p(std::exp(-delta));
    }
    return y + std::log1p(std::exp(delta));
}

void update_kt(Node& node, const int bit) {
    const double zeros = static_cast<double>(node.zeros);
    const double ones = static_cast<double>(node.ones);
    if (bit == 0) {
        node.log_kt += std::log(zeros + 0.5) - std::log(zeros + ones + 1.0);
        ++node.zeros;
    } else {
        node.log_kt += std::log(ones + 0.5) - std::log(zeros + ones + 1.0);
        ++node.ones;
    }
}

std::vector<Snapshot> update(Node& root,
                             const std::vector<std::uint8_t>& context,
                             const int bit) {
    std::vector<Snapshot> traversed;
    traversed.reserve(context.size() + 1U);
    Node* node = &root;
    traversed.push_back({node, node->log_prob, node->zeros, node->ones,
                         node->log_kt, false, false});
    update_kt(*node, bit);

    for (std::size_t depth = 0; depth < context.size(); ++depth) {
        const std::uint8_t context_bit =
            context[context.size() - 1U - depth];
        bool is_new = false;
        const bool new_left = context_bit != 0;
        std::unique_ptr<Node>& child = new_left ? node->left : node->right;
        if (!child) {
            child = std::make_unique<Node>();
            is_new = true;
        }
        node = child.get();
        traversed.push_back({node, node->log_prob, node->zeros, node->ones,
                             node->log_kt, is_new, new_left});
        update_kt(*node, bit);
    }

    for (std::size_t index = traversed.size(); index-- > 0;) {
        Node& current = *traversed[index].node;
        if (current.left || current.right) {
            const double left = current.left ? current.left->log_prob : 0.0;
            const double right = current.right ? current.right->log_prob : 0.0;
            current.log_prob = log_add_exp(
                std::log(0.5) + current.log_kt,
                std::log(0.5) + left + right);
        } else {
            current.log_prob = current.log_kt;
        }
    }
    return traversed;
}

void revert(std::vector<Snapshot>& traversed) {
    for (std::size_t index = traversed.size(); index-- > 0;) {
        Snapshot& snapshot = traversed[index];
        Node& node = *snapshot.node;
        node.log_prob = snapshot.log_prob;
        node.zeros = snapshot.zeros;
        node.ones = snapshot.ones;
        node.log_kt = snapshot.log_kt;
        if (index + 1U < traversed.size()) {
            const Snapshot& next = traversed[index + 1U];
            if (next.is_new) {
                if (next.new_left) {
                    node.left.reset();
                } else {
                    node.right.reset();
                }
            }
        }
    }
}

class CtwModel {
public:
    explicit CtwModel(const std::uint8_t depth)
        : context_(depth, 0), root_(std::make_unique<Node>()) {}

    double prob_zero() {
        const double before = root_->log_prob;
        std::vector<Snapshot> traversal = update(*root_, context_, 0);
        const double after = root_->log_prob;
        revert(traversal);
        return std::exp(after - before);
    }

    void observe(const int bit) {
        std::vector<Snapshot> ignored = update(*root_, context_, bit);
        (void)ignored;
        if (!context_.empty()) {
            std::move(context_.begin() + 1, context_.end(), context_.begin());
            context_.back() = static_cast<std::uint8_t>(bit);
        }
    }

private:
    std::vector<std::uint8_t> context_;
    std::unique_ptr<Node> root_;
};

struct ExpTables {
    std::array<std::uint64_t, (1U << kProbabilityPrecision) + 1U> a{};
    std::array<std::uint64_t, (1U << kProbabilityPrecision)> b{};

    ExpTables() {
        for (std::size_t index = 1; index <= kProbabilityScale; ++index) {
            a[index] = static_cast<std::uint64_t>(
                static_cast<double>(kProbabilityScale) *
                    std::exp2(-static_cast<double>(index) /
                              static_cast<double>(kProbabilityScale)) +
                0.5);
        }
        for (std::size_t index = (1U << (kProbabilityPrecision - 1U));
             index < kProbabilityScale; ++index) {
            b[index] = static_cast<std::uint64_t>(
                -static_cast<double>(kProbabilityScale) *
                    std::log2(static_cast<double>(index) /
                              static_cast<double>(kProbabilityScale)) +
                0.5);
        }
        for (std::size_t index = 1;
             index < (1U << (kProbabilityPrecision - 1U)); ++index) {
            const double k = std::ceil(
                static_cast<double>(kProbabilityPrecision) - 1.0 -
                std::log2(static_cast<double>(index)));
            const std::size_t scaled = static_cast<std::size_t>(
                std::exp2(k) * static_cast<double>(index));
            if (scaled >= b.size() || b[scaled] == 0) {
                throw std::runtime_error("CTW exponential table construction failed");
            }
            b[index] = b[scaled] + static_cast<std::uint64_t>(
                k * static_cast<double>(kProbabilityScale));
        }
    }
};

const ExpTables& exp_tables() {
    static const ExpTables tables;
    return tables;
}

std::uint64_t v0_for(const double probability) {
    const double p = std::max(probability, 1.0 - probability);
    std::uint64_t value = static_cast<std::uint64_t>(
        std::exp2(static_cast<double>(kProbabilityPrecision)) *
            std::log2(1.0 / p) + 0.5);
    return std::max<std::uint64_t>(3U, value);
}

class BitWriter {
public:
    void push(const std::uint8_t bit) {
        if ((bit & 1U) != 0) {
            if (bit_index_ == 0) bytes_.push_back(0);
            bytes_.back() |= static_cast<std::uint8_t>(1U << bit_index_);
        } else if (bit_index_ == 0) {
            bytes_.push_back(0);
        }
        bit_index_ = (bit_index_ + 1U) & 7U;
        ++count_;
    }
    const std::vector<std::uint8_t>& bytes() const noexcept { return bytes_; }
    std::uint64_t count() const noexcept { return count_; }

private:
    std::vector<std::uint8_t> bytes_;
    unsigned bit_index_ = 0;
    std::uint64_t count_ = 0;
};

class BitReader {
public:
    BitReader(const ByteView bytes, const std::uint64_t bit_count)
        : bytes_(bytes), bit_count_(bit_count) {}

    std::uint8_t pull() {
        if (position_ >= bit_count_) {
            throw std::runtime_error("CTW arithmetic stream ended early");
        }
        const std::uint8_t result = static_cast<std::uint8_t>(
            (bytes_[position_ / 8U] >> (position_ & 7U)) & 1U);
        ++position_;
        return result;
    }
    std::uint64_t position() const noexcept { return position_; }

private:
    ByteView bytes_;
    std::uint64_t bit_count_ = 0;
    std::uint64_t position_ = 0;
};

std::vector<std::uint8_t> encode_bits(const ByteView input,
                                      const std::uint8_t depth,
                                      std::uint64_t& bit_count) {
    const ExpTables& tables = exp_tables();
    CtwModel model(depth);
    BitWriter output;
    std::uint64_t delay = 0;
    std::uint64_t accum = 0;
    std::uint64_t range = 1;
    for (std::size_t index = 0; index < input.size(); ++index) {
        const std::uint8_t value = input[index];
        for (unsigned bit_index = 0; bit_index < 8U; ++bit_index) {
            const int bit = (value >> bit_index) & 1U;
            const double probability = model.prob_zero();
            model.observe(bit);
            const bool zero_more_likely = probability > 0.5;
            const int transformed = zero_more_likely ? bit : (bit == 0 ? 1 : 0);
            const std::uint64_t v0 = v0_for(probability);

            while (range > kProbabilityScale) {
                output.push(delay >= (1ULL << (kDelayWidth - 1U)) ? 1U : 0U);
                delay = (delay >= (1ULL << (kDelayWidth - 1U)))
                    ? 2ULL * (delay - (1ULL << (kDelayWidth - 1U)))
                    : 2ULL * delay;
                if (accum >= kProbabilityScale) {
                    ++delay;
                    accum = 2ULL * (accum - kProbabilityScale);
                } else {
                    accum *= 2ULL;
                }
                range -= kProbabilityScale;
            }
            while (delay == std::numeric_limits<std::uint64_t>::max()) {
                output.push(1U);
                delay = 2ULL * (delay - (1ULL << (kDelayWidth - 1U)));
                if (accum >= kProbabilityScale) {
                    ++delay;
                    accum = 2ULL * (accum - kProbabilityScale);
                } else {
                    accum *= 2ULL;
                }
            }

            const std::uint64_t v = range + v0;
            if (transformed != 0) {
                if (v <= kProbabilityScale) {
                    accum += 2ULL * tables.a[v];
                    if (accum >= (1ULL << (kProbabilityPrecision + 1U))) {
                        ++delay;
                        accum -= (1ULL << (kProbabilityPrecision + 1U));
                    }
                    range = tables.b[tables.a[range] - tables.a[v]];
                } else {
                    accum += tables.a[v - kProbabilityScale];
                    if (accum >= (1ULL << (kProbabilityPrecision + 1U))) {
                        ++delay;
                        accum -= (1ULL << (kProbabilityPrecision + 1U));
                    }
                    range = tables.b[2ULL * tables.a[range] -
                                     tables.a[v - kProbabilityScale]] +
                            kProbabilityScale;
                }
            } else {
                range = v;
            }
        }
    }
    for (unsigned index = 0; index < kDelayWidth; ++index) {
        output.push(delay < (1ULL << (kDelayWidth - 1U)) ? 0U : 1U);
        delay = delay < (1ULL << (kDelayWidth - 1U))
            ? 2ULL * delay
            : 2ULL * (delay - (1ULL << (kDelayWidth - 1U)));
    }
    for (unsigned index = 0; index < kProbabilityPrecision + 1U; ++index) {
        output.push(accum < kProbabilityScale ? 0U : 1U);
        accum = accum < kProbabilityScale
            ? 2ULL * accum
            : 2ULL * (accum - kProbabilityScale);
    }
    bit_count = output.count();
    return output.bytes();
}

std::vector<std::uint8_t> decode_bits(const ByteView packed,
                                      const std::uint64_t bit_count,
                                      const std::uint64_t output_size,
                                      const std::uint8_t depth) {
    const ExpTables& tables = exp_tables();
    CtwModel model(depth);
    BitReader input(packed, bit_count);
    std::uint64_t delay = 0;
    std::uint64_t accum = 0;
    std::uint64_t range = 1;
    std::uint64_t coded_delay = 0;
    std::uint64_t coded_accum = 0;
    for (unsigned index = 0; index < kDelayWidth; ++index) {
        coded_delay = coded_delay * 2ULL + input.pull();
    }
    for (unsigned index = 0; index < kProbabilityPrecision + 1U; ++index) {
        coded_accum = coded_accum * 2ULL + input.pull();
    }

    std::vector<std::uint8_t> output(static_cast<std::size_t>(output_size));
    for (std::size_t byte_index = 0; byte_index < output.size(); ++byte_index) {
        std::uint8_t value = 0;
        for (unsigned bit_index = 0; bit_index < 8U; ++bit_index) {
            const double probability = model.prob_zero();
            const std::uint64_t v0 = v0_for(probability);
            while (range > kProbabilityScale) {
                delay = delay >= (1ULL << (kDelayWidth - 1U))
                    ? 2ULL * (delay - (1ULL << (kDelayWidth - 1U)))
                    : 2ULL * delay;
                if (accum >= kProbabilityScale) {
                    ++delay;
                    accum = 2ULL * (accum - kProbabilityScale);
                } else {
                    accum *= 2ULL;
                }
                range -= kProbabilityScale;
                coded_delay = coded_delay >=
                        (1ULL << (kDelayWidth - 1U))
                    ? 2ULL * (coded_delay -
                              (1ULL << (kDelayWidth - 1U)))
                    : 2ULL * coded_delay;
                const std::uint8_t pulled = input.pull();
                if (coded_accum >= kProbabilityScale) {
                    ++coded_delay;
                    coded_accum =
                        2ULL * (coded_accum - kProbabilityScale) + pulled;
                } else {
                    coded_accum = 2ULL * coded_accum + pulled;
                }
            }
            while (delay == std::numeric_limits<std::uint64_t>::max()) {
                delay = 2ULL * (delay - (1ULL << (kDelayWidth - 1U)));
                if (accum >= kProbabilityScale) {
                    ++delay;
                    accum = 2ULL * (accum - kProbabilityScale);
                } else {
                    accum *= 2ULL;
                }
                coded_delay = coded_delay >=
                        (1ULL << (kDelayWidth - 1U))
                    ? 2ULL * (coded_delay -
                              (1ULL << (kDelayWidth - 1U)))
                    : 2ULL * coded_delay;
                const std::uint8_t pulled = input.pull();
                if (coded_accum >= kProbabilityScale) {
                    ++coded_delay;
                    coded_accum =
                        2ULL * (coded_accum - kProbabilityScale) + pulled;
                } else {
                    coded_accum = 2ULL * coded_accum + pulled;
                }
            }

            const std::uint64_t v = range + v0;
            int transformed = 0;
            if (v <= kProbabilityScale) {
                std::uint64_t candidate_accum = accum + 2ULL * tables.a[v];
                std::uint64_t candidate_delay = delay;
                if (candidate_accum >= (1ULL << (kProbabilityPrecision + 1U))) {
                    ++candidate_delay;
                    candidate_accum -= (1ULL << (kProbabilityPrecision + 1U));
                }
                if (coded_delay == candidate_delay &&
                    coded_accum < candidate_accum) {
                    transformed = 0;
                } else if (coded_delay < candidate_delay) {
                    transformed = 0;
                } else {
                    transformed = 1;
                    delay = candidate_delay;
                    accum = candidate_accum;
                    range = tables.b[tables.a[range] - tables.a[v]];
                }
                if (transformed == 0) range = v;
            } else {
                std::uint64_t candidate_accum =
                    accum + tables.a[v - kProbabilityScale];
                std::uint64_t candidate_delay = delay;
                if (candidate_accum >= (1ULL << (kProbabilityPrecision + 1U))) {
                    ++candidate_delay;
                    candidate_accum -= (1ULL << (kProbabilityPrecision + 1U));
                }
                if (coded_delay == candidate_delay &&
                    coded_accum < candidate_accum) {
                    transformed = 0;
                } else if (coded_delay < candidate_delay) {
                    transformed = 0;
                } else {
                    transformed = 1;
                    delay = candidate_delay;
                    accum = candidate_accum;
                    range = tables.b[2ULL * tables.a[range] -
                                     tables.a[v - kProbabilityScale]] +
                            kProbabilityScale;
                }
                if (transformed == 0) range = v;
            }
            const int bit = probability <= 0.5 ? (transformed == 0 ? 1 : 0)
                                               : transformed;
            if (bit != 0) value |= static_cast<std::uint8_t>(1U << bit_index);
            model.observe(bit);
        }
        output[byte_index] = value;
    }
    return output;
}

void require_valid_view(const ByteView input, const char* operation) {
    if (!input.empty() && input.data() == nullptr) {
        throw std::invalid_argument(std::string(operation) +
                                    " received a null byte view");
    }
}

}  // namespace

CtwBackend::CtwBackend(const std::uint8_t depth,
                       const std::size_t maximum_output_size)
    : depth_(depth), maximum_output_size_(maximum_output_size) {
    if (depth_ == 0 || depth_ > kMaximumDepth) {
        throw std::invalid_argument("CTW depth must be between 1 and 64");
    }
    if (maximum_output_size_ > std::numeric_limits<std::uint64_t>::max()) {
        throw std::invalid_argument("CTW output limit is not representable");
    }
}

std::vector<std::uint8_t> CtwBackend::encode(const ByteView input) const {
    require_valid_view(input, "CTW encoder");
    if (input.empty()) {
        throw std::invalid_argument("CTW does not encode an empty block");
    }
    if (input.size() > maximum_output_size_) {
        throw std::length_error("CTW input exceeds configured block limit");
    }
    std::uint64_t bit_count = 0;
    const std::vector<std::uint8_t> compressed =
        encode_bits(input, depth_, bit_count);
    if (compressed.size() > std::numeric_limits<std::uint64_t>::max()) {
        throw std::length_error("CTW payload is too large");
    }
    std::vector<std::uint8_t> output(
        checked_add(kPayloadHeaderSize, compressed.size(),
                    "CTW payload size overflow"));
    std::copy(kMagic.begin(), kMagic.end(), output.begin());
    output[4] = kVersion;
    output[5] = kFlags;
    output[6] = depth_;
    output[7] = static_cast<std::uint8_t>(kPayloadHeaderSize);
    write_u64_le(output, 8, static_cast<std::uint64_t>(input.size()));
    write_u64_le(output, 16, bit_count);
    write_u64_le(output, 24, static_cast<std::uint64_t>(compressed.size()));
    write_u32_le(output, 32, crc32(input));
    std::copy(compressed.begin(), compressed.end(),
              output.begin() + static_cast<std::ptrdiff_t>(kPayloadHeaderSize));
    write_u32_le(output, 36,
                 crc32(ByteView(compressed.data(), compressed.size())));
    return output;
}

std::vector<std::uint8_t> CtwBackend::decode(
    const ByteView payload, const std::size_t expected_size) const {
    require_valid_view(payload, "CTW decoder");
    if (payload.size() < kPayloadHeaderSize + 1U) {
        throw std::runtime_error("Truncated CTW payload");
    }
    if (!std::equal(kMagic.begin(), kMagic.end(), payload.data()) ||
        payload[4] != kVersion || payload[5] != kFlags ||
        payload[7] != kPayloadHeaderSize) {
        throw std::runtime_error("Unsupported CTW payload format");
    }
    const std::uint8_t depth = payload[6];
    if (depth == 0 || depth > kMaximumDepth) {
        throw std::runtime_error("Unsupported CTW depth");
    }
    for (std::size_t index = 40; index < kPayloadHeaderSize; ++index) {
        if (payload[index] != 0) {
            throw std::runtime_error("Invalid CTW payload header");
        }
    }
    const std::uint64_t raw_size = read_u64_le(payload, 8);
    const std::uint64_t bit_count = read_u64_le(payload, 16);
    const std::uint64_t compressed_size = read_u64_le(payload, 24);
    const std::uint64_t required_stream_size =
        bit_count / 8U + (bit_count % 8U != 0 ? 1U : 0U);
    if (raw_size != expected_size || raw_size == 0 ||
        raw_size > maximum_output_size_ ||
        compressed_size != payload.size() - kPayloadHeaderSize ||
        compressed_size != required_stream_size ||
        compressed_size == 0 || bit_count < kCoderTerminationBits ||
        bit_count > compressed_size * 8U) {
        throw std::runtime_error("Invalid CTW payload size contract");
    }
    const ByteView compressed(payload.data() + kPayloadHeaderSize,
                              static_cast<std::size_t>(compressed_size));
    if (crc32(compressed) != read_u32_le(payload, 36)) {
        throw std::runtime_error("CTW compressed payload checksum mismatch");
    }
    const unsigned padding_bits = static_cast<unsigned>(
        compressed_size * 8U - bit_count);
    if (padding_bits != 0 &&
        (compressed[compressed.size() - 1U] &
         static_cast<std::uint8_t>(0xFFU << (8U - padding_bits))) != 0) {
        throw std::runtime_error("CTW packed stream has nonzero padding bits");
    }
    std::vector<std::uint8_t> output = decode_bits(
        compressed, bit_count, raw_size, depth);
    if (crc32(ByteView(output)) != read_u32_le(payload, 32)) {
        throw std::runtime_error("CTW output checksum mismatch");
    }
    return output;
}

std::size_t CtwBackend::maximum_payload_size(const std::size_t input_size) {
    return checked_add(
        kPayloadHeaderSize,
        checked_add(checked_multiply(input_size, kMaximumExpansionFactor,
                                     "CTW payload bound overflow"),
                    1U, "CTW payload bound overflow"),
        "CTW payload bound overflow");
}

}  // namespace hz::r2
