#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"

namespace Turbo 
{
    enum CursorMode : u32
    {
        CursorMode_Normal = 0,
        CursorMode_Hidden = 1,
        CursorMode_Locked = 2,

        CursorMode_Count = 3
    };

    class Input
    {
    public:
        static void Update();

        static bool IsKeyPressed(const KeyCode keyCode);
        static bool IsKeyReleased(const KeyCode keyCode);

        static bool IsMouseButtonPressed(const MouseCode mouseCode);
        static bool IsMouseButtonReleased(const MouseCode mouseCode);

        static void SetCursorMode(CursorMode cursorMode);
        static CursorMode GetCursorMode();

        static std::pair<i32, i32> GetMousePosition();
    };
}
