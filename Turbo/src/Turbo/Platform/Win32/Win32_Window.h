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

        void InitializeSwapchain() override;
        void AcquireNewFrame() override;
        void SwapFrame() override;
    private:
        void InitializeWindow();
        LRESULT CALLBACK ProcessWin32Events(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        static LRESULT CALLBACK Win32Procedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    private:
        HWND m_Handle = nullptr;
        HINSTANCE m_Instance = nullptr;
    };

}
