#pragma once

#include <cstdint>
#include <cstddef>

namespace server {

using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

class BufferView
{
public:
    BufferView(const std::byte* data, int32 size) : m_data(data), m_size(size) {}
    const std::byte* GetData() const { return m_data; }
    int32 GetSize() const { return m_size; }

private:
    const std::byte* m_data;
    int32 m_size;
};

} // namespace server