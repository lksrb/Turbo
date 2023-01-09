#include "Gameplay.h"

#include "Turbo/Scene/Components.h"
#include "Turbo/Core/Input.h"

#include "Turbo/Benchmark/ScopeTimer.h"

#include <imgui.h>

namespace PacMan
{
    Gameplay::Gameplay()
    {
        m_Scene = new Scene({ "Gameplay" });

        // Create player
        m_Player = new Player(m_Scene->CreateEntity(), 5.0f);

        // Create enemy manager
        m_EnemyManager = new Enemy(m_Scene);
        m_EnemyManager->SetGameEventCallback(TBO_BIND_FN(Gameplay::OnGameEvent));

        // Generate grid from file
        Grid::Config config = {};
        config.Height = 13;
        config.Width = 17;
        config.CurrentScene = m_Scene;

        {
            std::string& level = config.MapSkeleton;
            level += "#################";
            level += "#--------------P#";
            level += "#-##-#######-##-#";
            level += "#-#-----------#-#";
            level += "#-#-#-##-##-#-#-#";
            level += "#-----#---#-----#";
            level += "#-#-#-##-##-#-#-#";
            level += "#---------------#";
            level += "#-#-####-####-#-#";
            level += "#E-------------E#";
            level += "#-###-##-##-###-#";
            level += "#E-------------E#";
            level += "#################";
            std::reverse(level.begin(), level.end());
        }

        // Create grid
        m_Grid = new Grid(config);

        // Set callback for grid to call events
        m_Grid->SetGameEventCallback(TBO_BIND_FN(Gameplay::OnGameEvent));

        // Generate map from skeleton
        m_Grid->Generate();

        // Get loot count
        m_LootMaxCount = static_cast<u32>(m_Scene->GetAllEntitiesWith<LootComponent>().size());

        // Create ingame UI
        m_UI = new GamePlayUI(m_Scene, m_LootMaxCount);

        // Create camera entity
        m_Camera = m_Scene->CreateEntity();
        auto& camera = m_Camera.AddComponent<CameraComponent>();
        camera.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
        m_Camera.Transform().Translation.z = 23.0f;

        // Physics start
        m_Scene->OnRuntimeStart();
    }

    Gameplay::~Gameplay()
    {
        m_Scene->OnRuntimeStop();

        // TODO: Reference counting
        delete m_UI;
        delete m_EnemyManager;
        delete m_Player;
        delete m_Grid;
        delete m_Scene;
    }

    void Gameplay::SetViewportSize(u32 width, u32 height)
    {
        m_Scene->SetViewportSize(width, height);
    }

    void Gameplay::OnUpdate(Time_T ts)
    {
        m_TimeStep = ts;

        // Update player movement
        m_Player->OnUpdate(ts);

        // Update UI
        m_UI->OnUpdate(ts);
        
        // Checking loot entities
        for (auto& e : m_EntityUnderPlayer.GetComponent<TileComponent>().Neighbours)
        {
            Entity lootEntity = e.Entity.GetComponent<TileComponent>().LootEntity;

            if (lootEntity)
            {
                auto& [transform, src, loot] = lootEntity.GetComponents<TransformComponent, SpriteRendererComponent, LootComponent>();

                if (!loot.Looted && glm::distance(transform.Translation, m_Player->Translation()) < 0.50f)
                {
                    loot.Looted = true;
                    src.Color = { 0.0f, 0.0f, 0.0f, 1.0f };
                    m_Collected++;

                    OnGameEvent(GameEvent::OnCollectibleAcquired, m_Collected);
                }
            }
        }

        // Updating players position
        for (auto& e : m_EntityUnderPlayer.GetComponent<TileComponent>().Neighbours)
        {
            if (glm::distance(e.Entity.Transform().Translation, m_Player->Translation()) < 0.50f)
            {
                m_EntityUnderPlayer = e.Entity;

                // Dispatch
                OnGameEvent(GameEvent::PlayerChangedGridPosition, e.Entity);
                break;
            }
        }

        // Update enemies
        m_EnemyManager->OnUpdate(ts);

        // Update physics
        m_Scene->OnRuntimeUpdate(ts);

        // Check if player won
        if (m_Collected == m_LootMaxCount)
        {
            m_Collected++;
            OnGameEvent(GameEvent::GameOverPlayerWon, nullptr);
        }
    }

    void Gameplay::OnGameEvent(GameEvent e, const std::any& data)
    {
        switch (e)
        {
            case GameEvent::PlayerStartEntityRetrieved:
            {
                Entity entity = std::any_cast<Entity>(data);
                m_EntityUnderPlayer = entity;
                m_Player->SetStartPosition(entity.Transform().Translation);
                break;
            }
            case GameEvent::GameOverPlayerWon:
            case GameEvent::GameOverPlayerLost:
            {
                m_Callback(e, nullptr);
                break;
            }
        }

        m_UI->OnGameEvent(e, data);
        m_EnemyManager->OnGameEvent(e, data);
    }
}
