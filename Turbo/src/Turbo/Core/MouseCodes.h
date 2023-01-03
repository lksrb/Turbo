#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    using MouseCode = u32;

    namespace Mouse
    {
        enum : MouseCode
        {
            Button0 = 0,
            Button1,
            Button2,
            Button3,
            Button4,
            Button5,
            Button6,
            Button7,

            ButtonLast = Button7,
            ButtonLeft = Button0,
            ButtonRight = Button1,
            ButtonMiddle = Button2


        };
    }
}
