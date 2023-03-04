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
            ButtonLeft = MK_LBUTTON,
            ButtonRight = MK_RBUTTON,
            ButtonMiddle = MK_MBUTTON
        };
    }
#endif
}
