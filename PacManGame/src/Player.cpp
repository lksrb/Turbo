#include "Player.h"

#include <Turbo/Scene/Scene.h>
#include <Turbo/Core/Input.h>
#include <Turbo/Scene/Components.h>

// TODO: Abstract
#include "box2d/b2_body.h"

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

    Player::Player(Entity entity, f32 speed)
        : m_PlayerEntity(entity), m_Speed(speed)
    {
        m_PlayerEntity.Transform().Scale *= 0.95;

        auto& rb = m_PlayerEntity.AddComponent<Rigidbody2DComponent>();
        rb.Gravity = false;
        rb.FixedRotation = true;
        auto& box2d = m_PlayerEntity.AddComponent<CircleCollider2DComponent>();
        box2d.Friction = 0.0f;

        m_PacManSpriteSheet = Texture2D::Create({ "Assets/Textures/PacManSpriteSheet.png" });

        // Animation
        m_Animation[0] = SubTexture2D::CreateFromTexture(m_PacManSpriteSheet, { 0, 0 }, { 32, 32 });
        m_Animation[1] = SubTexture2D::CreateFromTexture(m_PacManSpriteSheet, { 1, 0 }, { 32, 32 });
        m_Animation[2] = SubTexture2D::CreateFromTexture(m_PacManSpriteSheet, { 2, 0 }, { 32, 32 });
        m_Animation[3] = m_Animation[1];

        // Sprite
        auto& src = m_PlayerEntity.AddComponent<SpriteRendererComponent>();
        src.SubTexture = m_Animation[2];
    }

    Player::~Player()
    {
        auto& src = m_PlayerEntity.GetComponent<SpriteRendererComponent>();
        src.SubTexture = nullptr;

        delete m_PacManSpriteSheet;

        for (size_t i = 0; i < 3; i++)
        {
            delete m_Animation[i];
        }
    }

    void Player::OnUpdate(Time_T ts)
    {
        bool isMoving = m_LastPosition != Translation();

        // Movement
        {
            auto& rotation = m_PlayerEntity.Transform().Rotation;

            b2Body* body = reinterpret_cast<b2Body*>(m_PlayerEntity.GetComponent<Rigidbody2DComponent>().RuntimeBody);

            if (Input::IsKeyPressed(Key::Up))
            {
                m_Velocity.x = 0;
                m_Velocity.y = m_Speed;

                if (m_LastKey != Key::Up)
                    rotation.z = glm::radians(90.0f);

                m_LastKey = Key::Up;
            }
            else if (Input::IsKeyPressed(Key::Down))
            {
                m_Velocity.x = 0;
                m_Velocity.y = -m_Speed;

                if (m_LastKey != Key::Down)
                    rotation.z = glm::radians(270.0f);

                m_LastKey = Key::Down;
            }
            else if (Input::IsKeyPressed(Key::Left))
            {
                m_Velocity.x = -m_Speed;
                m_Velocity.y = 0;

                if (m_LastKey != Key::Left)
                    rotation.z = glm::radians(180.0f);

                m_LastKey = Key::Left;
            }
            else if (Input::IsKeyPressed(Key::Right))
            {
                m_Velocity.x = m_Speed;
                m_Velocity.y = 0;

                if (m_LastKey != Key::Right)
                    rotation.z = glm::radians(0.0f);

                m_LastKey = Key::Right;
            }

            body->SetLinearVelocity({ m_Velocity.x, m_Velocity.y });
        }

        m_LastPosition = Translation();

      /*  if (!isMoving)
            return;*/

        // Animation
        {
            static u32 frames = 0;
            frames++;

            if (frames > 8)
            {
                frames = 0;
                m_AnimationFrame = (m_AnimationFrame + 1) % MAX_ANIMATION_FRAMES;
            }

            auto& src = m_PlayerEntity.GetComponent<SpriteRendererComponent>();
            src.SubTexture = m_Animation[m_AnimationFrame];
        }
    }

    void Player::SetStartPosition(const glm::vec3& pos)
    {
        m_PlayerEntity.Transform().Translation = pos;
        // Draw order
        m_PlayerEntity.Transform().Translation.z = 0.01f;
    }
}
