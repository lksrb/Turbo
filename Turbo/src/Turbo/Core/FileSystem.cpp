#include "tbopch.h"
#include "FileSystem.h"

#include "Turbo/Debug/ScopeTimer.h"

#include <Windows.h>

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

}
