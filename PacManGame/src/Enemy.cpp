#include "Enemy.h"

#include "EnemyAI.h"

namespace PacMan
{
    namespace Utils
    {
        static f32 Lerp(f32 start, f32 end, f32 maxDistanceDelta)
        {
            if (glm::abs(end - start) <= maxDistanceDelta)
                return end;

            return start + glm::sign(end - start) * maxDistanceDelta;
        }
    }

    // Enemy entity
    // Enemy entity
    // Enemy entity
    EnemyEntity::EnemyEntity(Entity entity)
        : m_Entity(entity), m_Direction(Direction::Up)
    {
    }

    void EnemyEntity::SetAnimation(const void* animation)
    {
        memcpy(m_Animation, animation, sizeof(Ptr<SubTexture2D>) * ENEMY_DIR * ENEMY_ANIM_DIR_FRAMES);
    }

    extern bool g_ChangeDirection = false;

    void EnemyEntity::Follow(const std::list<Entity>& path)
    {
        //m_Path = path;

        for (auto& e : path)
        {
            m_Path.push_back(e);
        }
    }

    void EnemyEntity::OnUpdate(Time_T ts)
    {
        Animate();

        if (m_Path.empty())
            return;

        auto& transform = m_Entity.Transform();
        auto& next = m_Path.front().Transform();

        transform.Translation.x = Utils::Lerp(transform.Translation.x, next.Translation.x, ts * m_Speed);
        transform.Translation.y = Utils::Lerp(transform.Translation.y, next.Translation.y, ts * m_Speed);

        if (transform.Translation.x == next.Translation.x && transform.Translation.y == next.Translation.y)
        {
            m_Path.front().GetComponent<SpriteRendererComponent>().Color.g = 0.0f;
            m_Path.pop_front();
        }
    }

    void EnemyEntity::Animate()
    {
        auto& src = m_Entity.GetComponent<SpriteRendererComponent>();

        if (++m_Frames > 10)
        {
            m_Frames = 0;

            src.SubTexture = m_Animation[(u32)m_Direction][m_AnimationIndex];

            m_AnimationIndex = (m_AnimationIndex + 1) % ENEMY_ANIM_DIR_FRAMES;
        }

        if (g_ChangeDirection)
        {
            m_Frames = 0;

            m_Direction = (Direction)(((u32)m_Direction + 1) % ENEMY_DIR);
        }
    }

    // Enemy Manager
    // Enemy Manager
    // Enemy Manager
    Enemy::Enemy(Scene* scene)
    {
        m_EnemySpriteSheet = Texture2D::Create({ "Assets/Textures/EnemySpritesheet.png" });

        // Create animation for every enemy
        for (size_t i = 0; i < 4; ++i)
        {
            for (size_t j = 0; j < ENEMY_ANIM_FRAMES; ++j)
            {
                m_AnimEnemies[i][j] = SubTexture2D::CreateFromTexture(m_EnemySpriteSheet, { j, i }, { 16, 16 });
            }
        }

        // Create enemies
        for (size_t i = 0; i < 4; ++i)
        {
            Entity e = scene->CreateEntity("Enemy");
            auto& src = e.AddComponent<SpriteRendererComponent>();
            src.SubTexture = m_AnimEnemies[i][0];
            // Create enemy
            m_Enemies[i] = { e };
        }

        // Animation
        Ptr<SubTexture2D> dirAnim[ENEMY_DIR][ENEMY_ANIM_DIR_FRAMES];
        for (size_t i = 0; i < ENEMY_COUNT; ++i)
        {
            u32 index = 0;

            // Set animation according to directions
            for (size_t j = 0; j < ENEMY_DIR; ++j)
            {
                dirAnim[j][0] = m_AnimEnemies[i][index];
                dirAnim[j][1] = m_AnimEnemies[i][index + 1];

                index += 2;
            }

            m_Enemies[i].SetAnimation(dirAnim);
        }
    }

    Enemy::~Enemy()
    {
        for (size_t i = 0; i < ENEMY_COUNT; ++i)
        {
            m_Enemies[i].GetEntity().GetComponent<SpriteRendererComponent>().SubTexture = nullptr;

            for (size_t j = 0; j < ENEMY_ANIM_FRAMES; ++j)
            {
                delete m_AnimEnemies[i][j];
            }
        }

        delete m_EnemySpriteSheet;
    }

    void Enemy::OnGameEvent(GameEvent e, const std::any& data)
    {
        switch (e)
        {
            case GameEvent::EnemyStartEntityRetrieved:
            {
                TBO_ASSERT(m_EnemyIndex < ENEMY_COUNT);

                Entity entity = std::any_cast<Entity>(data);

                auto& transform = m_Enemies[m_EnemyIndex].GetEntity().Transform();
                transform.Translation = entity.Transform().Translation;
                transform.Translation.z = 0.01f;
                transform.Scale *= 0.75;

                m_EnemyGridEntities[m_EnemyIndex] = entity;

                m_EnemyIndex++;
                break;
            }
            case GameEvent::PlayerStartEntityRetrieved:
            case GameEvent::PlayerChangedGridPosition:
            {
                m_UnderPlayerGridEntity = std::any_cast<Entity>(data);
                break;
            }
        }
    }

    void Enemy::OnUpdate(Time_T ts)
    {
        for (u32 i = 0; i < ENEMY_COUNT; i++)
        {
            m_Enemies[i].OnUpdate(ts);

            if (glm::distance(m_UnderPlayerGridEntity.Transform().Translation, m_Enemies[i].GetEntity().Transform().Translation) < 0.40f)
            {
                m_Callback(GameEvent::GameOverPlayerLost, nullptr);
                return;
            }

            if(m_Enemies[i].IsFollowingPath())
                continue;

            std::list<Entity> l = AI::RandomPath(m_EnemyGridEntities[i]);
            for (auto& e : l)
            {
               // e.GetComponent<SpriteRendererComponent>().Color.g = 1.0f;
            }

            if (l.size())
                m_EnemyGridEntities[i] = l.back();
            m_Enemies[i].Follow(l);
        }
    }
}
