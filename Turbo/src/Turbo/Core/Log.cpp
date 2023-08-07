#include "tbopch.h"

#include "Log.h"

#include "Turbo/Editor/EditorConsolePanel.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/base_sink.h>

namespace Turbo 
{
    void Log::Initialize()
    {
        if (s_Initialized)
            return;

        spdlog::sink_ptr logSinks[2] =
        {
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
            std::make_shared<spdlog::sinks::basic_file_sink_mt>("Turbo.log", true)
        };

        spdlog::sink_ptr editorSinks[2] =
        {
            std::make_shared<EditorConsoleSink>(),
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
        };

        logSinks[0]->set_pattern("%^[%T] %n: %v%$");
        logSinks[1]->set_pattern("[%T] [%l] %n: %v");

        editorSinks[0]->set_pattern("%^[%T] %n: %v%$");
        editorSinks[1]->set_pattern("%^[%T] %n: %v%$");

        s_EngineLogger = std::make_shared<spdlog::logger>("Turbo", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_EngineLogger);
        s_EngineLogger->set_level(spdlog::level::trace);
        s_EngineLogger->flush_on(spdlog::level::trace);

        s_ClientLogger = std::make_shared<spdlog::logger>("App", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_ClientLogger);
        s_ClientLogger->set_level(spdlog::level::trace);
        s_ClientLogger->flush_on(spdlog::level::trace);

        s_EditorConsoleLogger = std::make_shared<spdlog::logger>("Console", begin(editorSinks), end(editorSinks));
        s_EditorConsoleLogger->set_level(spdlog::level::trace);

        s_Initialized = true;
    }

    void Log::Shutdown()
    {
        s_EditorConsoleLogger.reset();
        s_EngineLogger.reset();
        s_ClientLogger.reset();
        spdlog::drop_all();

    }

}
