#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#include "Turbo/Event/Event.h"

#include "Turbo/Renderer/SwapChain.h"

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
        static Window* Create(const Window::Config& specification);

        virtual void ProcessEvents() = 0;
        virtual void Show() = 0;

        virtual void AcquireNewFrame() = 0;
        virtual void SwapFrame() = 0;

        virtual void SetTitle(const std::string& title) = 0;
        bool IsFocused() const;

        Ref<SwapChain> GetSwapchain() const;
        std::string_view GetTitle() const;
        u32 GetWidth() const;
        u32 GetHeight() const;

        i32 GetOffsetX() const;
        i32 GetOffsetY() const;

        bool IsMinimized() const;

        void SetEventCallback(const EventCallback& callback);
    protected:
        Window(const Window::Config& specification);
        virtual void InitializeSwapchain() = 0;
    protected:
        Ref<SwapChain> m_Swapchain;

        i32 m_OffsetX = 0, m_OffsetY = 0;

        bool m_Focused = false;
        bool m_Minimized = false;
        EventCallback m_Callback = nullptr;
        Window::Config m_Config;

        friend class RendererContext;
    };

}
