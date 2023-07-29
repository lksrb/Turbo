#include "tbopch.h"
#include "Memory.h"

#ifdef TBO_PROFILE_MEMORY
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
#endif

namespace Turbo
{
    constexpr u64 DefaultPoolSize = 100 * 1024 * 1024;

    struct MemoryInternal
    {
        MemoryInternal()
        {
            _CrtSetBreakAlloc(13810);
            _CrtSetBreakAlloc(17835);
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
            _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        }
    };

#ifdef TBO_PROFILE_MEMORY
    // Statically allocated
    static MemoryInternal s_Internal;
#endif
}
