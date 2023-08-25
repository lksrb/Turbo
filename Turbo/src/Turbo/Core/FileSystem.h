#pragma once

#include "Buffer.h"

#include <filesystem>

namespace Turbo
{
    class FileSystem
    {
    public:
        static Buffer ReadBinary(const std::filesystem::path& path);
        static void WriteBinary(const std::filesystem::path& path, Buffer buffer);

        static bool Exists(const std::filesystem::path& path);
        static std::filesystem::path ReplaceExtension(std::filesystem::path filepath, const std::filesystem::path& extension);
        static std::filesystem::path GetCurrentDirectory();
        static std::filesystem::path RelativeToCurrentDirectory(const std::filesystem::path filepath);
    };
}
