#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"

namespace Turbo 
{
    class Input
    {
    public:
        static bool IsKeyPressed(const KeyCode keyCode);
        static bool IsKeyReleased(const KeyCode keyCode);

        static bool IsMouseButtonPressed(const MouseCode mouseCode);
        static bool IsMouseButtonReleased(const MouseCode mouseCode);

        static i32 GetMouseX();
        static i32 GetMouseY();
    };
}
