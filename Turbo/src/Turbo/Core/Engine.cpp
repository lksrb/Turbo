#pragma region Headers

#include "tbopch.h"

#include "Engine.h"

#include "Turbo/Core/Platform.h"

#include "Turbo/Renderer/Renderer.h"

#include <filesystem>
#include <future>

#pragma endregion

namespace Turbo
{
    Engine::Engine(ApplicationCreateCallback callback)
        : m_ApplicationCreateCallback(callback), m_Initialized(false), m_Running(false), m_ViewportWindow(nullptr), m_Application(nullptr), m_UI(nullptr)
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

        // Initialize platform
        Platform::Initialize();
        RendererContext::Initialize();

        // Create window 
        Window::Config specification;
        specification.Title = m_Application->m_Config.Title;
        specification.Width = m_Application->m_Config.Width;
        specification.Height = m_Application->m_Config.Height;
        specification.VSync = m_Application->m_Config.VSync;
        specification.StartMaximized = m_Application->m_Config.StartMaximized;
        specification.Resizable = m_Application->m_Config.Resizable;

        m_ViewportWindow = Window::Create(specification);
        m_ViewportWindow->SetEventCallback(TBO_BIND_FN(Engine::OnEvent));

        // Set window context
        RendererContext::SetWindowContext(m_ViewportWindow);

        Renderer::Initialize();

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

        RendererContext::WaitIdle();

        delete m_ViewportWindow;
        delete m_Application;

        Renderer::Shutdown();
        delete m_UI;

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

