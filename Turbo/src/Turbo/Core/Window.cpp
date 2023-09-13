#include "tbopch.h"
#include "Window.h"

#include "Application.h"

#ifdef TBO_PLATFORM_WIN32 
    #include "Turbo/Platform/Win32/Win32_Window.h"
#endif

namespace Turbo {
   
    Window::Window(const Window::Config& config)
        : m_Config(config)
    {
        // Creates vulkan context and debugger
        m_RendererContext = RendererContext::Create();
    }

    Window::~Window()
    {
        m_Swapchain.Reset();
        m_RendererContext.Reset();
    }

    Owned<Window> Window::Create()
    {
        const auto& appConfig = Application::Get().GetConfig();

        // Populate window config
        Window::Config config;
        config.Title = appConfig.Title;
        config.Width = appConfig.Width;
        config.Height = appConfig.Height;
        config.VSync = appConfig.VSync;
        config.StartMaximized = appConfig.StartMaximized;
        config.Resizable = appConfig.Resizable;

        // TODO: If UI is disabled, change render target to swapchain framebuffers, TLDR; Render into the window instead of the UI
        config.SwapChainTarget = !appConfig.EnableUI;

#ifdef TBO_PLATFORM_WIN32
        return Owned<Win32_Window>::Create(config);
#else
    #error Platform not supported!
        return nullptr;
#endif
    }

    void Window::InitializeSwapChain()
    {
        // Creates vulkan surface and device
        m_RendererContext->Initialize();

        m_Swapchain = SwapChain::Create();
    }

}
