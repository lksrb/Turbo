#include "tbopch.h"

#include "Win32_Window.h"

#include "Turbo/Core/Application.h"
#include "Turbo/Core/Platform.h"
#include "Turbo/Core/Input.h"

#include "Turbo/Event/MouseEvent.h"
#include "Turbo/Event/WindowEvent.h"
#include "Turbo/Event/KeyEvent.h"

#include "Turbo/Renderer/SwapChain.h"

#include <windowsx.h>

#define TBO_EDITOR_ICON_PATH "Resources\\Icons\\EditorIcon.ico"

// Copy this line into your .cpp file to forward declare the function.
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Turbo {

    const wchar_t* s_ClassName = L"MY WINDOW HOLY MOLY";

    Win32_Window::Win32_Window(const Window::Config& config)
        : Window(config)
    {
        InitializeWindow();
    }

    Win32_Window::~Win32_Window()
    {
        ::DestroyWindow(m_Handle);
        ::UnregisterClass(s_ClassName, m_Instance);
    }

    void Win32_Window::SetCursorPosition(POINT cursorPos)
    {
        m_LastCursorPosition = cursorPos;

        ::ClientToScreen(m_Handle, &cursorPos);
        ::SetCursorPos(cursorPos.x, cursorPos.y);
    }

    void Win32_Window::InitializeWindow()
    {
        std::filesystem::path iconPath = std::filesystem::current_path() / TBO_EDITOR_ICON_PATH;

        m_Instance = ::GetModuleHandle(NULL);
        WNDCLASS wndClass = {};
        wndClass.lpszClassName = s_ClassName;
        wndClass.hInstance = m_Instance;
        wndClass.hIcon = reinterpret_cast<HICON>(::LoadImage(nullptr, iconPath.c_str(), IMAGE_ICON, 256, 256, LR_LOADFROMFILE));
        wndClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wndClass.lpfnWndProc = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT 
        {
            // User Interface
            if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
                return 0;

            Win32_Window* currentWindow = reinterpret_cast<Win32_Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));;

            if (currentWindow)
            {
                return currentWindow->ProcessWin32Events(hWnd, uMsg, wParam, lParam);
            }

            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        };

        RegisterClass(&wndClass);

        DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;

        if (m_Config.StartMaximized)
            style |= WS_MAXIMIZE;

        if (m_Config.Resizable)
            style |= WS_MAXIMIZEBOX | WS_THICKFRAME;

        RECT rect;
        rect.left = 250;
        rect.top = 250;
        rect.right = rect.left + m_Config.Width;
        rect.bottom = rect.top + m_Config.Height;

        ::AdjustWindowRect(&rect, style, false);

        size_t title_size = m_Config.Title.size() + 1;
        WCHAR ws[MAX_PATH] = {};
        mbstowcs_s(NULL, &ws[0], title_size, m_Config.Title.c_str(), title_size - 1);

        m_Handle = CreateWindowEx(
            0,
            s_ClassName,
            ws,
            style,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            NULL,
            NULL,
            m_Instance, /*Pass instance of this class*/this
        );

        // TODO: Custom title bar
