#include "GamePlayUI.h"

#define UI_WON 0
#define UI_LOST 1

namespace PacMan
{
    GamePlayUI::GamePlayUI(Scene* scene, u32 collectibleCount)
    {
        // UI
        m_ScoreTextTexture = Texture2D::Create({ "Assets/Textures/ScoreText.png" });

        // Numbers
        m_NumbersSpriteSheet = Texture2D::Create({ "Assets/Textures/Numbers.png" });
        for (u32 i = 0; i < UI_MAX_NUM; ++i)
        {
            m_Numbers[i] = SubTexture2D::CreateFromTexture(m_NumbersSpriteSheet, { i, 0 }, { 20, 20 });
        }

        // Score text
        Entity text = scene->CreateEntity();
        text.AddComponent<SpriteRendererComponent>().Texture = m_ScoreTextTexture;
        text.Transform().Scale *= 5.0f;
        text.Transform().Translation.x = -3.0f;
        text.Transform().Translation.y = 8.0f;
        text.Transform().Translation.z = -0.01f;

        // Collected
        for (u32 i = 0; i < UI_MAX_SCORE_NUM; ++i)
        {
            m_CollectedDisplayEntities[i] = scene->CreateEntity();
            m_CollectedDisplayEntities[i].Transform().Translation.x = static_cast<f32>(i) + 1.0f;
            m_CollectedDisplayEntities[i].Transform().Translation.y = 8.0f;
            m_CollectedDisplayEntities[i].Transform().Translation.z = 0.01f;
            auto& src = m_CollectedDisplayEntities[i].AddComponent<SpriteRendererComponent>();
            src.SubTexture = m_Numbers[0];
        }

        // MaxCollected
        for (u32 i = 0; i < UI_MAX_SCORE_NUM; ++i)
        {
            m_MaxCollectedDisplayEntities[i] = scene->CreateEntity();
            m_MaxCollectedDisplayEntities[i].Transform().Translation.x = static_cast<f32>(i) + 5.0f;
            m_MaxCollectedDisplayEntities[i].Transform().Translation.y = 8.0f;
            m_MaxCollectedDisplayEntities[i].Transform().Translation.z = 0.01f;
            auto& src = m_MaxCollectedDisplayEntities[i].AddComponent<SpriteRendererComponent>();

            std::string stringify = std::to_string(collectibleCount);
            stringify.insert(0, UI_MAX_SCORE_NUM - stringify.size(), '0');
            src.SubTexture = GetSubTextureFromString(stringify, i);
        }

        // Gameover screen
        m_GameOverOptions[UI_WON]  = Texture2D::Create({ "Assets/Textures/GameOverWon.png" });
        m_GameOverOptions[UI_LOST] = Texture2D::Create({ "Assets/Textures/GameOverLost.png" });

        m_GameOverEntity = scene->CreateEntity();

        // "/" Symbol
        Entity e = scene->CreateEntity();
        e.AddComponent<SpriteRendererComponent>().SubTexture
            = SubTexture2D::CreateFromTexture(m_NumbersSpriteSheet, { 10,0 }, { 20,20 });
        e.Transform().Translation.x = 4.0f;
        e.Transform().Translation.y = 8.0f;
    }

    GamePlayUI::~GamePlayUI()
    {
        for (size_t i = 0; i < UI_MAX_SCORE_NUM; ++i)
        {
            m_CollectedDisplayEntities[i].GetComponent<SpriteRendererComponent>().SubTexture = nullptr;
            m_MaxCollectedDisplayEntities[i].GetComponent<SpriteRendererComponent>().SubTexture = nullptr;
        }
        if(m_GameOverEntity.HasComponent<SpriteRendererComponent>())
            m_GameOverEntity.GetComponent<SpriteRendererComponent>().Texture = nullptr;

        delete m_GameOverOptions[UI_WON];
        delete m_GameOverOptions[UI_LOST];

        for (size_t i = 0; i < UI_MAX_NUM; ++i)
        {
            delete m_Numbers[i];
        }

        delete m_NumbersSpriteSheet;
    }

    void GamePlayUI::OnUpdate(Time_T ts)
    {

    }

    void GamePlayUI::OnGameEvent(GameEvent e, const std::any& data)
    {
        switch (e)
        {
            case GameEvent::OnCollectibleAcquired:
            {
                // Collected
                u32 collected = std::any_cast<u32>(data);
                std::string stringify = std::to_string(collected);

                stringify.insert(0, UI_MAX_SCORE_NUM - stringify.size(), '0');
                for (u32 i = 0; i < UI_MAX_SCORE_NUM; ++i)
                {
                    auto& src = m_CollectedDisplayEntities[i].GetComponent<SpriteRendererComponent>();
                    src.SubTexture = GetSubTextureFromString(stringify, i);
                }
                break;
            }
            case GameEvent::GameOverPlayerWon:
            {
                m_GameOverEntity.AddComponent<SpriteRendererComponent>().Texture = m_GameOverOptions[UI_WON];
                m_GameOverEntity.Transform().Scale *= 10.0f;
                m_GameOverEntity.Transform().Translation.z = 0.1f;
                break;
            }
            case GameEvent::GameOverPlayerLost:
            {
                m_GameOverEntity.AddComponent <SpriteRendererComponent>().Texture = m_GameOverOptions[UI_LOST];
                m_GameOverEntity.Transform().Scale *= 10.0f;
                m_GameOverEntity.Transform().Translation.z = 0.1f;
                break;
            }
        }
    }

    SubTexture2D* GamePlayUI::GetSubTextureFromString(const std::string& strCount, u32 index)
    {
        u32 singleDigit = strCount[index] - '0';
        return m_Numbers[singleDigit];
    }

}
