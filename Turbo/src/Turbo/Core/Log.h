#pragma once

#include "common.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace Turbo {

    class Log {
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
