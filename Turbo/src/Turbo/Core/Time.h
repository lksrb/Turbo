#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    class Time_T
    {
    public:
        Time_T(f32 ts = 0.0f) : m_Time(ts) {}

        f32 ms() const { return m_Time * 1000.0f; }
        f32 s() const { return m_Time; }

        f32 operator()()
        {
            return m_Time;
        }

        f32 operator+=(f32 value)
        {
            m_Time += value;
            return m_Time;
        }

        operator f32() const { return m_Time; }

    private:
        f32 m_Time;
    };

    struct Time
    {
        Time_T DeltaTime;
        Time_T TimeSinceStart;
    };

}
