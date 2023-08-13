#pragma once

#include "RendererContext.h"

#include <array>

namespace Turbo
{
    constexpr inline u32 MaxFifCapacity = 3;

    // Templated class that abstracts frames in flight resources
    template<typename T>
    class Fly
    {
    public:
        Fly() : m_Data{}, m_Size(RendererContext::FramesInFlight()) {}
        ~Fly() = default;

        u32 Size() const { return m_Size; }
        constexpr T* Data() { return m_Data.data(); }
        constexpr const T* Data() const { return m_Data; }
        constexpr bool Empty() const { /* Maybe a hack */ return (u64)m_Data[0] == 0; }

        constexpr T& operator[](u32 index) { TBO_ENGINE_ASSERT(index < m_Size); return m_Data[index]; }
        constexpr const T& operator[](u32 index) const { TBO_ENGINE_ASSERT(index < m_Size); return m_Data[index]; }

        [[nodiscard]] constexpr auto begin() noexcept { return m_Data.begin(); }
        [[nodiscard]] constexpr auto end() noexcept { return m_Data.begin() + m_Size; }

        [[nodiscard]] constexpr auto begin() const noexcept { return m_Data.begin(); }
        [[nodiscard]] constexpr auto end() const noexcept { return m_Data.begin() + m_Size; }
    private:
        std::array<T, MaxFifCapacity> m_Data;
        u32 m_Size;
    };
}
