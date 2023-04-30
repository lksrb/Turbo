#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    enum class RootKey : u32
    {
        None = 0,
#ifdef TBO_PLATFORM_WIN32
        LocalMachine
#endif
    };

    class Platform
    {
    public:
        static void Init();
        static void Shutdown();

        static f32 GetTime();

        // Dialogs
        static std::filesystem::path OpenFileDialog(const char* title = "", const char* filter = "");
        static std::filesystem::path OpenBrowseFolderDialog(const char* title = "", const char* savedPath = "");
        static std::filesystem::path SaveFileDialog(const char* filter = "", const char* suffix = "");

        static bool OpenFileExplorer(const std::filesystem::path& directory);

        static bool Execute(const std::filesystem::path& appName, const std::wstring& args = L"", const std::filesystem::path& currentPath = "", bool wait = false);

        static std::filesystem::path GetRegistryValue(RootKey rootKey, const std::wstring& registryKey, const std::wstring& value = L"");
    };
}
