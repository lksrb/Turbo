#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"

namespace Turbo 
{
    enum CursorMode : u32
    {
        CursorMode_Hidden = 0,
        CursorMode_Arrow,
        CursorMode_Hand,

        CursorMode_Count
    };

    class Input
    {
    public:
        static bool IsKeyPressed(const KeyCode keyCode);
        static bool IsKeyReleased(const KeyCode keyCode);

        static bool IsMouseButtonPressed(const MouseCode mouseCode);
        static bool IsMouseButtonReleased(const MouseCode mouseCode);

        static void SetCursorMode(CursorMode cursorMode);
        static CursorMode GetCursorMode();

        static i32 GetMouseX();
        static i32 GetMouseY();
    };
}
