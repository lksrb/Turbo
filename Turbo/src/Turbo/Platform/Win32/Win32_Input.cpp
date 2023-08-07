#include "tbopch.h" 
#include "Turbo/Core/Input.h"

#include "Turbo/Core/Engine.h"
#include "Win32_Window.h"
#include "Win32_Utils.h"

#include <imgui_internal.h>
#include <WinUser.h>

#define TBO_HOLD 0x8000

namespace Turbo
{
    struct InputData
    {
        std::array<HCURSOR, CursorMode_Count> Cursors;
        CursorMode Mode;

        InputData()
        {
            Cursors[CursorMode_Hidden] = nullptr;
            Cursors[CursorMode_Arrow] = LoadCursor(NULL, IDC_ARROW);
            Cursors[CursorMode_Hand] = LoadCursor(NULL, IDC_HAND);

            Mode = CursorMode_Arrow;
        }
    };

    static InputData s_Data;

    namespace Utils
    {
        static bool IsViewportFocused()
        {
            bool enableUI = Engine::Get().GetApplication()->GetConfig().EnableUI;

            if (!enableUI)
            {
                return Engine::Get().GetViewportWindow()->IsFocused();
            }

            ImGuiContext* context = ImGui::GetCurrentContext();
            return context->NavWindow != nullptr;
        }
    }


    bool Input::IsKeyPressed(const KeyCode keyCode)
    {
        Win32Code win32Code = Utils::GetWin32CodeFromKeyCode(keyCode);

        return Utils::IsViewportFocused() && (::GetAsyncKeyState(win32Code) & TBO_HOLD) != 0;

    }
    bool Input::IsKeyReleased(const KeyCode keyCode)
    {
        Win32Code win32Code = Utils::GetWin32CodeFromKeyCode(keyCode);

        return Utils::IsViewportFocused() && (::GetAsyncKeyState(win32Code) & TBO_HOLD) == 0;
    }

    bool Input::IsMouseButtonPressed(const MouseCode mouseCode)
    {
        return  Utils::IsViewportFocused() && (::GetAsyncKeyState(static_cast<int>(mouseCode)) & TBO_HOLD) != 0;
    }

    bool Input::IsMouseButtonReleased(const MouseCode mouseCode)
    {
        return Utils::IsViewportFocused() && (::GetAsyncKeyState(static_cast<int>(mouseCode)) & TBO_HOLD) == 0;
    }

    void Input::SetCursorMode(CursorMode cursorMode)
    {
        if (cursorMode == s_Data.Mode)
            return;

        if (cursorMode >= CursorMode::CursorMode_Count)
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
