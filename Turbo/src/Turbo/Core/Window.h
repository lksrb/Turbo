#pragma once

#include "Turbo/Event/Event.h"

#include "Turbo/Renderer/SwapChain.h"
#include "Turbo/Renderer/RendererContext.h"

#include <functional>

namespace Turbo {

    using EventCallback = std::function<void(Event&)>;

    class Window
    {
    public:
        struct Config
        {
            std::string Title;
            u32 Width;
            u32 Height;
            bool VSync;
            bool StartMaximized;
            bool Resizable;
            bool SwapChainTarget;
        };

        virtual ~Window();
        static Window* Create();

        virtual void ProcessEvents() = 0;
        virtual void Show() = 0;

        virtual void AcquireNewFrame() = 0;
        virtual void SwapFrame() = 0;

        virtual void SetTitle(const std::string& title) = 0;

        void InitializeSwapChain();

        Ref<SwapChain> GetSwapchain() const { return m_Swapchain; }
        Ref<RendererContext> GetRendererContext() const { return m_RendererContext; }

        i32 GetOffsetX() const { return m_OffsetX; }
        i32 GetOffsetY() const { return m_OffsetY; }

        u32 GetWidth() const { return m_Config.Width; }
        u32 GetHeight() const { return m_Config.Height; }

        bool IsMinimized() const { return m_Minimized; }
        bool IsFocused() const { return m_Focused; }
        void SetEventCallback(const EventCallback& callback) { m_Callback = callback; }
    protected:
        Window(const Window::Config& config);
    protected:
        Ref<RendererContext> m_RendererContext;
        Ref<SwapChain> m_Swapchain;

        i32 m_OffsetX = 0, m_OffsetY = 0;

        bool m_Focused = false;
        bool m_Minimized = false;
        EventCallback m_Callback = nullptr;
        Window::Config m_Config;

        friend class RendererContext;
    };

}