#if 0
        // Remove the caption
        LONG_PTR style1 = GetWindowLongPtr(m_Handle, GWL_STYLE);
        SetWindowLongPtr(m_Handle, GWL_STYLE, style1 & ~WS_CAPTION);

        // Redraw the window to apply the changes
        SetWindowPos(m_Handle, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
#endif

        // Set user pointer
        SetWindowLongPtr(m_Handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }

    LRESULT CALLBACK Win32_Window::ProcessWin32Events(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            // Mouse events
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            {
                MouseCode button = -1;
                i32 mouseX = static_cast<i32>(GET_X_LPARAM(lParam));
                i32 mouseY = static_cast<i32>(GET_Y_LPARAM(lParam));

                if (wParam & MK_LBUTTON) { button = Mouse::ButtonLeft; }
                if (wParam & MK_RBUTTON) { button = Mouse::ButtonRight; }

                //if (uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONDBLCLK) { button = Mouse::Button2; }
                //if (uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }

                MouseButtonPressedEvent e(button, mouseX, mouseY);
                m_Callback(e);
                break;
            }
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            {
                MouseCode button = -1;
                i32 mouseX = static_cast<i32>(GET_X_LPARAM(lParam));
                i32 mouseY = static_cast<i32>(GET_Y_LPARAM(lParam));

                if (wParam & MK_LBUTTON) { button = Mouse::ButtonLeft; }
                if (wParam & MK_RBUTTON) { button = Mouse::ButtonRight; }

                MouseButtonReleasedEvent e(button, mouseX, mouseY);
                m_Callback(e);
                break;
            }
            case WM_MOUSEWHEEL:
            {
                f32 deltaX = 0.0f, deltaY = 0.0f;
                f32 commonDelta = std::signbit(static_cast<f32>(GET_WHEEL_DELTA_WPARAM(wParam))) ? -1.0f : 1.0f;

                // Vertical
                if (!(wParam & MK_SHIFT))
                {
                    deltaX = 0;

                    deltaY = commonDelta;
                }
                else // Horizontal
                {
                    deltaX = commonDelta;
                    deltaY = 0;
                }

                MouseScrolledEvent e(deltaX, deltaY);
                m_Callback(e);

                break;
            }
            case WM_MOUSEMOVE:
            {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);

                const int dx = x - m_LastCursorPosition.x;
                const int dy = y - m_LastCursorPosition.y;

                m_VirtualCursorPosition.x += dx;
                m_VirtualCursorPosition.y += dy;

                m_LastCursorPosition.x = x;
                m_LastCursorPosition.y = y;
                break;
            }

            // Window events
            case WM_CLOSE:
            {
                WindowCloseEvent e;
                m_Callback(e);

                ::ShowWindow(m_Handle, SW_HIDE);

                return 0;
            }
            case WM_DESTROY:
            {
                ::PostQuitMessage(0);
                return 0;
            }
            case WM_SIZE:
            {
                // TODO: Maybe should be managed by function; SetWidth, SetHeight
                m_Config.Width = GET_X_LPARAM(lParam);
                m_Config.Height = GET_Y_LPARAM(lParam);

                m_Minimized = m_Config.Width == 0 || m_Config.Height == 0;

                if (!m_Minimized)
                {
                    m_Swapchain->Resize(m_Config.Width, m_Config.Height);
                }

                WindowResizeEvent e(m_Config.Width, m_Config.Height);
                m_Callback(e);
                break;
            }
            case WM_MOVE:
            {
                u32 x = GET_X_LPARAM(lParam);
                u32 y = GET_Y_LPARAM(lParam);

                m_OffsetX = x;
                m_OffsetY = y;

                WindowMovedEvent e(x, y);
                m_Callback(e);
                break;
            }
            case WM_SETFOCUS:
            {
                m_Focused = true;

                WindowFocusedEvent e;
                m_Callback(e);
                break;
            }
            case WM_KILLFOCUS:
            {
                m_Focused = false;

                WindowLostFocusEvent e;
                m_Callback(e);
                break;
            }
            // Key events
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                if (wParam < 256) // ?? 
                {
                    bool isKeyDown = uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN;
                    KeyCode keyCode = static_cast<u32>(wParam);

                    if (isKeyDown)
                    {
                        // Checking the previous keystate 
                        bool isRepeated = lParam & 0x40000000;

                        KeyPressedEvent e(keyCode, isRepeated);
                        m_Callback(e);
                    }
                    else
                    {
                        KeyReleasedEvent e(keyCode);
                        m_Callback(e);
                    }
                }
                break;
            }
            case WM_MENUCHAR:
            case WM_SYSCHAR:
            {
                // Disable beeping sound
                return 0;
            }
        }

        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    void Win32_Window::ProcessEvents()
    {
        TBO_PROFILE_FUNC();

        MSG msg = {};

        while (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        Input::Update();
    }

    void Win32_Window::Show()
    {
        ::ShowWindow(m_Handle, SW_SHOW);
    }

    void Win32_Window::SetTitle(const std::string& title)
    {
        m_Config.Title = title;

        size_t titleSize = m_Config.Title.size() + 1;
        WCHAR ws[MAX_PATH] = {};
        mbstowcs_s(NULL, &ws[0], titleSize, m_Config.Title.c_str(), titleSize - 1);

        ::SetWindowText(m_Handle, ws);
    }

    void Win32_Window::AcquireNewFrame()
    {
        m_Swapchain->NewFrame();
    }

    void Win32_Window::SwapFrame()
    {
        m_Swapchain->SwapFrame();
    }

}
