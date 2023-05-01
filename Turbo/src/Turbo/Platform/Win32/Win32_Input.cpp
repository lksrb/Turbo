#include "tbopch.h" 
#include "Turbo/Core/Input.h"

#include "Turbo/Core/Engine.h"

#include "Win32_Window.h"
#include "Win32_Utils.h"

#include <WinUser.h>

namespace Turbo 
{
    bool Input::IsKeyPressed(const KeyCode keyCode)
    {
        bool focused = Engine::Get().GetViewportWindow()->IsFocused();

        Win32Code win32Code = Utils::GetWin32CodeFromKeyCode(keyCode);

        return focused && (::GetAsyncKeyState(win32Code) & 0x8000) != 0;
    }

    bool Input::IsKeyReleased(const KeyCode keyCode)
    {
        bool focused = Engine::Get().GetViewportWindow()->IsFocused();

        Win32Code win32Code = Utils::GetWin32CodeFromKeyCode(keyCode);

        return focused && (::GetAsyncKeyState(win32Code) & 0x8000) == 0;
    }

    bool Input::IsMouseButtonPressed(const MouseCode mouseCode)
    {
        bool focused = Engine::Get().GetViewportWindow()->IsFocused();

        return focused && (::GetAsyncKeyState(static_cast<int>(mouseCode)) & 0x8000) != 0;
    }

    bool Input::IsMouseButtonReleased(const MouseCode mouseCode)
    {
        bool focused = Engine::Get().GetViewportWindow()->IsFocused();

        return focused && (::GetAsyncKeyState(static_cast<int>(mouseCode)) & 0x8000) == 0;
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
