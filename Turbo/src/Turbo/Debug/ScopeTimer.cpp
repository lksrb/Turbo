#include "tbopch.h"
#include "ScopeTimer.h"

namespace Turbo {

    ScopeTimer::ScopeTimer(std::string_view name, bool output)
        : m_Name(name), m_Output(output)
    {
        m_Start = std::chrono::high_resolution_clock::now();
    }

    f32 ScopeTimer::Stop()
    {
        m_End = std::chrono::high_resolution_clock::now();

        std::chrono::duration<f32> duration = m_End - m_Start;
        return duration.count() * 1000.0f;
    }

    ScopeTimer::~ScopeTimer()
    {
        if (m_Output == false)
            return;

        m_End = std::chrono::high_resolution_clock::now();

        std::chrono::duration<f32> duration = m_End - m_Start;
        TBO_ENGINE_WARN("{0} took {1} ms", m_Name, duration.count() * 1000.0f);
    }

}
