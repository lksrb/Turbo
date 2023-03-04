#include "tbopch.h" 
#include "Turbo/Core/Input.h"

#include <windows.h>

namespace Turbo 
{
    bool Input::IsKeyPressed(KeyCode keycode)
    {
        return (::GetAsyncKeyState(static_cast<int>(keycode)) & 0x8000) != 0;
    }

    bool Input::IsKeyReleased(KeyCode keycode)
    {
        return (::GetAsyncKeyState(static_cast<int>(keycode)) & 0x8000) == 0;
    }

    bool Input::IsMouseButtonPressed(MouseCode mousecode)
    {
        return (::GetAsyncKeyState(static_cast<int>(mousecode)) & 0x8000) != 0;
    }

    bool Input::IsMouseButtonReleased(MouseCode mousecode)
    {
        return (::GetAsyncKeyState(static_cast<int>(mousecode)) & 0x8000) == 0;
    }

    i32 Input::GetMouseX()
    {
        POINT mouse_pos = { -1, -1 };
        GetCursorPos(&mouse_pos);
        return mouse_pos.x;
    }
    i32 Input::GetMouseY()
    {
        POINT mouse_pos = { -1, -1 };
        GetCursorPos(&mouse_pos);
        return mouse_pos.y;
    }
}
