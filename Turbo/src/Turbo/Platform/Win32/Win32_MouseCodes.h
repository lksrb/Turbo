#pragma once

#include "Turbo/Core/MouseCodes.h"

namespace Turbo::Mouse {

    // From WinUser.h
    enum : MouseCode
    {
        ButtonLeft = 0x01,
        ButtonRight = 0x02,
        ButtonMiddle = 0x04
    };

}
