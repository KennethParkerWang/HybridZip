#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace hz {

class ByteHistory {
public:
    explicit ByteHistory(std::size_t capacity);

    std::uint64_t position() const noexcept;
    std::size_t size() const noexcept;
    std::size_t capacity() const noexcept;

    std::uint8_t back(std::size_t distance) const;
    std::uint8_t at_absolute(std::uint64_t position) const;
    bool contains(std::uint64_t position) const noexcept;

    void push(std::uint8_t byte);
    void reset() noexcept;

private:
    std::vector<std::uint8_t> storage_;
    std::uint64_t position_ = 0;
    std::size_t size_ = 0;
};

}  // namespace hz
