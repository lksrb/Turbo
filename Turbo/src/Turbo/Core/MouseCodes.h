#pragma once

#include "PrimitiveTypes.h"

namespace Turbo {

    using MouseCode = u32;

    namespace Mouse {
        // From WinUser.h
        enum : MouseCode
        {
            ButtonLeft = 0x01,
            ButtonRight = 0x02,
            ButtonMiddle = 0x04
        };
    }

}
