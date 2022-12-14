#pragma once

#include "Turbo/Core/Application.h"
#include "Turbo/Event/WindowEvent.h"

#include "Turbo/Renderer/Renderer.h"
#include "Turbo/Renderer/Renderer.h"
#include "Turbo/Renderer/RendererContext.h"
#include "Turbo/Renderer/SwapChain.h"

#include "Turbo/UI/UserInterface.h"

#include <thread>

namespace Turbo 
{
    class Engine 
    {
    public:
        static Turbo::Engine* Create(ApplicationCreateCallback callback)
        {
            TBO_ENGINE_ASSERT(!s_Instance, "Engine already running!");
            s_Instance = new Turbo::Engine(callback);
            s_Instance->Initialize();
            return s_Instance;
        }
        ~Engine();

        Engine(const Engine& other) = delete;

        static Turbo::Engine& Get()
        {
            TBO_ENGINE_ASSERT(s_Instance, "Engine is not running!");
            return *s_Instance;
        }

        void Run();
        void Close();

        Window* GetViewportWindow() const { return m_ViewportWindow; }
    private:
        Engine(ApplicationCreateCallback callback);

        void Initialize();
        void Shutdown();

        void OnEvent(Event& event);

        bool WindowResized(WindowResizeEvent& e);
        bool WindowClosed(WindowCloseEvent& e);
    private:
        bool m_Initialized;
        bool m_Running;

        std::thread m_RenderThread;

        UserInterface* m_UI;
        Ptr<Renderer> m_Renderer;

        // User
        Application* m_Application;
        ApplicationCreateCallback m_ApplicationCreateCallback;

        Window* m_ViewportWindow;

    private:
        static inline Turbo::Engine* s_Instance;

        friend class Window;
#ifdef TBO_PLATFORM_WIN32
        friend class Win32_Window;
#endif
    };
}
