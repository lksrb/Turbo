#pragma once

#include "Turbo/Core/PrimitiveTypes.h"
#include "Turbo/Core/UUID.h"

#include "Turbo/Asset/Asset.h"

#include "Turbo/Renderer/Font.h"

#include "Turbo/Audio/AudioClip.h"

#include "Turbo/Scene/SceneCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Turbo/Core/Math.h"

namespace Turbo {

    // Matches b2BodyType and Jolt's EMotionType
    enum class RigidbodyType : u32 { Static = 0, Kinematic, Dynamic };
    enum class CollisionDetectionType : u32 { Discrete = 0, LinearCast };

    using BodyHandle = u32;

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

    struct PrefabComponent
    {
        AssetHandle Handle = 0;

        PrefabComponent() = default;
        PrefabComponent(const PrefabComponent&) = default;
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

    // TODO: Cache large relationship hierarchies
    struct WorldTransformComponent
    {
        glm::mat4 Transform;

        WorldTransformComponent() = default;
        WorldTransformComponent(const WorldTransformComponent&) = default;

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

    struct SpriteRendererComponent
    {
        glm::vec4 Color{ 1.0f };
        f32 Tiling = 1.0f;
        AssetHandle Texture = 0;

        bool IsSpriteSheet = false;
        glm::vec2 SpriteCoords{ 0.0f, 0.0f };
        glm::vec2 SpriteSize{ 32.0f, 32.0f };
        std::array<glm::vec2, 4> TextureCoords;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;

        inline void UpdateTextureCoords(u32 textureWidth, u32 textureHeight)
        {
            if (IsSpriteSheet)
            {
                glm::vec2 min = { (SpriteCoords.x * SpriteSize.x) / textureWidth, (SpriteCoords.y * SpriteSize.y) / textureHeight };
                glm::vec2 max = { ((SpriteCoords.x + 1) * SpriteSize.x) / textureWidth, ((SpriteCoords.y + 1) * SpriteSize.y) / textureHeight };

                TextureCoords[0] = { min.x, min.y };
                TextureCoords[1] = { max.x, min.y };
                TextureCoords[2] = { max.x, max.y };
                TextureCoords[3] = { min.x, max.y };
            }
            else
            {
                TextureCoords[0] = { 0.0f, 0.0f };
                TextureCoords[1] = { 1.0f, 0.0f };
                TextureCoords[2] = { 1.0f, 1.0f };
                TextureCoords[3] = { 0.0f, 1.0f };
            }
        }
    };

    struct LineRendererComponent
    {
        glm::vec3 Destination{ 1.0f, 0.0f, 0.0f };
        glm::vec4 Color{ 1.0f };

        LineRendererComponent() = default;
        LineRendererComponent(const LineRendererComponent&) = default;
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
        AssetHandle Mesh = 0;

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

    struct DirectionalLightComponent
    {
        glm::vec3 Radiance{ 1.0f };
        f32 Intensity = 1.0f;

        DirectionalLightComponent() = default;
        DirectionalLightComponent(const DirectionalLightComponent&) = default;
    };

    struct PointLightComponent
    {
        glm::vec3 Radiance{ 1.0f };
        f32 Intensity = 1.0f;
        f32 Radius = 10.0f;
        f32 FallOff = 1.0f;

        PointLightComponent() = default;
        PointLightComponent(const PointLightComponent&) = default;
    };

    struct SpotLightComponent
    {
        glm::vec3 Radiance{ 1.0f };
        f32 Intensity = 5.0f;
        f32 InnerCone = 12.5f;
        f32 OuterCone = 17.5f;

        SpotLightComponent() = default;
        SpotLightComponent(const SpotLightComponent&) = default;
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
        RigidbodyType Type = RigidbodyType::Static;
        bool FixedRotation = false;
        f32 GravityScale = 1.0f;
        bool Enabled = true;
        bool ContactEnabled = true;
        bool IsBullet = false; // true == continous collision detection

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
        bool IsTrigger = false;
        u16 CollisionCategory = 0x0001; // What category is this entity
        u16 CollisionMask = 0xFFFF; // What other categories will this collide with
        i16 CollisionGroup = 0;

        // Storage for runtime
        void* RuntimeFixture = nullptr;

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
        bool IsTrigger = false;
        u16 CollisionCategory = 0x0001; // What category is this entity
        u16 CollisionMask = 0xFFFF; // What other categories will this collide with
        i16 CollisionGroup = 0;

        // Storage for runtime
        void* RuntimeFixture = nullptr;

        CircleCollider2DComponent() = default;
        CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
    };

    // Physics 3D

    struct RigidbodyComponent
    {
        RigidbodyType Type = RigidbodyType::Static;
        CollisionDetectionType CollisionDetection = CollisionDetectionType::Discrete;
        f32 GravityScale = 1.0f;
        f32 Mass = 1.0f;
        f32 LinearDamping = 0.05f;
        f32 AngularDamping = 0.05f;

        // This is here for now because we only support 1 collider per rigidbody
        // And also I do not know how to add another shape to rigidbody
        bool IsTrigger = false;
        f32 Friction = 0.2f;
        f32 Restitution = 0.0f;

        bool LockTranslationX = false;
        bool LockTranslationY = false;
        bool LockTranslationZ = false;
        bool LockRotationX = false;
        bool LockRotationY = false;
        bool LockRotationZ = false;

        BodyHandle RuntimeBodyHandle = 0;

        RigidbodyComponent() = default;
        RigidbodyComponent(const RigidbodyComponent&) = default;
    };

    struct BoxColliderComponent
    {
        glm::vec3 Size = { 0.5f, 0.5f, 0.5f };
        glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };

        BoxColliderComponent() = default;
        BoxColliderComponent(const BoxColliderComponent&) = default;
    };

    struct SphereColliderComponent
    {
        glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };
        f32 Radius = 0.5f;

        SphereColliderComponent() = default;
        SphereColliderComponent(const SphereColliderComponent&) = default;
    };

    struct CapsuleColliderComponent
    {
        glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };
        f32 Radius = 0.5f;
        f32 Height = 1.0f;

        CapsuleColliderComponent() = default;
        CapsuleColliderComponent(const CapsuleColliderComponent&) = default;
    };

    template<typename... Components>
    struct ComponentGroup
    {
        constexpr static u64 Size = sizeof...(Components);
    };

    // Order matters -> Physics2D and 3D components

    using AllComponents =
        ComponentGroup<TransformComponent, RelationshipComponent, PrefabComponent, CameraComponent, SpriteRendererComponent, LineRendererComponent,
        CircleRendererComponent, TextComponent, DirectionalLightComponent, PointLightComponent, SpotLightComponent, StaticMeshRendererComponent, 
        ScriptComponent, AudioSourceComponent, AudioListenerComponent, Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent,
        BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, RigidbodyComponent>;

}
