#pragma once

#include "RendererSettings.h"

#include <array>

namespace Turbo
{
    constexpr inline u32 MaxFramesInFlightCapacity = 3;

    // Templated class that abstracts frames in flight resources
    template<typename T, u32 TSize = RendererSettings::FramesInFlight>
    class Fly
    {
    public:
        constexpr Fly() : m_Data{} {}
        constexpr ~Fly() = default;

        constexpr u32 Size() const { return TSize; }
        constexpr T* Data() { return m_Data.data(); }
        constexpr const T* Data() const { return m_Data; }
        constexpr bool Empty() const { /* Maybe a hack */ return (u64)m_Data[0] == 0; }

        constexpr T& operator[](u32 index) { TBO_ENGINE_ASSERT(index < TSize); return m_Data[index]; }
        constexpr const T& operator[](u32 index) const { TBO_ENGINE_ASSERT(index < TSize); return m_Data[index]; }

        [[nodiscard]] constexpr auto begin() noexcept { return m_Data.begin(); }
        [[nodiscard]] constexpr auto end() noexcept { return m_Data.begin() + TSize; }

        [[nodiscard]] constexpr auto begin() const noexcept { return m_Data.begin(); }
        [[nodiscard]] constexpr auto end() const noexcept { return m_Data.begin() + TSize; }
    private:
        std::array<T, MaxFramesInFlightCapacity> m_Data;
    };
}
