#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <glm/glm.hpp>

namespace Turbo 
{
    class Log 
    {
    public:
        enum Level
        {
            Trace = 0,
            Info,
            Warn,
            Error,
            Fatal
        };

        static void Initialize();
        static void Shutdown();

        static inline std::shared_ptr<spdlog::logger>& GetEngineLogger() { return s_EngineLogger; }
        static inline std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
        static inline std::shared_ptr<spdlog::logger>& GetEditorConsoleLogger() { return s_EditorConsoleLogger; }
    private:
        static inline std::shared_ptr<spdlog::logger> s_EngineLogger;
        static inline std::shared_ptr<spdlog::logger> s_ClientLogger;
        static inline std::shared_ptr<spdlog::logger> s_EditorConsoleLogger;

        static inline bool s_Initialized = false;
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
    // Editor log macros
    #define TBO_CONSOLE_TRACE(...)	::Turbo::Log::GetEditorConsoleLogger()->trace(__VA_ARGS__)
    #define TBO_CONSOLE_INFO(...)	::Turbo::Log::GetEditorConsoleLogger()->info(__VA_ARGS__)
    #define TBO_CONSOLE_WARN(...)	::Turbo::Log::GetEditorConsoleLogger()->warn(__VA_ARGS__)
    #define TBO_CONSOLE_ERROR(...)	::Turbo::Log::GetEditorConsoleLogger()->error(__VA_ARGS__)
    #define TBO_CONSOLE_FATAL(...)	::Turbo::Log::GetEditorConsoleLogger()->critical(__VA_ARGS__)
#elif TBO_RELEASE
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
    // Editor log macros
    #define TBO_CONSOLE_TRACE(...)	::Turbo::Log::GetEditorConsoleLogger()->trace(__VA_ARGS__)
    #define TBO_CONSOLE_INFO(...)	::Turbo::Log::GetEditorConsoleLogger()->info(__VA_ARGS__)
    #define TBO_CONSOLE_WARN(...)	::Turbo::Log::GetEditorConsoleLogger()->warn(__VA_ARGS__)
    #define TBO_CONSOLE_ERROR(...)	::Turbo::Log::GetEditorConsoleLogger()->error(__VA_ARGS__)
    #define TBO_CONSOLE_FATAL(...)	::Turbo::Log::GetEditorConsoleLogger()->critical(__VA_ARGS__)
#endif

