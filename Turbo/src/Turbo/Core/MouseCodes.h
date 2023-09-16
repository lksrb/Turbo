#pragma once

#include "PrimitiveTypes.h"

namespace Turbo {
    using MouseCode = u32;
}

#ifdef TBO_PLATFORM_WIN32
    #include "Turbo/Platform/Win32/Win32_MouseCodes.h"
#else
    #error "Platform not supported yet!"
#endif

