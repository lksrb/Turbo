#pragma once

#include "Buffer.h"

namespace Turbo
{
    class FileSystem
    {
    public:
        static Buffer ReadBinary(const std::filesystem::path& path);
    };
}
