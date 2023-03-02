#include "tbopch.h"

#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/base_sink.h>

namespace Turbo {

    std::shared_ptr<spdlog::logger> Log::s_EngineLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    bool Log::s_Initialized = false;

    void Log::Initialize()
    {
        if (s_Initialized)
            return;
        spdlog::sink_ptr log_sinks[2] =
        {
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
            std::make_shared<spdlog::sinks::basic_file_sink_mt>("Turbo.log", true)
        };

        log_sinks[0]->set_pattern("%^[%T] %n: %v%$");
        log_sinks[1]->set_pattern("[%T] [%l] %n: %v");

        s_EngineLogger = std::make_shared<spdlog::logger>("Turbo", begin(log_sinks), end(log_sinks));
        spdlog::register_logger(s_EngineLogger);
        s_EngineLogger->set_level(spdlog::level::trace);
        s_EngineLogger->flush_on(spdlog::level::trace);

        s_ClientLogger = std::make_shared<spdlog::logger>("App", begin(log_sinks), end(log_sinks));
        spdlog::register_logger(s_ClientLogger);
        s_ClientLogger->set_level(spdlog::level::trace);
        s_ClientLogger->flush_on(spdlog::level::trace);

        s_Initialized = true;
    }

    void Log::Shutdown()
    {
        if (s_Initialized == false)
            return;
        s_Initialized = false;

        s_EngineLogger.reset();
        s_ClientLogger.reset();
        spdlog::drop_all();

    }

}
