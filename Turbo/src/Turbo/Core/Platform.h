#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#include "Turbo/Core/Filepath.h"

namespace Turbo
{
    namespace Platform
    {
        enum class Result : u32
        {
            Success = 0,
            PathNotFound,
            AlreadyExists,
            Error
        };

        void Initialize();
        void Shutdown();

        Result CreateDirectory(const Filepath& path);
        bool IsDirectoryEmpty(const Filepath& path);

        bool PathExists(const Filepath& path);

        Result CreateFile(const Filepath& rootPath, const char* filename, const char* extension);

        Filepath GetCurrentPath();
        void SetCurrentPath(const Filepath& path);

        Filepath OpenFileDialog(const char* title = "", const char* filter = "");
        Filepath OpenBrowseFolderDialog(const char* title = "", const char* savedPath = "");
        Filepath SaveFileDialog(const char* filter = "", const char* suffix = "");
        f32 GetTime();
    };

}
