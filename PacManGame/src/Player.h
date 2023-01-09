#pragma once

#include <Turbo/Scene/Entity.h>
#include <Turbo/Core/KeyCodes.h>

#include <Turbo/Renderer/Texture2D.h>

#define MAX_ANIMATION_FRAMES 4

namespace PacMan
{
    using namespace Turbo;

    enum class Direction : u32
    {
        Up = 0,
        Down,
        Left,
        Right,
        None
    };

    class Player
    {
    public:
        Player(Entity entity, f32 speed);
        ~Player();

        void SetSpeed(f32 speed) { m_Speed = speed; }

        void OnUpdate(Time_T ts);

        void SetStartPosition(const glm::vec3& pos);

        Direction GetDirection() const { return m_CurrentDirection; }

        glm::vec3 Translation() { return m_PlayerEntity.Transform().Translation; }
    private:
        glm::vec2 m_Velocity = {};

        Ptr<SubTexture2D> m_Animation[MAX_ANIMATION_FRAMES];
        glm::vec3 m_LastPosition = {};

        Ptr<Texture2D> m_PacManSpriteSheet;
        u32 m_AnimationFrame = 0;

        KeyCode m_LastKey = 0;
        Direction m_CurrentDirection = Direction::None;
        f32 m_Speed;
        Entity m_PlayerEntity;
    };
}
