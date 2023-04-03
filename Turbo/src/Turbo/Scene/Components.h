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
        UUID Parent;
        std::vector<UUID> Children;
    };

    struct IDComponent
    {
        UUID ID;

        IDComponent(const UUID& id) : ID(id) {}
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

    struct AudioSourceComponent
    {
        Ref<AudioClip> Clip;
        f32 Gain = 1.0f;
        bool Spatial = false;
        bool PlayOnStart = true;
        bool Loop = false;
    };

    struct AudioListenerComponent
    {
        bool Primary = true;
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
        f32 Tiling = 1.0f;
        Ref<Texture2D> Texture; // TODO: Combine texture and subtexture
        Ref<SubTexture2D> SubTexture;
    };

    struct CircleRendererComponent
    {
        glm::vec4 Color{ 1.0f };
        f32 Thickness = 1.0f;
        f32 Fade = 0.005f;
    };

    struct TextComponent
    {
        std::string Text;
        glm::vec4 Color{ 1.0f };
        f32 KerningOffset = 0.0f;
        f32 LineSpacing = 0.0f;
        Ref<Font> FontAsset = Font::GetDefaultFont();
    };

    struct ScriptComponent
    {
        std::string ClassName;
    };

    // Physics
    struct Rigidbody2DComponent
    {
        enum class BodyType : u32 { Static = 0, Dynamic, Kinematic };
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

    template<typename... Components>
    struct ComponentGroup
    {
        constexpr static size_t Size = sizeof...(Components);
    };

    using AllComponents =
        ComponentGroup<TransformComponent, CameraComponent, SpriteRendererComponent, CircleRendererComponent, TextComponent, ScriptComponent, 
        AudioSourceComponent, AudioListenerComponent, Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent>;
}
