#include "Game.h"

namespace PacMan
{
    Game::Game(const Application::Config& config)
        : Application(config)
    {
    }

    void Game::OnInitialize()
    {
        // Create scene renderer
        m_Renderer = new SceneRenderer;
        
        // Create new scene containing actual game
        m_CurrentScene = new Scene({ "InGame" });
        m_CurrentScene->SetViewportSize(Window->GetWidth(), Window->GetHeight());

        m_CurrentScene->CreateEntity().AddComponent<CameraComponent>();
        m_CurrentScene->CreateEntity().AddComponent<SpriteRendererComponent>();
    }

    void Game::OnShutdown()
    {
        delete m_CurrentScene;
        delete m_Renderer;
    }

    void Game::OnUpdate()
    {
    }

    void Game::OnDraw()
    {
        m_CurrentScene->OnRuntimeRender(m_Renderer);
    }

    void Game::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>(TBO_BIND_FN(Game::OnWindowResize));
    }

    void Game::OnDrawUI()
    {
    }

    bool Game::OnWindowResize(WindowResizeEvent& e)
    {
        // Updates viewport size
        m_CurrentScene->SetViewportSize(e.GetWidth(), e.GetHeight());

        return false;
    }

}
