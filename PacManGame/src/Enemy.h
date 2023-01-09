#pragma once

#include "Grid.h"

#define ENEMY_COUNT 4
#define ENEMY_ANIM_FRAMES 8
#define ENEMY_ANIM_DIR_FRAMES 2
#define ENEMY_DIR 4

#include <list>

namespace PacMan
{
    class EnemyEntity
    {
    public:
        EnemyEntity(Entity entity = {});
        ~EnemyEntity() {};

        void SetAnimation(const void* animation);

        Entity GetEntity() const { return m_Entity; }

        void Follow(const std::list<Entity>& path);

        void OnUpdate(Time_T ts);

        bool IsFollowingPath() const { return !m_Path.empty(); }
    private:
        void Animate();
    private:
        u32 m_Frames = 0;
        u32 m_AnimationIndex = 0;

        Ptr<SubTexture2D> m_Animation[ENEMY_DIR][ENEMY_ANIM_DIR_FRAMES];

        std::list<Entity> m_Path;
        f32 m_Speed = 5.0f;
        Direction m_Direction;
        Entity m_Entity;
    };

    class Enemy
    {
    public:
        Enemy(Scene* scene);
        ~Enemy();

        void OnGameEvent(GameEvent e, const std::any& data);

        void OnUpdate(Time_T ts);

        void SetGameEventCallback(const GameEventCallback& callback) { m_Callback = callback; };
    private:
        GameEventCallback m_Callback;

        u32 m_EnemyIndex = 0;

        Entity m_UnderPlayerGridEntity;

        Ptr<Texture2D> m_EnemySpriteSheet;

        Ptr<SubTexture2D> m_AnimEnemies[ENEMY_COUNT][ENEMY_ANIM_FRAMES];

        Entity m_EnemyGridEntities[ENEMY_COUNT];

        EnemyEntity m_Enemies[ENEMY_COUNT];
    };
}
