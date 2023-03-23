#include "tbopch.h" 
#include "Turbo/Core/Input.h"

#include "Turbo/Core/Engine.h"

#include <windows.h>

namespace Turbo 
{
    bool Input::IsKeyPressed(KeyCode keycode)
    {
        bool focused = Engine::Get().GetViewportWindow()->IsFocused();

        return focused && (::GetAsyncKeyState(static_cast<int>(keycode)) & 0x8000) != 0;
    }

    bool Input::IsKeyReleased(KeyCode keycode)
    {
        bool focused = Engine::Get().GetViewportWindow()->IsFocused();

        return focused && (::GetAsyncKeyState(static_cast<int>(keycode)) & 0x8000) == 0;
    }

    bool Input::IsMouseButtonPressed(MouseCode mousecode)
    {
        bool focused = Engine::Get().GetViewportWindow()->IsFocused();

        return focused && (::GetAsyncKeyState(static_cast<int>(mousecode)) & 0x8000) != 0;
    }

    bool Input::IsMouseButtonReleased(MouseCode mousecode)
    {
        bool focused = Engine::Get().GetViewportWindow()->IsFocused();

        return focused && (::GetAsyncKeyState(static_cast<int>(mousecode)) & 0x8000) == 0;
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
