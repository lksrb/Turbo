#pragma once

#include "Turbo/Core/Time.h"
#include "Turbo/Core/Window.h"
#include "Turbo/Event/Event.h"
#include "Turbo/Renderer/Renderer2D.h"

namespace Turbo
{
    class Application;
    class Engine;

    using ApplicationCreateCallback = Application*(*)();

    class Application
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
            bool EnableUI;
        };
    protected:
        Application(const Application::Config& config);
        virtual ~Application() = default;

        virtual void OnInitialize() {};
        virtual void OnShutdown() {};
        virtual void OnUpdate() {};
        virtual void OnDraw() {};
        virtual void OnEvent(Event& event) {};
        virtual void OnDrawUI() {}
    protected:
        Turbo::Engine* Engine = nullptr;
        Turbo::Window* Window = nullptr;
        Turbo::Time Time;

        Application::Config m_Config;

        friend class Turbo::Window;
        friend class Win32_Window;
        friend class Turbo::Engine;
    };

    // Define this in your entrypoint
    Application* CreateApplication();
}
