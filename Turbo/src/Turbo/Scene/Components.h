#pragma once

#include "Turbo/Core/PrimitiveTypes.h"
#include "Turbo/Core/FString.h"
#include "Turbo/Core/UUID.h"

#include "Turbo/Renderer/Texture2D.h"

#include "Turbo/Scene/SceneCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Turbo
{
    class Entity;

    struct TagComponent
    {
        FString64 Tag;

        TagComponent() = default;
        TagComponent(const FString64& tagName) : Tag(tagName) {}
    };

    struct RelationshipComponent
    {
        UUID Parent;
        std::vector<UUID> Children;
    };

    struct IDComponent
    {
        UUID Uuid;

        IDComponent(const UUID& uuid) : Uuid(uuid) {}
    };

    struct TransformComponent
    {
        glm::vec3 Translation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };

        glm::mat4 GetMatrix() const
        {
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

            return glm::translate(glm::mat4(1.0f), Translation)
                * rotation
                * glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    struct CameraComponent
    {
        SceneCamera Camera;
        bool Primary = true;
        bool FixedAspectRatio = false;
    };

    struct SpriteRendererComponent
    {
        glm::vec4 Color{ 1.0f };
        f32 Tiling{ 1.0f };
        Ptr<Texture2D> Texture; // TODO: Combine texture and subtexture
        Ptr<SubTexture2D> SubTexture;

        ~SpriteRendererComponent()
        {
            delete SubTexture;
            delete Texture;
        }
    };

    // Physics
    struct Rigidbody2DComponent
    {
        enum class BodyType { Static = 0, Dynamic, Kinematic };
        BodyType Type = BodyType::Dynamic;
        bool FixedRotation = false;
        bool Gravity = true;
        bool Enabled = true;

        // Storage for runtime
        void* RuntimeBody = nullptr;
    };

    struct BoxCollider2DComponent
    {
        glm::vec2 Offset = { 0.0f, 0.0f };
        glm::vec2 Size = { 0.5f, 0.5f };

        f32 Density = 1.0f;
        f32 Friction = 0.5f;
        f32 Restitution = 0.0f;
        f32 RestitutionThreshold = 0.5f;
        bool IsSensor = false;
    };

    struct CircleCollider2DComponent
    {
        glm::vec2 Offset = { 0.0f, 0.0f };
        f32 Radius = 0.5f;

        f32 Density = 1.0f;
        f32 Friction = 0.5f;
        f32 Restitution = 0.0f;
        f32 RestitutionThreshold = 0.5f;
        bool IsSensor = false;
    };
}
