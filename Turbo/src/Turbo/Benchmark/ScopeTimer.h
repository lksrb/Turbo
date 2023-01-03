#pragma once

#include "Turbo/Core/Common.h"

#include <chrono>

namespace Turbo::Benchmark
{
	class ScopeTimer 
	{
	public:
		ScopeTimer(const std::string& name, bool output = true)
			: m_Name(name), m_Output(output)
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		float Stop() 
		{
			m_End = std::chrono::high_resolution_clock::now();

			std::chrono::duration<float> duration = m_End - m_Start;
			return duration.count() * 1000.0f;
		}
		~ScopeTimer() 
		{
			if (m_Output == false)
				return;
			m_End = std::chrono::high_resolution_clock::now();

			std::chrono::duration<float> duration = m_End - m_Start;
			TBO_ENGINE_WARN("{0} took {1} ms", m_Name, duration.count() * 1000.0f);
		}
	private:
		std::chrono::time_point<std::chrono::steady_clock> m_Start, m_End;
		std::string m_Name;
		bool m_Output;
	};
}

