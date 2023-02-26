#include "tbopch.h"

#include "Win32_Window.h"

#include "Turbo/Core/Engine.h"
#include "Turbo/Core/Filepath.h"
#include "Turbo/Core/Platform.h"

#include "Turbo/Event/MouseEvent.h"
#include "Turbo/Event/WindowEvent.h"
#include "Turbo/Event/KeyEvent.h"

#include "Turbo/Renderer/SwapChain.h"

#include <windowsx.h>

// Copy this line into your .cpp file to forward declare the function.
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Turbo {

    LRESULT CALLBACK Win32_Window::Win32Procedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
    }

    const wchar_t* s_ClassName = L"MY WINDOW HOLY MOLY";

    Win32_Window::Win32_Window(const Window::Config& specification)
        : Window(specification), m_Handle(nullptr), m_Instance(nullptr)
    {
        InitializeWindow();
    }

    Win32_Window::~Win32_Window()
    {
        //m_Swapchain.Reset();

        ::DestroyWindow(m_Handle);
        ::UnregisterClass(s_ClassName, m_Instance);
    }

    void Win32_Window::InitializeSwapchain()
    {
        // Create swapchain
        m_Swapchain = SwapChain::Create();
    }

    void Win32_Window::InitializeWindow()
    {
        Filepath currentDirectory = Platform::GetCurrentPath() / "Resources" / "Icons" / "EditorIcon.ico";

        m_Instance = ::GetModuleHandle(NULL);
        WNDCLASS wndClass = {};
        wndClass.lpszClassName = s_ClassName;
        wndClass.hInstance = m_Instance;
        wndClass.hIcon = reinterpret_cast<HICON>(::LoadImageA(nullptr, currentDirectory.CStr(), IMAGE_ICON, 256, 256, LR_LOADFROMFILE));
        wndClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wndClass.lpfnWndProc = Win32_Window::Win32Procedure;

        RegisterClass(&wndClass);

        DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;

        if (m_Config.Resizable)
            style |= WS_MAXIMIZEBOX | WS_THICKFRAME;

        RECT rect;
        rect.left = 250;
        rect.top = 250;
        rect.right = rect.left + m_Config.Width;
        rect.bottom = rect.top + m_Config.Height;

        ::AdjustWindowRect(&rect, style, false);

        size_t sizeInBytes = m_Config.Title.Cap();

        wchar_t ws[64];
        mbstowcs_s(NULL, &ws[0], sizeInBytes, m_Config.Title.CStr(), sizeInBytes);
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
                MouseCode button = Mouse::Button0;
                i32 mouseX = static_cast<i32>(GET_X_LPARAM(lParam));
                i32 mouseY = static_cast<i32>(GET_Y_LPARAM(lParam));

                if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK) { button = Mouse::Button0; }
                if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK) { button = Mouse::Button1; }
                //if (uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONDBLCLK) { button = Mouse::Button2; }
                //if (uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }

                MouseButtonPressedEvent e(button, mouseX, mouseY);
                m_Callback(e);
                break;
            }
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            {
                MouseCode button = Mouse::Button0;
                i32 mouseX = static_cast<i32>(GET_X_LPARAM(lParam));
                i32 mouseY = static_cast<i32>(GET_Y_LPARAM(lParam));

                if (uMsg == WM_LBUTTONUP) { button = Mouse::Button0; }
                if (uMsg == WM_RBUTTONUP) { button = Mouse::Button1; }

                MouseButtonReleasedEvent e(button, mouseX, mouseY);
                m_Callback(e);
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

                if (m_Minimized == false) 
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
                if (wParam < 256) // [?] UTF-8
                {
                    bool isKeyDown = WM_KEYDOWN || uMsg == WM_SYSKEYDOWN;
                    int key = (int)wParam;

                    static int lastKey = -1;

                    bool repeat = lastKey == key;
                    lastKey = key;

                    if (isKeyDown)
                    {
                        KeyPressedEvent e(static_cast<KeyCode>(key), repeat);
                        m_Callback(e);
                    }
                    else
                    {
                        lastKey = -1;
                        KeyReleasedEvent e(static_cast<KeyCode>(key));
                        m_Callback(e);
                    }
                }
                break;
            }
            case WM_MENUCHAR:
            case WM_SYSCHAR:
            {
                // Annoying beeping sound disable
                return 0;
            }
        }

        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    void Win32_Window::ProcessEvents()
    {
        MSG msg = {};

        while (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    void Win32_Window::Show()
    {
        ::ShowWindow(m_Handle, SW_SHOW);
    }

    void Win32_Window::SetTitle(const FString64& title)
    {
        m_Config.Title = title;

        size_t sizeInBytes = title.Cap();

        wchar_t ws[64];
        mbstowcs_s(NULL, &ws[0], sizeInBytes , title.CStr(), sizeInBytes);

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
