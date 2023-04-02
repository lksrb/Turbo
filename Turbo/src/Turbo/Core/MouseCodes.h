#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#ifdef TBO_PLATFORM_WIN32
    #include <winuser.h>
#endif

namespace Turbo
{
    using MouseCode = u32;

    namespace Mouse
    {
#ifdef TBO_PLATFORM_WIN32
        enum : MouseCode
        {
            ButtonLeft = VK_LBUTTON,
            ButtonRight = VK_RBUTTON,
            ButtonMiddle = VK_MBUTTON
        };
    }
#endif
}
