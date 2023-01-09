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
        
        // Create menu
        m_MainMenu = new MainMenu;
        m_MainMenu->SetViewportSize(Window->GetWidth(), Window->GetHeight());

        // Event callback for main menu
        m_MainMenu->SetGameEventCallback(TBO_BIND_FN(Game::OnGameEvent));

        m_CurrentScene = m_MainMenu->GetScene();
    }

    void Game::OnShutdown()
    {
        delete m_MainMenu;
        delete m_Gameplay;
        delete m_Renderer;
    }

    void Game::OnUpdate()
    {
        // Game over screen with simple timer 5s
        if (m_WaitForGameOverScreen)
        {
            m_TimeTillSwitch -= Time.DeltaTime;

            TBO_INFO(m_TimeTillSwitch);

            if (m_TimeTillSwitch < 0.0f)
            {
                m_TimeTillSwitch = 5.0f;
                m_WaitForGameOverScreen = false;

               m_CurrentMode = Mode::MainMenu;

                delete m_Gameplay;
                m_Gameplay = nullptr;

                m_CurrentScene = m_MainMenu->GetScene();
            }
        }

        // Do not update game while in gameover screen
        if (m_WaitForGameOverScreen == false)
        {
            switch (m_CurrentMode)
            {
                case Mode::InGame:  m_Gameplay->OnUpdate(Time.DeltaTime); break;
                case Mode::MainMenu:m_MainMenu->OnUpdate(Time.DeltaTime); break;
            }
        }

        // Render scene
        if (m_CurrentScene)
            m_CurrentScene->OnRuntimeRender(m_Renderer);
    }

    void Game::OnEvent(Event& e)
    {
        // Catching events 
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>(TBO_BIND_FN(Game::OnWindowResize));
        dispatcher.Dispatch<KeyPressedEvent>(TBO_BIND_FN(Game::OnKeyPressedEvent));
    }

    void Game::OnNewGame()
    {
        // Create new scene containing actual game
        m_Gameplay = new Gameplay;
        m_Gameplay->SetViewportSize(Window->GetWidth(), Window->GetHeight());

        // Event callback for game
        m_Gameplay->SetGameEventCallback(TBO_BIND_FN(Game::OnGameEvent));

        m_CurrentMode = Mode::InGame;
        m_CurrentScene = m_Gameplay->GetScene();
    }

    void Game::OnGameEvent(GameEvent e, const std::any& data)
    {
        switch (e)
        {
            case GameEvent::MenuNewGame:
                OnNewGame();
                break;
            case GameEvent::MenuExit:
                Engine->Close();
                break;
            case GameEvent::GameOverPlayerWon:
            case GameEvent::GameOverPlayerLost:
                m_WaitForGameOverScreen = true;
                break;
        }
    }

    bool Game::OnWindowResize(WindowResizeEvent& e)
    {
        // Updates viewport size
        m_CurrentScene->SetViewportSize(e.GetWidth(), e.GetHeight());

        return false;
    }

    bool Game::OnKeyPressedEvent(KeyPressedEvent& e)
    {
        if (m_CurrentMode == Mode::InGame)
            return false;

        return m_MainMenu->OnPressedKeyEvent(e);
    }
}
