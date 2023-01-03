#include "tbopch.h"
#include "Window.h"

#include "Application.h"

#ifdef TBO_PLATFORM_WIN32 
    #include "Turbo/Platform/Win32/Win32_Window.h"
#endif

namespace Turbo {
   
    Window::Window(const Window::Config& specification)
        : m_Config(specification), m_Callback(nullptr), m_Swapchain(nullptr), m_Minimized(false), m_Focused(false)
    {
    }

    Window::~Window()
    {
    }

    Window* Window::Create(const Window::Config& specification)
    {
        //Window::Config wndSpecification;
        //wndSpecification.Title = appSpecification.Title;
        //wndSpecification.Width = appSpecification.Width;
        //wndSpecification.Height = appSpecification.Height;
        //wndSpecification.VSync = appSpecification.VSync;
        //wndSpecification.StartMaximized = appSpecification.StartMaximized;
        //wndSpecification.Resizable = appSpecification.Resizable;

#ifdef TBO_PLATFORM_WIN32
        return new Win32_Window(specification);
#else
#error Platform not supported!
        return nullptr;
#endif
    }

	bool Window::IsFocused() const
	{
        return m_Focused;
	}

    Ptr<SwapChain> Window::GetSwapchain() const
    {
        return m_Swapchain;
    }

    const FString64& Window::GetTitle() const
    {
        return m_Config.Title;
    }

    u32 Window::GetWidth() const
	{
        return m_Config.Width;
	}

	u32 Window::GetHeight() const
	{
        return m_Config.Height;
	}

    bool Window::IsMinimized() const
    {
        return m_Minimized;
    }

    void Window::SetEventCallback(const EventCallback& callback)
    {
        m_Callback = callback;
    }

}
