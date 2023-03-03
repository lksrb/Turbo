#pragma once

#include "Common.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <glm/glm.hpp>

namespace Turbo 
{
    class Log 
    {
    public:
        static void Initialize();
        static void Shutdown();

        inline static std::shared_ptr<spdlog::logger>& GetEngineLogger() { return s_EngineLogger; }
        inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
    private:
        static std::shared_ptr<spdlog::logger> s_EngineLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;

        static bool s_Initialized;
    };
}

template<typename OStream>
OStream& operator<<(OStream& os, const glm::vec3& vec)
{
    return os << '[' << vec.x << ", " << vec.y << ", " << vec.z << ']';
}

template<typename OStream>
OStream& operator<<(OStream& os, const glm::vec4& vec)
{
    return os << '[' << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ']';
}

#ifdef TBO_DEBUG
    //Client log macros
    #define TBO_TRACE(...)          ::Turbo::Log::GetClientLogger()->trace(__VA_ARGS__)
    #define TBO_INFO(...)           ::Turbo::Log::GetClientLogger()->info(__VA_ARGS__)
    #define TBO_WARN(...)           ::Turbo::Log::GetClientLogger()->warn(__VA_ARGS__)
    #define TBO_ERROR(...)		    ::Turbo::Log::GetClientLogger()->error(__VA_ARGS__)
    #define TBO_FATAL(...)          ::Turbo::Log::GetClientLogger()->critical(__VA_ARGS__)
    //Core log macros
    #define TBO_ENGINE_TRACE(...)	::Turbo::Log::GetEngineLogger()->trace(__VA_ARGS__)
    #define TBO_ENGINE_INFO(...)	::Turbo::Log::GetEngineLogger()->info(__VA_ARGS__)
    #define TBO_ENGINE_WARN(...)	::Turbo::Log::GetEngineLogger()->warn(__VA_ARGS__)
    #define TBO_ENGINE_ERROR(...)	::Turbo::Log::GetEngineLogger()->error(__VA_ARGS__)
    #define TBO_ENGINE_FATAL(...)	::Turbo::Log::GetEngineLogger()->critical(__VA_ARGS__)
#elif TBO_RELEASE
    //Client log macros
    #define TBO_ERROR(...)		
    #define TBO_WARN(...)		
    #define TBO_TRACE(...)		
    #define TBO_INFO(...)			
    #define TBO_FATAL(...)		
    
    //Core log macros
    #define TBO_ENGINE_TRACE(...)
    #define TBO_ENGINE_INFO(...)
    #define TBO_ENGINE_WARN(...)
    #define TBO_ENGINE_ERROR(...)
    #define TBO_ENGINE_FATAL(...)
#endif

