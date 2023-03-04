#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"

namespace Turbo 
{
    class Input
    {
    public:
        static bool IsKeyPressed(KeyCode keycode);
        static bool IsKeyReleased(KeyCode keycode);

        static bool IsMouseButtonPressed(KeyCode keycode);
        static bool IsMouseButtonReleased(KeyCode keycode);

        static i32 GetMouseX();
        static i32 GetMouseY();
    };
}
