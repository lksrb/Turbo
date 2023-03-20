#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    class Platform
    {
    public:
        static void Initialize();
        static void Shutdown();

        static f32 GetTime();

        // Dialogs
        static std::filesystem::path OpenFileDialog(const char* title = "", const char* filter = "");
        static std::filesystem::path OpenBrowseFolderDialog(const char* title = "", const char* savedPath = "");
        static std::filesystem::path SaveFileDialog(const char* filter = "", const char* suffix = "");

        static bool Start(const std::string& app_name, const std::string& args);
    };
}
