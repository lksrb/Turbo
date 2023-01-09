#pragma once

#include "GameEvent.h"

#include <Turbo/Scene/Entity.h>
#include <Turbo/Renderer/Texture2D.h>

#define UI_MAX_SCORE_NUM 3
#define UI_MAX_NUM 10

namespace PacMan
{
    using namespace Turbo;

    class GamePlayUI
    {
    public:
        GamePlayUI(Scene* scene, u32 collectibleCount);
        ~GamePlayUI();

        void OnGameEvent(GameEvent e, const std::any& data);
    private:
        SubTexture2D* GetSubTextureFromString(const std::string& strCount, u32 index);

        Entity m_CollectedDisplayEntities[UI_MAX_SCORE_NUM];
        Entity m_MaxCollectedDisplayEntities[UI_MAX_SCORE_NUM];
        
        Entity m_GameOverEntity;

        Texture2D* m_GameOverOptions[2];
        SubTexture2D* m_Numbers[UI_MAX_NUM];
        Texture2D* m_ScoreTextTexture;
        Texture2D* m_NumbersSpriteSheet;
    };
}
