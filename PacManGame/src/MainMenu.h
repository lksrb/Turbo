#pragma once

#include "GameEvent.h"

#include <Turbo/Scene/Scene.h>
#include <Turbo/Scene/Entity.h>
#include <Turbo/Event/KeyEvent.h>

#include <Turbo/Renderer/Texture2D.h>

namespace PacMan
{
    using namespace Turbo;

    class MainMenu
    {
    public:
        enum class Option : u32
        {
            Play = 0,
            Exit
        };

        MainMenu();
        ~MainMenu();

        void SetViewportSize(u32 width, u32 height);
        void OnUpdate(Time_T ts);

        bool OnPressedKeyEvent(KeyPressedEvent& e);

        Scene* GetScene() const { return m_Scene; }

        bool WantClose() const { return m_WantClose; }

        bool StartGame() const { return m_SwitchScene; }

        void SetGameEventCallback(const GameEventCallback& callback) { m_Callback = callback; }
    private:
        void OnOptionConfirmed();

        bool m_WantClose = false;
        bool m_SwitchScene = false;

        Texture2D* m_MainMenuTexture;
        Texture2D* m_MainMenuArrowHead;

        Option m_SelectedOption = Option::Play;

        Entity m_ArrowHead;

        Entity m_Camera;

        GameEventCallback m_Callback;

        Scene* m_Scene;
    };
}
