#pragma once

#include "Turbo/Core/PrimitiveTypes.h"
#include "Turbo/Core/UUID.h"

#include "Turbo/Renderer/Texture2D.h"
#include "Turbo/Renderer/Font.h"

#include "Turbo/Audio/AudioClip.h"

#include "Turbo/Scene/SceneCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Turbo/Core/Math.h"

namespace Turbo
{
    class Entity;

    struct TagComponent
    {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const std::string& tag) : Tag(tag) {}
    };

    struct RelationshipComponent
    {
        UUID Parent = 0;
        std::vector<UUID> Children;

        RelationshipComponent() = default;
        RelationshipComponent(const RelationshipComponent&) = default;
    };

    struct IDComponent
    {
        UUID ID;

        IDComponent() = default;
        IDComponent(const UUID& id) : ID(id) {}
        IDComponent(const IDComponent&) = default;
    };

    struct TransformComponent
    {
        glm::vec3 Translation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;

        inline glm::mat4 GetTransform() const
        {
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

            return glm::translate(glm::mat4(1.0f), Translation)
                * rotation
                * glm::scale(glm::mat4(1.0f), Scale);
        }

        inline void SetTransform(const glm::mat4& transform)
        {
            Math::DecomposeTransform(transform, Translation, Rotation, Scale);
        }
    };

    struct AudioSourceComponent
    {
        std::string AudioPath; // TODO: Put this in asset manager in the future

        f32 Gain = 1.0f;
        bool Spatial = false;
        bool PlayOnAwake = false;
        bool Loop = false;

        AudioSourceComponent() = default;
        AudioSourceComponent(const AudioSourceComponent&) = default;
    };

    struct AudioListenerComponent
    {
        bool IsPrimary = true;

        AudioListenerComponent() = default;
        AudioListenerComponent(const AudioListenerComponent&) = default;
    };

    struct CameraComponent
    {
        SceneCamera Camera;
        bool IsPrimary = true;
        bool FixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };

    struct LineRendererComponent
    {
        glm::vec3 Position0{ 0.0f };
        glm::vec3 Position1{ 0.0f };
        glm::vec4 Color{ 1.0f };

        LineRendererComponent() = default;
        LineRendererComponent(const LineRendererComponent&) = default;
    };

    struct SpriteRendererComponent
    {
        glm::vec4 Color{ 1.0f };
        f32 Tiling = 1.0f;
        Ref<SubTexture2D> SubTexture;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
    };

    struct CircleRendererComponent
    {
        glm::vec4 Color{ 1.0f };
        f32 Thickness = 1.0f;
        f32 Fade = 0.005f;

        CircleRendererComponent() = default;
        CircleRendererComponent(const CircleRendererComponent&) = default;
    };

    struct StaticMeshRendererComponent
    {
        enum class PrimitiveType : u32 { Cube, Sphere, Cylinder };

        PrimitiveType Type;

        StaticMeshRendererComponent() = default;
        StaticMeshRendererComponent(const StaticMeshRendererComponent&) = default;
    };

    struct TextComponent
    {
        std::string Text;
        glm::vec4 Color{ 1.0f };
        f32 KerningOffset = 0.0f;
        f32 LineSpacing = 0.0f;
        Ref<Font> FontAsset = Font::GetDefaultFont();

        TextComponent() = default;
        TextComponent(const TextComponent&) = default;
    };

    // TODO: Think about what belongs here
    struct PointLightComponent
    {
        f32 Intensity = 1.0f;
        f32 Radius = 10.0f;
        f32 FallOff = 1.0f;
    };

    struct ScriptComponent
    {
        std::string ClassName;

        ScriptComponent() = default;
        ScriptComponent(const std::string& className) : ClassName(className) {};
        ScriptComponent(const ScriptComponent&) = default;
    };

    // Physics 2D
    struct Rigidbody2DComponent
    {
        enum class BodyType : u32 { Static = 0, Dynamic, Kinematic };
        BodyType Type = BodyType::Dynamic;
        bool FixedRotation = false;
        f32 GravityScale = 1.0f;
        bool Enabled = true;
        bool ContactEnabled = true;
        bool IsBullet = false; // Continous collision detection

        // Storage for runtime
        void* RuntimeBody = nullptr;

        Rigidbody2DComponent() = default;
        Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
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
        u16 CollisionCategory = 0x0001; // What category is this entity
        u16 CollisionMask = 0xFFFF; // What other categories will this collide with

        BoxCollider2DComponent() = default;
        BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
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
        u16 CollisionCategory = 0x0001; // What category is this entity
        u16 CollisionMask = 0xFFFF; // What other categories will this collide with

        CircleCollider2DComponent() = default;
        CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
    };

    template<typename... Components>
    struct ComponentGroup
    {
        constexpr static size_t Size = sizeof...(Components);
    };

    using AllComponents =
        ComponentGroup<TransformComponent, RelationshipComponent, CameraComponent, LineRendererComponent, SpriteRendererComponent, CircleRendererComponent, TextComponent, ScriptComponent,
        AudioSourceComponent, AudioListenerComponent, Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent>;
}
