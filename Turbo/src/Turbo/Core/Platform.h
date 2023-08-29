#pragma once

namespace Turbo
{
    enum class RootKey : u32
    {
        None = 0,
        LocalMachine
    };

    class Platform
    {
    public:
        static f32 GetTime();

        // Dialogs
        static std::filesystem::path OpenFileDialog(const wchar_t* title = L"", const wchar_t* filter = L"", const std::filesystem::path& initialDir = L"");
        static std::filesystem::path OpenBrowseFolderDialog(const char* title = "", const char* savedPath = "");
        static std::filesystem::path SaveFileDialog(const char* filter = "", const char* suffix = "");

        static bool OpenFileExplorer(const std::filesystem::path& directory);

        static bool Execute(const std::filesystem::path& appName, const std::wstring& args = L"", const std::filesystem::path& currentPath = "", bool wait = false);

        static std::filesystem::path GetRegistryValue(RootKey rootKey, const std::wstring& registryKey, const std::wstring& value = L"");
    };
}
