#pragma region Headers

#include "tbopch.h"

#include "Engine.h"

#include "Turbo/Core/Platform.h"

#include "Turbo/Renderer/Renderer.h"
#include "Turbo/Script/Script.h"

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
        Platform::Initialize();

        // Initialize render context (VulkanContext)
        RendererContext::Initialize();

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

        // Creates Win32 surface and initializes swapchain
        RendererContext::SetWindowContext(m_ViewportWindow);

        // Initializes rendering
        Renderer::Initialize();

        // Initialize mono script engine
        Script::Init();

        if(m_Application->m_Config.EnableUI)
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

        f32 last_frame = 0.0f;

        while (m_Running)
        {
            f32 current_frame = Platform::GetTime();
            m_Application->Time.DeltaTime = current_frame - last_frame;
            m_Application->Time.TimeSinceStart += m_Application->Time.DeltaTime;
            last_frame = current_frame;

            m_ViewportWindow->ProcessEvents();

            if (!m_ViewportWindow->IsMinimized())
            {
                if (m_Application)
                    m_Application->OnUpdate();

                // Wait for render thread
                if (m_RenderThread.joinable())
                    m_RenderThread.join();

                // Main renderer
                Renderer::Begin();

                // Render2D
                Renderer::Submit([this]() { m_Application->OnDraw(); });

                // Render UI
                if(m_Application->m_Config.EnableUI)
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
                //);
            }
        }
        // On Exit

        // Wait for render thread
        if (m_RenderThread.joinable())
            m_RenderThread.join();

        m_Application->OnShutdown();
    }

    void Engine::OnEvent(Event& e)
    {
        // Handle resizing
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>(TBO_BIND_FN(Engine::WindowResized));
        dispatcher.Dispatch<WindowCloseEvent>(TBO_BIND_FN(Engine::WindowClosed));

        if(m_Application->m_Config.EnableUI)
            m_UI->OnEvent(e);

        if (e.Handled == false)
            m_Application->OnEvent(e);
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

