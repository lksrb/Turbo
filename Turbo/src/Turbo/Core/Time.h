#pragma once

#include "PrimitiveTypes.h"

namespace Turbo
{
    struct FTime
    {
    public:
        FTime(f32 ts = 0.0f) : m_Time(ts) {}

        f32 ms() const { return m_Time * 1000.0f; }
        f32 s() const { return m_Time; }

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
        FTime DeltaTime;
        FTime TimeSinceStart;
    };

}
