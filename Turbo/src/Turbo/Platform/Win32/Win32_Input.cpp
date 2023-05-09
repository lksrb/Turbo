#include "tbopch.h" 
#include "Turbo/Core/Input.h"

#include "Turbo/Core/Engine.h"

#include "Win32_Window.h"
#include "Win32_Utils.h"

#include <WinUser.h>

namespace Turbo
{
    struct InputData
    {
        std::array<HCURSOR, CursorMode::Count> Cursors;
        CursorMode Mode;

        InputData()
        {
            Cursors[CursorMode::Hidden] = nullptr;
            Cursors[CursorMode::Arrow] = LoadCursor(NULL, IDC_ARROW);
            Cursors[CursorMode::Hand] = LoadCursor(NULL, IDC_HAND);

            Mode = CursorMode::Arrow;
        }
    };

    static InputData s_Data;

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

    void Input::SetCursorMode(CursorMode cursorMode)
    {
        if (cursorMode == s_Data.Mode)
            return;

        if (cursorMode >= CursorMode::Count)
        {
            TBO_ENGINE_ERROR("Invalid cursor mode!");
            return;
        }

        s_Data.Mode = cursorMode;
        ::SetCursor(s_Data.Cursors[cursorMode]);
    }

    CursorMode Input::GetCursorMode()
    {
        return s_Data.Mode;
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
