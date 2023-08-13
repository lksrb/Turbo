#pragma once

#include "Turbo/Core/Application.h"
#include "Turbo/Event/WindowEvent.h"

#include "Turbo/Renderer/Renderer.h"
#include "Turbo/Renderer/Renderer.h"
#include "Turbo/Renderer/RendererContext.h"
#include "Turbo/Renderer/SwapChain.h"

#include "Turbo/UI/UserInterface.h"

#include <thread>

namespace Turbo {

    using ApplicationCreateCallback = Application*(*)();

    class Engine
    {
    public:
        Engine(ApplicationCreateCallback callback);
        ~Engine();

        Engine(const Engine& other) = delete;

        static Turbo::Engine& Get()
        {
            TBO_ENGINE_ASSERT(s_Instance, "Engine is not initialized!");
            return *s_Instance;
        }

        void Run();
        void Close();

        Window* GetViewportWindow() const { return m_ViewportWindow; }

        template<typename F>
        void SubmitToMainThread(F&& func)
        {
            std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

            m_MainThreadQueue.emplace_back(func);
        }

        void SetUIBlockEvents(bool blockEvents) { m_UserInterface->SetBlockEvents(blockEvents); }
        bool IsClosing() const { return !m_Running; }

        Application* GetApplication() const { return m_Application; }
    private:
        void Initialize(ApplicationCreateCallback callback);
        void Shutdown();

        void OnEvent(Event& event);

        void ExecuteMainThreadQueue();

        bool WindowResized(WindowResizeEvent& e);
        bool WindowClosed(WindowCloseEvent& e);
    private:
        bool m_Initialized = false;
        bool m_Running = false;

        std::vector<std::function<void()>> m_MainThreadQueue;
        std::mutex m_MainThreadQueueMutex;

        Owned<UserInterface> m_UserInterface;

        // User
        Application* m_Application = nullptr;

        Window* m_ViewportWindow = nullptr;
    private:
        static inline Turbo::Engine* s_Instance;
    };
}
