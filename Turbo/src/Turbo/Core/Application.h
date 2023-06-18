#pragma once

#include "Turbo/Core/Time.h"
#include "Turbo/Core/Window.h"
#include "Turbo/Event/Event.h"
#include "Turbo/Renderer/DrawList2D.h"

namespace Turbo
{
    class Engine;

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

        const Config& GetConfig() const { return m_Config; }
    protected:
        Application(const Application::Config& config);
        virtual ~Application() = default;

        virtual void OnInitialize() {};
        virtual void OnShutdown() {};
        virtual void OnUpdate() {};
        virtual void OnEvent(Event& event) {};
        virtual void OnDrawUI() {}
    protected:
        Turbo::Time Time;

        Application::Config m_Config;

        friend class Turbo::Engine;
    };

    // Define this in your entrypoint
    Application* CreateApplication();
}
