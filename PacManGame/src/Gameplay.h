#pragma once

#include "Player.h"
#include "Grid.h"
#include "Enemy.h"
#include "GamePlayUI.h"

#include <Turbo/Scene/Scene.h>
#include <Turbo/Scene/Entity.h>

namespace PacMan
{
    using namespace Turbo;

    class Gameplay
    {
    public:
        Gameplay();
        ~Gameplay();

        void SetViewportSize(u32 width, u32 height);
        void OnUpdate(Time_T ts);

        Scene* GetScene() const { return m_Scene; }
        
        void SetGameEventCallback(const GameEventCallback& callback) { m_Callback = callback; }

        void OnGameEvent(GameEvent e, const std::any& data);
    private:
        GameEventCallback m_Callback;
        u32 m_Collected = 0, m_LootMaxCount;

        Time_T m_TimeStep;

        Entity m_EntityUnderPlayer;
        Player* m_Player;
        Grid* m_Grid;
        Enemy* m_EnemyManager;

        Entity m_Camera;

        GamePlayUI* m_UI;

        Scene* m_Scene;
    };
}
