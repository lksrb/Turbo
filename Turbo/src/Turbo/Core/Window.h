#pragma once

#include "Turbo/Core/PrimitiveTypes.h"
#include "Turbo/Core/String.h"

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
            String64 Title;
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

        virtual void SetTitle(const String64& title) = 0;
        bool IsFocused() const;

        Ref<SwapChain> GetSwapchain() const;
        const String64& GetTitle() const;
        u32 GetWidth() const;
        u32 GetHeight() const;
        bool IsMinimized() const;

        void SetEventCallback(const EventCallback& callback);
    protected:
        Window(const Window::Config& specification);
        virtual void InitializeSwapchain() = 0;
    protected:
        Ref<SwapChain> m_Swapchain;

        bool m_Focused;
        bool m_Minimized;
        EventCallback m_Callback;
        Window::Config m_Config;

        friend class RendererContext;
    };

}
