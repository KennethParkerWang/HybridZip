#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace hz::r2 {

class ByteView {
public:
    ByteView() noexcept = default;

    ByteView(const std::uint8_t* data, const std::size_t size) noexcept
        : data_(data), size_(size) {}

    explicit ByteView(const std::vector<std::uint8_t>& bytes) noexcept
        : data_(bytes.data()), size_(bytes.size()) {}

    const std::uint8_t* data() const noexcept { return data_; }
    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    const std::uint8_t& operator[](const std::size_t index) const noexcept {
        return data_[index];
    }

private:
    const std::uint8_t* data_ = nullptr;
    std::size_t size_ = 0;
};

inline std::vector<std::uint8_t> copy_bytes(const ByteView input) {
    if (input.empty()) {
        return {};
    }
    return {input.data(), input.data() + input.size()};
}

}  // namespace hz::r2

