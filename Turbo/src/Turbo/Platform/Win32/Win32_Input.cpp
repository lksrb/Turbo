#include "tbopch.h" 
#include "Turbo/Core/Input.h"

#include "Turbo/Core/Engine.h"

#include "Win32_Window.h"

#include <Windows.h>

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
        Win32_Window* window = (Win32_Window*)Engine::Get().GetViewportWindow();

        POINT mousePosition = { -1, -1 };
        GetCursorPos(&mousePosition);
        //ScreenToClient(window->GetHandle(), &mousePosition);

        return mousePosition.x;
    }
    i32 Input::GetMouseY()
    {
        Win32_Window* window = (Win32_Window*)Engine::Get().GetViewportWindow();

        POINT mousePosition = { -1, -1 };
        GetCursorPos(&mousePosition);
        //ScreenToClient(window->GetHandle(), &mousePosition);

        return mousePosition.y;
    }
}
