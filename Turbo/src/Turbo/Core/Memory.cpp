#include "tbopch.h"
#include "Memory.h"

#include <filesystem>

#if defined(_MSC_VER) && defined(TBO_PROFILE_MEMORY)
#include <crtdbg.h>
#include <windows.h>

#define _CRTDBG_MAP_ALLOC
#define TBO_CHECK_MEMORYLEAKS(__buffer)  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE); \
_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT); \
_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); \
_CrtSetReportFile(_CRT_WARN, __buffer);

#else
#error Currently Windows only!
#endif

namespace Turbo
{
    constexpr size_t DefaultPoolSize = size_t(100) * size_t(1024) * size_t(1024);

    struct MemoryInternal
    {
        HANDLE OutMemoryLeakStream = nullptr;
    };

    // Statically allocated
    static MemoryInternal s_Internal;

    void Memory::Initialize()
    {
        // NOTE: Moved memory leaks checks because mono leaks so much I cannot even see the console output
        // Now outputs to memoryleaks.txt

        // Outputs memory leaks 
#ifdef TBO_PROFILE_MEMORY

        //    _CrtSetBreakAlloc(462);

        const auto& path = std::filesystem::current_path() / "memoryleaks.txt";
        s_Internal.OutMemoryLeakStream = ::CreateFile(path.c_str(), GENERIC_WRITE,
            FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, NULL);
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_WARN, s_Internal.OutMemoryLeakStream);
#endif
    }

    void Memory::Shutdown()
    {
#ifdef TBO_PROFILE_MEMORY
        _CrtDumpMemoryLeaks();
        ::CloseHandle(s_Internal.OutMemoryLeakStream);
#endif
    }

}
