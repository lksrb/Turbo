#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#include <chrono>

namespace Turbo::Debug
{
	class ScopeTimer 
	{
	public:
        ScopeTimer(std::string_view name, bool output = true);
        ~ScopeTimer();

        f32 Stop();
	private:
		std::chrono::time_point<std::chrono::steady_clock> m_Start, m_End;
        std::string_view m_Name;
		bool m_Output;
	};
}

