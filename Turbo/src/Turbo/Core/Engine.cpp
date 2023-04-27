#pragma region Headers

#include "tbopch.h"

#include "Engine.h"

#include "Turbo/Core/Platform.h"

#include "Turbo/Renderer/Renderer.h"
#include "Turbo/Script/Script.h"
#include "Turbo/Audio/Audio.h"

#include <filesystem>
#include <future>

#pragma endregion

namespace Turbo
{
    Engine::Engine(ApplicationCreateCallback callback)
        : m_ApplicationCreateCallback(callback)
    {
        Log::Initialize();
    }

    Engine::~Engine()
    {
        Shutdown();

        Log::Shutdown();
    }

    void Engine::Initialize()
    {
        TBO_ASSERT(m_Initialized == false);

        // Call client function
        m_Application = m_ApplicationCreateCallback();

        // Initialize platform (Timers, dialogs, ...)
        Platform::Init();

        // Initialize render context (VulkanContext)
        RendererContext::Init();

        // Create window 
        Window::Config config;
        config.Title = m_Application->m_Config.Title;
        config.Width = m_Application->m_Config.Width;
        config.Height = m_Application->m_Config.Height;
        config.VSync = m_Application->m_Config.VSync;
        config.StartMaximized = m_Application->m_Config.StartMaximized;
        config.Resizable = m_Application->m_Config.Resizable;

        // TODO: If UI is disabled, change render target to swapchain framebuffers i.e. render into the window instead of the UI
        config.SwapChainTarget = !m_Application->m_Config.EnableUI;

        m_ViewportWindow = Window::Create(config);
        m_ViewportWindow->SetEventCallback(TBO_BIND_FN(Engine::OnEvent));

        // Create Win32 surface and initializes swapchain
        RendererContext::SetWindowContext(m_ViewportWindow);

        // Initialize rendering
        Renderer::Init();

        // Initialize mono script engine
        Script::Init();

        // Initialize audio engine
        Audio::Init();

        if (m_Application->m_Config.EnableUI)
            m_UI = UserInterface::Create();

        // Client access
        m_Application->Engine = this;
        m_Application->Window = m_ViewportWindow;

        m_Initialized = true;

        TBO_ENGINE_INFO("Engine initialized succefully!");
    }

    void Engine::Shutdown()
    {
        m_Initialized = false;

        Audio::Shutdown();
        Script::Shutdown();
        RendererContext::WaitIdle();

        delete m_ViewportWindow;
        delete m_Application;

        Renderer::Shutdown();
        m_UI.Reset();


        RendererContext::Shutdown();

        Platform::Shutdown();

        TBO_ENGINE_WARN("Engine is shutting down!");
    }

    void Engine::Close()
    {
        m_Running = false;
    }

    void Engine::Run()
    {
        m_Running = true;

        m_Application->OnInitialize();

        // First resize
        m_ViewportWindow->Show();

        f32 lastFrame = 0.0f;

        while (m_Running)
        {
            f32 currentFrame = Platform::GetTime();
            m_Application->Time.DeltaTime = currentFrame - lastFrame;
            m_Application->Time.TimeSinceStart += m_Application->Time.DeltaTime;
            lastFrame = currentFrame;

            m_ViewportWindow->ProcessEvents();

            ExecuteMainThreadQueue();

            if (!m_ViewportWindow->IsMinimized())
            {
                m_Application->OnUpdate();

                Renderer::BeginFrame();
                Renderer::Submit([this]() { m_Application->OnDraw(); });

                // Render UI
                if (m_Application->m_Config.EnableUI)
                {
                    Renderer::Submit([this]() { m_UI->BeginUI(); });
                    Renderer::Submit([this]() { m_Application->OnDrawUI(); });
                    Renderer::Submit([this]() { m_UI->EndUI(); });
                }

                // TODO: To be on render thread
                {
                    m_ViewportWindow->AcquireNewFrame();
                    Renderer::Render();
                    m_ViewportWindow->SwapFrame();
                }
                
            }
        }

        // On Exit
        m_Application->OnShutdown();
    }

    void Engine::OnEvent(Event& e)
    {
        // Handle resizing
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>(TBO_BIND_FN(Engine::WindowResized));
        dispatcher.Dispatch<WindowCloseEvent>(TBO_BIND_FN(Engine::WindowClosed));

        if (m_Application->m_Config.EnableUI)
            m_UI->OnEvent(e);

        if (e.Handled == false)
            m_Application->OnEvent(e);
    }

    void Engine::ExecuteMainThreadQueue()
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        for (const auto& func : m_MainThreadQueue)
            func();

        m_MainThreadQueue.clear();
    }

    bool Engine::WindowResized(WindowResizeEvent& e)
    {
        if (!m_ViewportWindow->IsMinimized())
        {
            TBO_ENGINE_TRACE("Window resized! {0}, {1}", e.GetWidth(), e.GetHeight());
        }
        return false;
    }

    bool Engine::WindowClosed(WindowCloseEvent& e)
    {
        Close();
        return false;
    }
}

