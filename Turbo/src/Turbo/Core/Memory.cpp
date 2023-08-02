#include "tbopch.h"
#include "Memory.h"

#ifdef TBO_PROFILE_MEMORY
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
#endif

namespace Turbo {

#ifdef TBO_PROFILE_MEMORY
    struct MemoryInternal {

        MemoryInternal()
        {
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
            _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        }
    };

    // Statically allocated
    static MemoryInternal s_Internal;
#endif
}
