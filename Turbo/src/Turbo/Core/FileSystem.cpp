#include "tbopch.h"
#include "FileSystem.h"

#include "Turbo/Debug/ScopeTimer.h"

#include <Windows.h>
#include <Shlwapi.h>

namespace Turbo
{
    Buffer FileSystem::ReadBinary(const std::filesystem::path& path)
    {
        // std::ios::ate - seeks end of file
        std::ifstream stream(path, std::ios::binary | std::ios::ate);

        if (!stream)
            return {};

        std::streampos end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        u64 size = static_cast<u64>(end - stream.tellg());
        if (size == 0)
            return {};

        Buffer buffer(size);
        stream.read(buffer.As<char>(), size);
        stream.close();
        return buffer;
    }

    bool FileSystem::Exists(const std::filesystem::path& path)
    {
        return ::PathFileExists(path.c_str());
    }

    std::filesystem::path FileSystem::ReplaceExtension(std::filesystem::path filepath, const std::filesystem::path& extension)
    {
        return filepath.replace_extension(extension);
    }

    std::filesystem::path FileSystem::GetCurrentDirectory()
    {
        return std::filesystem::current_path();
    }

    std::filesystem::path FileSystem::RelativeToCurrentDirectory(const std::filesystem::path filepath)
    {
        return std::filesystem::relative(filepath, GetCurrentDirectory());
    }

}
