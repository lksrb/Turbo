#include "tbopch.h"

#include "Application.h"

#include "Turbo/Core/Platform.h"
#include "Turbo/Core/Window.h"

#include "Turbo/Audio/Audio.h"
#include "Turbo/Physics/Physics.h"
#include "Turbo/Renderer/Renderer.h"
#include "Turbo/Script/Script.h"

#include "Turbo/Event/WindowEvent.h"

#include "Turbo/UI/UserInterfaceLayer.h"

namespace Turbo {

    Application::Application(const Application::Config& config)
        : m_Config(config)
    {
        s_Instance = this;

        // Initialize logging before anything else
        Log::Init();

        Init();
    }

    Application::~Application()
    {
        Shutdown();

        Log::Shutdown();

        s_Instance = nullptr;
    }

    void Application::Init()
    {
        // Create window
        m_ViewportWindow = Window::Create();
        m_ViewportWindow->InitializeSwapChain();
        m_ViewportWindow->SetEventCallback(TBO_BIND_FN(Application::OnEvent));

        // Initialize rendering
        Renderer::Init();

        // Initialize audio engine
        Audio::Init();

        // Initialize mono script engine
        Script::Init();

        // Initialize physics engine
        Physics::Init();

        // If desired, initialize also UI features
        if (m_Config.EnableUI)
        {
            m_UserInterfaceLayer = UserInterfaceLayer::Create();
            PushOverlay(m_UserInterfaceLayer);
        }

        TBO_ENGINE_INFO("Turbo initialized succefully!");
    }

    void Application::Shutdown()
    {
        TBO_ENGINE_WARN("Turbo shutting down...");

        // Wait for GPU to finish work
        Renderer::Wait();

        for (auto layer : m_LayerStack)
        {
            layer->OnDetach();
            delete layer;
        }

        Physics::Shutdown();
        Script::Shutdown();
        Audio::Shutdown();

        Renderer::Shutdown();

        delete m_ViewportWindow;
    }

    void Application::Close()
    {
        m_Running = false;
    }

    void Application::Run()
    {
        m_Running = true;

        OnInit();

        // First resize
        m_ViewportWindow->Show();

        while (m_Running)
        {
            TBO_PROFILE_FRAME("MainThread");

            m_ViewportWindow->ProcessEvents();

            ExecuteMainThreadQueue();

            if (!m_ViewportWindow->IsMinimized())
            {
                Renderer::BeginFrame();

                {
                    TBO_PROFILE_SCOPE("Application::OnUpdate");
                    for (auto layer : m_LayerStack)
                    {
                        layer->OnUpdate(m_CurrentTime);
                    }
                }

                // Render UI
                if (m_Config.EnableUI)
                {
                    Renderer::Submit([this]() { m_UserInterfaceLayer->Begin(); });
                    Renderer::Submit([this]() 
                    {  
                        for (auto layer : m_LayerStack) 
                            layer->OnDrawUI();
                    });
                    Renderer::Submit([this]() { m_UserInterfaceLayer->End(); });
                }

                // TODO: To be on render thread
                {
                    TBO_PROFILE_SCOPE("Renderer::Render");
                    m_ViewportWindow->AcquireNewFrame();
                    Renderer::Render();
                    m_ViewportWindow->SwapFrame();
                }
            }

            f32 currentFrame = Platform::GetTime();
            // Minimal frame is at 0.0333f - Avoid large timesteps when minimized
            m_CurrentTime.DeltaTime = glm::min(currentFrame - m_LastFrameTime, 0.0333f);
            m_CurrentTime.TimeSinceStart += currentFrame;
            m_LastFrameTime = currentFrame;
        }
    }

    void Application::OnEvent(Event& e)
    {
        // Handle resizing
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>(TBO_BIND_FN(Application::WindowResized));
        dispatcher.Dispatch<WindowCloseEvent>(TBO_BIND_FN(Application::WindowClosed));

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            (*it)->OnEvent(e);

            if (e.Handled)
                break;
        }
    }

    void Application::ExecuteMainThreadQueue()
    {
        TBO_PROFILE_FUNC();

        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        for (const auto& func : m_MainThreadQueue)
            func();

        m_MainThreadQueue.clear();
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    }

    bool Application::WindowResized(WindowResizeEvent& e)
    {
        if (!m_ViewportWindow->IsMinimized())
        {
            TBO_ENGINE_TRACE("Window resized! {0}, {1}", e.GetWidth(), e.GetHeight());
        }

        return false;
    }

    bool Application::WindowClosed(WindowCloseEvent& e)
    {
        Close();
        return false;
    }
}

