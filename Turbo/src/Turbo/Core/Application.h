#pragma once

#include "Memory.h"
#include "LayerStack.h"

#include <mutex>

namespace Turbo {

    class Window;
    class UserInterfaceLayer;
    class WindowResizeEvent;
    class WindowCloseEvent;

    class Application
    {
    public:
        struct Config
        {
            std::string Title;
            u32 Width;
            u32 Height;
            bool VSync;
            bool Resizable;
            bool StartMaximized;
            bool EnableUI;
        };

        Application(const Application::Config& config);
        virtual ~Application();

        virtual void OnInit() = 0;

        Application(const Application& other) = delete;

        static Turbo::Application& Get()
        {
            TBO_ENGINE_ASSERT(s_Instance, "Application is not initialized!");
            return *s_Instance;
        }

        void Run();
        void Close();

        UserInterfaceLayer* GetUserInterfaceLayer() const { return m_UserInterfaceLayer; }
        OwnedRef<Window> GetViewportWindow() const { return m_ViewportWindow; }

        template<typename F>
        void SubmitToMainThread(F&& func)
        {
            std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

            m_MainThreadQueue.emplace_back(func);
        }

        bool IsClosing() const { return !m_Running; }

        const Application::Config& GetConfig() const { return m_Config; }
    protected:
        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
    private:
        void Init();
        void Shutdown();

        void OnEvent(Event& event);

        void ExecuteMainThreadQueue();

        bool WindowResized(WindowResizeEvent& e);
        bool WindowClosed(WindowCloseEvent& e);
    private:
        bool m_Running = false;

        std::vector<std::function<void()>> m_MainThreadQueue;
        std::mutex m_MainThreadQueueMutex;

        Time m_CurrentTime;
        f32 m_LastFrameTime = 0.0f;

        UserInterfaceLayer* m_UserInterfaceLayer = nullptr;
        Owned<Window> m_ViewportWindow;

        LayerStack m_LayerStack;
        Application::Config m_Config;
    private:
        static inline Turbo::Application* s_Instance;
    };

    // Define this in your entrypoint
    Application* CreateApplication();
}
