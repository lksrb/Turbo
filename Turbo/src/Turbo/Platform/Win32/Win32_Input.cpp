#include "tbopch.h" 
#include "Turbo/Core/Input.h"

#include "Turbo/Core/Application.h"
#include "Win32_Window.h"
#include "Win32_Utils.h"

#include <imgui_internal.h>
#include <WinUser.h>

#define TBO_HOLD 0x8000

namespace Turbo {

    struct InputData
    {
        std::array<HCURSOR, CursorMode_Count> Cursors;
        CursorMode Mode = CursorMode_Normal;
        CursorMode LastMode = CursorMode_Normal;
        POINT RestoreCursorPos = {};
        POINT LastCursorPos = {};
        POINT VirtualCursorPos = {};

        InputData()
        {
            Cursors[CursorMode_Normal] = LoadCursor(NULL, IDC_ARROW);
            Cursors[CursorMode_Hidden] = nullptr;
            Cursors[CursorMode_Locked] = nullptr;
        }
    };

    static InputData s_Data;

    namespace Utils {

        static bool IsViewportFocused()
        {
            bool enableUI = Application::Get().GetConfig().EnableUI;

            if (!enableUI)
            {
                return Application::Get().GetViewportWindow()->IsFocused();
            }

            ImGuiContext* context = ImGui::GetCurrentContext();
            return context->NavWindow != nullptr;
        }
    }

    void Input::Update()
    {
        Win32_Window* window = (Win32_Window*)Application::Get().GetViewportWindow();
        s_Data.LastCursorPos = window->GetLastCursorPosition();
        s_Data.VirtualCursorPos = window->GetVirtualCursorPosition();

        if (s_Data.Mode == CursorMode_Locked)
        {
            // Center cursor 
            if (s_Data.LastCursorPos.x != window->GetWidth() / 2 || s_Data.LastCursorPos.y != window->GetHeight() / 2)
            {
                POINT pos = { (LONG)window->GetWidth() / 2, (LONG)window->GetHeight() / 2 };
                window->SetCursorPosition(pos);
            }
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

        s_Data.LastMode = s_Data.Mode;
        s_Data.Mode = cursorMode;

        Win32_Window* window = (Win32_Window*)Application::Get().GetViewportWindow();

        if (s_Data.Mode == CursorMode_Locked)
        {
            // Store cursor position
            ::GetCursorPos(&s_Data.RestoreCursorPos);
            ::ScreenToClient(window->GetHandle(), &s_Data.RestoreCursorPos);

            // Center cursor 
            RECT area;
            ::GetClientRect(window->GetHandle(), &area);
            ::SetCursorPos(area.right / 2, area.bottom / 2);

            // Ensure that the cursor stays inside screen
            RECT clipRect;
            ::GetClientRect(window->GetHandle(), &clipRect);
            ::ClientToScreen(window->GetHandle(), (POINT*)&clipRect.left);
            ::ClientToScreen(window->GetHandle(), (POINT*)&clipRect.right);
            ::ClipCursor(&clipRect);
        }

        if (s_Data.LastMode == CursorMode_Locked)
        {
            window->SetCursorPosition(s_Data.RestoreCursorPos);

            // Unlocks cursor from window
            ::ClipCursor(NULL);
        }

        ::SetCursor(s_Data.Cursors[cursorMode]);
    }

    CursorMode Input::GetCursorMode()
    {
        return s_Data.Mode;
    }

    std::pair<i32, i32> Input::GetMousePosition()
    {
        Win32_Window* window = (Win32_Window*)Application::Get().GetViewportWindow();
        POINT mousePosition = s_Data.Mode != CursorMode_Locked ? s_Data.LastCursorPos : s_Data.VirtualCursorPos;
        return { mousePosition.x, mousePosition.y };
    }


}
