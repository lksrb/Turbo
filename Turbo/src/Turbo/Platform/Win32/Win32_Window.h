#pragma once

#include "Turbo/Core/Window.h"

#include <Windows.h>

namespace Turbo
{
    class Win32_Window : public Window
    {
    public:
        Win32_Window(const Window::Config& config);
        ~Win32_Window();

        void ProcessEvents() override;
        void Show() override;
        void SetTitle(const std::string& title) override;

        HINSTANCE GetInstance() const { return m_Instance; }
        HWND GetHandle() const { return m_Handle; }
        POINT GetLastCursorPosition() { return m_LastCursorPosition; }
        POINT GetVirtualCursorPosition() const { return m_VirtualCursorPosition; }
        void SetCursorPosition(POINT cursorPos);

        void AcquireNewFrame() override;
        void SwapFrame() override;
    private:
        void InitializeWindow();
        LRESULT CALLBACK ProcessWin32Events(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    private:
        POINT m_LastCursorPosition = {};
        POINT m_VirtualCursorPosition = {}; // To keep track of cursor position even if its disabled
        HWND m_Handle = nullptr;
        HINSTANCE m_Instance = nullptr;
    };

}
