#pragma once

#include "Turbo/Core/PrimitiveTypes.h"
#include "Turbo/Core/FString.h"

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
            FString64 Title;
            u32 Width;
            u32 Height;
            bool VSync;
            bool StartMaximized;
            bool Resizable;
        };

        virtual ~Window();
        static Window* Create(const Window::Config& specification);

        virtual void ProcessEvents() = 0;
        virtual void Show() = 0;

        virtual void AcquireNewFrame() = 0;
        virtual void SwapFrame() = 0;

        virtual void SetTitle(const FString64& title) = 0;
        bool IsFocused() const;

        Ptr<SwapChain> GetSwapchain() const;
        const FString64& GetTitle() const;
        u32 GetWidth() const;
        u32 GetHeight() const;
        bool IsMinimized() const;

        void SetEventCallback(const EventCallback& callback);
    protected:
        Window(const Window::Config& specification);
        virtual void InitializeSwapchain() = 0;
    protected:
        Ptr<SwapChain> m_Swapchain;

        bool m_Focused;
        bool m_Minimized;
        EventCallback m_Callback;
        Window::Config m_Config;

        friend class RendererContext;
    };

}
