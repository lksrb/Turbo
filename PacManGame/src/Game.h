#pragma once

#include "Gameplay.h"
#include "MainMenu.h"

#include <Turbo.h>

namespace PacMan
{
    using namespace Turbo;

    class Game : public Application
    {
    public:
        enum class Mode : u32
        {
            MainMenu = 0,
            InGame
        };

        Game(const Application::Config& config);
    protected:
        void OnInitialize() override;
        void OnShutdown() override;
        void OnUpdate() override;
        void OnEvent(Event& e) override;
    private:
        void OnNewGame();
        void OnGameEvent(GameEvent e, const std::any& data);
        bool OnWindowResize(WindowResizeEvent& e);
        bool OnKeyPressedEvent(KeyPressedEvent& e);
    private:
        bool m_WaitForGameOverScreen = false;
        f32 m_TimeTillSwitch = 5.0f;

        Mode m_CurrentMode = Mode::MainMenu;

        Gameplay* m_Gameplay = nullptr;
        MainMenu* m_MainMenu = nullptr;

        SceneRenderer* m_Renderer = nullptr;
        Scene* m_CurrentScene = nullptr;
    };
}
