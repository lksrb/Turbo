#include "tbopch.h"
#include "InternalCalls.h"

#include "Script.h"
#include "ScriptInstance.h"

#include "Turbo/Physics/Physics2D.h"
#include "Turbo/Physics/PhysicsWorld2D.h"

#include "Turbo/Audio/Audio.h"
#include "Turbo/Asset/AssetManager.h"
#include "Turbo/Core/Engine.h"
#include "Turbo/Core/Input.h"
#include "Turbo/Core/Math.h"

#include "Turbo/Solution/Project.h"

#include "Turbo/Scene/Entity.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#define TBO_REGISTER_FUNCTION(name) mono_add_internal_call("Turbo.InternalCalls::" #name, name);

namespace Turbo
{
    static Entity GetEntity(u64 uuid)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        if (!entity)
            TBO_CONSOLE_ERROR("Entity doesnt exist!");

        return entity;
    }

    static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;
    static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityAddComponentFuncs;
    static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityRemoveComponentFuncs;

#pragma region Application

    static u32 Application_GetWidth()
    {
        Scene* context = Script::GetCurrentScene();
        return context->GetViewportWidth();
    }

    static u32 Application_GetHeight()
    {
        Scene* context = Script::GetCurrentScene();
        return context->GetViewportHeight();
    }

#pragma endregion

#pragma region Logging

    static void Log_String(u32 level, MonoString* string)
    {
        char* cstring = mono_string_to_utf8(string);
        std::string msg = cstring;
        mono_free(cstring);

        switch (level)
        {
            case Log::Level::Trace:
            {
                TBO_CONSOLE_TRACE(msg);
                break;
            }
            case Log::Level::Info:
            {
                TBO_CONSOLE_INFO(msg);
                break;
            }
            case Log::Level::Warn:
            {
                TBO_CONSOLE_WARN(msg);
                break;
            }
            case Log::Level::Error:
            {
                TBO_CONSOLE_ERROR(msg);
                break;
            }
            case Log::Level::Fatal:
            {
                TBO_CONSOLE_FATAL(msg);
                break;
            }
        }
    }

#pragma endregion

#pragma region Input

    static bool Input_IsKeyPressed(u32 key)
    {
        return Input::IsKeyPressed(key);
    }
    static bool Input_IsKeyReleased(u32 key)
    {
        return Input::IsKeyReleased(key);
    }
    static bool Input_IsMouseButtonPressed(u32 key)
    {
        return Input::IsMouseButtonPressed(key);
    }
    static bool Input_IsMouseButtonReleased(u32 key)
    {
        return Input::IsMouseButtonReleased(key);
    }
    static void Input_GetMousePosition(glm::vec2* mousePosition)
    {
        mousePosition->x = (f32)Input::GetMouseX();
        mousePosition->y = (f32)Input::GetMouseY();
    }

    static void Input_SetCursorMode(u32 cursorMode)
    {
        Input::SetCursorMode((CursorMode)cursorMode);
    }

#pragma endregion

#pragma region Physics2D

    static u64 Physics2D_RayCast(glm::vec2 a, glm::vec2 b)
    {
        Scene* context = Script::GetCurrentScene();

        PhysicsWorld2D* physicsWorld2d = context->GetPhysicsWorld2D();

        Entity hitEntity = physicsWorld2d->RayCast(a, b);
        if (hitEntity)
            return hitEntity.GetUUID();

        return 0;
    }

#pragma endregion

#pragma region Scene

    static u64 Scene_CreateEntity(u64 parentUUID, MonoString* name)
    {
        char* cString = mono_string_to_utf8(name);
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->CreateEntity(cString);
        TBO_ENGINE_ASSERT(entity);
        mono_free(cString);

        Entity parent = context->FindEntityByUUID(parentUUID);
        if (parent)
            entity.SetParent(parent);

        return entity.GetUUID();
    }

    static void Scene_DestroyEntity(u64 uuid)
    {
        Entity entity = GetEntity(uuid);
        Scene* context = Script::GetCurrentScene();
        context->GetPostUpdateFuncs().push_back([context, entity]() { context->DestroyEntity(entity); });
    }

    // TODO: MOVE! WHERE??????????????
    static void Scene_ScreenToWorldPosition(glm::vec2 screenPosition, glm::vec3* worldPosition) // FIXME: Raycasting
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindPrimaryCameraEntity();
        TBO_ENGINE_ASSERT(entity);

        const SceneCamera& camera = entity.GetComponent<CameraComponent>().Camera;

        glm::vec4 viewport =
        {
            context->GetViewportX(),
            context->GetViewportY(),
            context->GetViewportWidth(),
            context->GetViewportHeight()
        };

        *worldPosition = Math::UnProject(screenPosition, viewport, camera.GetViewProjection());
    }

    static void Scene_WorldToScreenPosition(glm::vec3 worldPosition, glm::vec2* screenPosition) // FIXME: Raycasting
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindPrimaryCameraEntity();
        TBO_ENGINE_ASSERT(entity);

        const SceneCamera& camera = entity.GetComponent<CameraComponent>().Camera;

        glm::vec4 clipSpacePos = camera.GetViewProjection() * glm::vec4(worldPosition, 1.0f);

        glm::vec3 ndcSpacePos = clipSpacePos / clipSpacePos.w;

        glm::vec2 windowSpacePos;
        windowSpacePos.x = ((ndcSpacePos.x + 1.0f) / 2.0f) * context->GetViewportWidth() + context->GetViewportX();
        windowSpacePos.y = ((ndcSpacePos.y + 1.0f) / 2.0f) * context->GetViewportHeight() + context->GetViewportY();

        *screenPosition = windowSpacePos;
    }

#pragma endregion

#pragma region Entity

    static u64 Entity_FindEntityByName(MonoString* string)
    {
        char* cString = mono_string_to_utf8(string);
        std::string name = cString;
        mono_free(cString);

        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByName(name);

        if (entity)
            return entity.GetUUID();

        TBO_CONSOLE_ERROR("Could not find entity with name \"{}\"", name);
        return 0;
    }

    static MonoObject* Entity_Get_Instance(UUID uuid)
    {
        Ref<ScriptInstance> instance = Script::FindEntityInstance(uuid);
        TBO_ENGINE_ASSERT(instance);

        return instance->GetMonoInstance();
    }

    static bool Entity_Has_Component(u64 uuid, MonoReflectionType* reflectionType)
    {
        Entity entity = GetEntity(uuid);

        if (!entity)
            return false;

        MonoType* component_type = mono_reflection_type_get_type(reflectionType);
        TBO_ENGINE_ASSERT(s_EntityHasComponentFuncs.find(component_type) != s_EntityHasComponentFuncs.end());

        return s_EntityHasComponentFuncs.at(component_type)(entity);
    }

    static void Entity_Add_Component(u64 uuid, MonoReflectionType* reflectionType)
    {
        Entity entity = GetEntity(uuid);

        if (!entity)
            return;

        MonoType* componentType = mono_reflection_type_get_type(reflectionType);

        TBO_ENGINE_ASSERT(s_EntityAddComponentFuncs.find(componentType) != s_EntityAddComponentFuncs.end());
        s_EntityAddComponentFuncs.at(componentType)(entity);
    }

    static void Entity_Remove_Component(u64 uuid, MonoReflectionType* reflectionType)
    {
        Entity entity = GetEntity(uuid);

        if (!entity)
            return;

        MonoType* componentType = mono_reflection_type_get_type(reflectionType);

        TBO_ENGINE_ASSERT(s_EntityRemoveComponentFuncs.find(componentType) != s_EntityRemoveComponentFuncs.end());
        s_EntityRemoveComponentFuncs.at(componentType)(entity);
    }

    static MonoArray* Entity_Get_Children(u64 uuid)
    {
        Entity entity = GetEntity(uuid);

        MonoDomain* appDomain = Script::GetAppDomain();
        Ref<ScriptClass> entityClass = Script::GetEntityBaseClass();

        const auto& children = entity.GetChildren();

        // Create an array of Entity refs
        MonoArray* monoArray = mono_array_new(appDomain, entityClass->GetMonoClass(), children.size());

        // Allocate new entities
        for (uintptr_t i = 0; i < children.size(); i++)
        {
            Ref<ScriptInstance> instance = Ref<ScriptInstance>::Create(entityClass, children[i]);
            mono_array_setref(monoArray, i, instance->GetMonoInstance());
        }

        return monoArray;
    }

    static MonoString* Entity_Get_Name(UUID uuid)
    {
        Entity entity = GetEntity(uuid);

        if (!entity)
            return nullptr;

        MonoDomain* appDomain = Script::GetAppDomain();

        MonoString* monoString = mono_string_new(appDomain, entity.GetName().c_str());
        TBO_ENGINE_ASSERT(monoString);
        return monoString;
    }

    static void Entity_Set_Name(UUID uuid, MonoString* name)
    {
        Entity entity = GetEntity(uuid);

        if (!entity)
            return;

        char* cString = mono_string_to_utf8(name);
        entity.SetName(cString);
        mono_free(cString);
    }

    // FIXME: Temporary
    static void TryInvokeOnCreateRecursively(Entity entity)
    {
        Scene* context = Script::GetCurrentScene();

        // Call C# Entity::OnCreate method
        if (entity.HasComponent<ScriptComponent>())
        {
            Script::InvokeEntityOnCreate(entity);
        }

        const auto& children = entity.GetChildren();
        for (auto childUUID : children)
        {
            Entity child = context->FindEntityByUUID(childUUID);
            TBO_ENGINE_ASSERT(child);
            TryInvokeOnCreateRecursively(child);
        }
    }

    static u64 Entity_InstantiatePrefabWithTranslation(MonoString* monoString, glm::vec3* translation)
    {
        Scene* context = Script::GetCurrentScene();
        char* cString = mono_string_to_utf8(monoString);
        std::filesystem::path prefabPath = Project::GetProjectDirectory() / cString;
        mono_free(cString);

        Entity entity = AssetManager::DeserializePrefab(prefabPath, context, *translation);

        if (entity)
        {
            TryInvokeOnCreateRecursively(entity);

            return entity.GetUUID();
        }

        TBO_ENGINE_ERROR("Could not instantiate prefab!");
        return 0;
    }

    static u64 Entity_InstantiateChildPrefabWithTranslation(u64 uuid, MonoString* monoString, glm::vec3* translation)
    {
        Scene* context = Script::GetCurrentScene();
        char* cString = mono_string_to_utf8(monoString);
        std::filesystem::path prefabPath = Project::GetProjectDirectory() / cString;
        mono_free(cString);

        Entity child = AssetManager::DeserializePrefab(prefabPath, context, *translation);

        if (child)
        {
            Entity parent = GetEntity(uuid);
            child.SetParent(parent);

            TryInvokeOnCreateRecursively(child);

            return child.GetUUID();
        }

        TBO_ENGINE_ERROR("Could not instantiate prefab!");
        return 0;
    }
#pragma endregion

#pragma region Components

#pragma region TransformComponent

    // Translation
    // Translation
    // Translation
    static void Component_Transform_Get_Translation(UUID uuid, glm::vec3* outTranslation)
    {
        Entity entity = GetEntity(uuid);
        if (entity)
            *outTranslation = entity.GetComponent<TransformComponent>().Translation;
    }

    static void Component_Transform_Set_Translation(UUID uuid, glm::vec3* translation)
    {
        Entity entity = GetEntity(uuid);

        if (entity)
            entity.GetComponent<TransformComponent>().Translation = *translation;
    }

    // Rotation
    // Rotation
    // Rotation
    static void Component_Transform_Get_Rotation(UUID uuid, glm::vec3* outRotation)
    {
        Entity entity = GetEntity(uuid);
        if (entity)
            *outRotation = entity.GetComponent<TransformComponent>().Rotation;
    }

    static void Component_Transform_Set_Rotation(UUID uuid, glm::vec3* rotation)
    {
        Entity entity = GetEntity(uuid);
        if (entity)
            entity.GetComponent<TransformComponent>().Rotation = *rotation;
    }

    // Scale
    // Scale
    // Scale
    static void Component_Transform_Get_Scale(UUID uuid, glm::vec3* outScale)
    {
        Entity entity = GetEntity(uuid);
        if (entity)
            *outScale = entity.GetComponent<TransformComponent>().Scale;
    }

    static void Component_Transform_Set_Scale(UUID uuid, glm::vec3* scale)
    {
        Entity entity = GetEntity(uuid);
        if (entity)
            entity.GetComponent<TransformComponent>().Scale = *scale;
    }

#pragma endregion

#pragma region SpriteRendererComponent

    static void Component_SpriteRenderer_Get_Color(UUID uuid, glm::vec4* out_color)
    {
        Entity entity = GetEntity(uuid);

        *out_color = entity.GetComponent<SpriteRendererComponent>().Color;
    }
    static void Component_SpriteRenderer_Set_Color(UUID uuid, glm::vec4* color)
    {
        Entity entity = GetEntity(uuid);

        entity.GetComponent<SpriteRendererComponent>().Color = *color;
    }

    static void Component_SpriteRenderer_SetSpriteBounds(UUID uuid, glm::vec2 position, glm::vec2 size)
    {
        Entity entity = GetEntity(uuid);

        auto& src = entity.GetComponent<SpriteRendererComponent>();

        if (src.SubTexture)
        {
            src.SubTexture->SetBounds(position, size);
            return;
        }

        TBO_CONSOLE_ERROR("\"{}\" entity sprite renderer does not have a texture!", entity.GetName());
    }

#pragma endregion

#pragma region CircleRendererComponent_TODO
#pragma endregion   

#pragma region TextComponent

    static void Component_Text_Set_Text(u64 uuid, MonoString* monoString)
    {
        char* cString = mono_string_to_utf8(monoString);
        std::string string = cString;
        mono_free(cString);

        Entity entity = GetEntity(uuid);

        entity.GetComponent<TextComponent>().Text = string;
    }

    static MonoString* Component_Text_Get_Text(u64 uuid)
    {
        Entity entity = GetEntity(uuid);

        MonoDomain* appDomain = Script::GetAppDomain();

        const std::string& text = entity.GetComponent<TextComponent>().Text;
        MonoString* monoString = mono_string_new(appDomain, text.c_str());
        return monoString;
    }

    static void Component_Text_Get_Color(UUID uuid, glm::vec4* outColor)
    {
        Entity entity = GetEntity(uuid);

        *outColor = entity.GetComponent<TextComponent>().Color;
    }
    static void Component_Text_Set_Color(UUID uuid, glm::vec4* color)
    {
        Entity entity = GetEntity(uuid);

        entity.GetComponent<TextComponent>().Color = *color;
    }

#pragma endregion

#pragma region AudioSourceComponent

    static f32 Component_AudioSource_Get_Gain(u64 uuid)
    {
        Entity entity = GetEntity(uuid);

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
        return audioSourceComponent.Gain;
    }

    static void Component_AudioSource_Set_Gain(u64 uuid, f32 gain)
    {
        Entity entity = GetEntity(uuid);

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
        audioSourceComponent.Gain = gain;
    }

    static bool Component_AudioSource_Get_PlayOnStart(u64 uuid)
    {
        Entity entity = GetEntity(uuid);

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
        return audioSourceComponent.PlayOnAwake;
    }

    static void Component_AudioSource_Set_PlayOnStart(u64 uuid, bool playOnAwake)
    {
        Entity entity = GetEntity(uuid);

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
        audioSourceComponent.PlayOnAwake = playOnAwake;
    }

    static bool Component_AudioSource_Get_Loop(u64 uuid)
    {
        Entity entity = GetEntity(uuid);

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
        return audioSourceComponent.Loop;
    }

    static void Component_AudioSource_Set_Loop(u64 uuid, bool loop)
    {
        Entity entity = GetEntity(uuid);

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
        audioSourceComponent.Loop = loop;
    }

    static void Component_AudioSource_Play(u64 uuid)
    {
        Entity entity = GetEntity(uuid);
        if (!entity)
            return;

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();

        if (audioSourceComponent.AudioPath.empty())
            return;

        // If its already playing, do not play it again
        if (Audio::IsPlaying(uuid))
            return;

        Audio::Play(uuid, audioSourceComponent.Loop);
    }

    static void Component_AudioSource_Stop(u64 uuid)
    {
        Entity entity = GetEntity(uuid);
        if (!entity)
            return;

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();

        if (audioSourceComponent.AudioPath.empty())
            return;

        Audio::Stop(uuid);
    }

    static void Component_AudioSource_Pause(u64 uuid)
    {
        Entity entity = GetEntity(uuid);
        if (!entity)
            return;

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();

        if (audioSourceComponent.AudioPath.empty())
            return;

        Audio::Pause(uuid);
    }

    static void Component_AudioSource_Resume(u64 uuid)
    {
        Entity entity = GetEntity(uuid);
        if (!entity)
            return;

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();

        if (audioSourceComponent.AudioPath.empty())
            return;

        Audio::Resume(uuid);
    }

    static bool Component_AudioSource_IsPlaying(u64 uuid)
    {
        Entity entity = GetEntity(uuid);
        if (!entity)
            return false;

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();

        if (audioSourceComponent.AudioPath.empty())
            return false;

        return Audio::IsPlaying(uuid);
    }

#pragma endregion

#pragma region AudioListenerComponent

    static bool Component_AudioListener_Get_IsPrimary(u64 uuid)
    {
        Entity entity = GetEntity(uuid);

        auto& audioListenerComponent = entity.GetComponent<AudioListenerComponent>();
        return audioListenerComponent.IsPrimary;
    }

    static void Component_AudioListener_Set_IsPrimary(u64 uuid, bool isPrimary)
    {
        Entity entity = GetEntity(uuid);

        auto& audioListenerComponent = entity.GetComponent<AudioListenerComponent>();
        audioListenerComponent.IsPrimary = isPrimary;
    }

#pragma endregion

#pragma region RigidBody2DComponent

    static void Component_Rigidbody2D_ApplyLinearImpulse(UUID uuid, glm::vec2* impulse, glm::vec2* worldPosition, bool wake)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;

        body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(worldPosition->x, worldPosition->y), wake);
    }
    static void Component_Rigidbody2D_ApplyLinearImpulseToCenter(UUID uuid, glm::vec2* impulse, bool wake)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
    }
    static void Component_Rigidbody2D_ApplyForceToCenter(UUID uuid, glm::vec2* force, bool wake)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        body->ApplyForceToCenter(b2Vec2(force->x, force->y), wake);
    }
    static void Component_Rigidbody2D_Set_LinearVelocity(UUID uuid, glm::vec2* velocity)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        body->SetLinearVelocity(b2Vec2(velocity->x, velocity->y));
    }
    static void Component_Rigidbody2D_Get_LinearVelocity(UUID uuid, glm::vec2* velocity)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        b2Vec2 b2Velocity = body->GetLinearVelocity();

        velocity->x = b2Velocity.x;
        velocity->y = b2Velocity.y;
    }
    static void Component_Rigidbody2D_ApplyTorque(UUID uuid, f32 torque, bool wake)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        body->ApplyTorque(torque, wake);
    }

    static f32 Component_Rigidbody2D_Get_GravityScale(UUID uuid)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        return rb2d.GravityScale;
    }
    static void Component_Rigidbody2D_Set_GravityScale(UUID uuid, f32 gravityScale)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        rb2d.GravityScale = gravityScale;
    }

    static Rigidbody2DComponent::BodyType Component_Rigidbody2D_Get_BodyType(UUID uuid)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        return rb2d.Type;
    }

    static void Component_Rigidbody2D_Set_BodyType(UUID uuid, Rigidbody2DComponent::BodyType type)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        rb2d.Type = type;
    }

    static void Component_Rigidbody2D_Set_Enabled(UUID uuid, bool enabled)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        rb2d.Enabled = enabled;
    }

    static bool Component_Rigidbody2D_Get_Enabled(UUID uuid)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        return rb2d.Enabled;
    }

    static void Component_Rigidbody2D_Set_ContactEnabled(UUID uuid, bool enabled)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        rb2d.ContactEnabled = enabled;
    }

    static bool Component_Rigidbody2D_Get_ContactEnabled(UUID uuid)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        return rb2d.ContactEnabled;
    }

#pragma endregion

#pragma region BoxCollider2DComponent

    // Offset
    static void Component_BoxCollider2D_Get_Offset(UUID uuid, glm::vec2* outOffset)
    {
        Entity entity = GetEntity(uuid);

        *outOffset = entity.GetComponent<BoxCollider2DComponent>().Offset;
    }
    static void Component_BoxCollider2D_Set_Offset(UUID uuid, glm::vec2* offset)
    {
        Entity entity = GetEntity(uuid);

        auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
        auto& transform = entity.Transform();

        if (bc2d.Offset == *offset)
            return;

        bc2d.Offset = *offset;

        // Destroys old fixture and replaces it with new one
        entity.ReplaceCompoment<BoxCollider2DComponent>(bc2d);
    }

    // Size
    static void Component_BoxCollider2D_Get_Size(UUID uuid, glm::vec2* outSize)
    {
        Entity entity = GetEntity(uuid);

        *outSize = entity.GetComponent<BoxCollider2DComponent>().Size;
    }
    static void Component_BoxCollider2D_Set_Size(UUID uuid, glm::vec2* size)
    {
        Entity entity = GetEntity(uuid);

        auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
        auto& transform = entity.Transform();

        if (bc2d.Size == *size)
            return;

        bc2d.Size = *size;

        // Destroys old fixture and replaces it with new one
        entity.ReplaceCompoment<BoxCollider2DComponent>(bc2d);
    }

    // IsSensor
    static bool Component_BoxCollider2D_Get_IsSensor(UUID uuid)
    {
        Entity entity = GetEntity(uuid);

        return entity.GetComponent<BoxCollider2DComponent>().IsSensor;
    }

    static void Component_BoxCollider2D_Set_IsSensor(UUID uuid, bool isSensor)
    {
        Entity entity = GetEntity(uuid);

        auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

        if (bc2d.IsSensor == isSensor)
            return;

        bc2d.IsSensor = isSensor;

        // Destroys old fixture and replaces it with new one
        entity.ReplaceCompoment<BoxCollider2DComponent>(bc2d);
    }

    static void Component_BoxCollider2D_Set_CollisionFilter(UUID uuid, u16 category, u16 mask)
    {
        Entity entity = GetEntity(uuid);

        auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

        if (bc2d.CollisionCategory == category && bc2d.CollisionMask == mask)
            return;

        bc2d.CollisionCategory = category;
        bc2d.CollisionMask = mask;

        if (!entity.HasComponent<BoxCollider2DComponent>())
        {
            TBO_CONSOLE_ERROR("Entity deoes not have a rigidbody!");
            return;
        }

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;

        // Iterate through all fixtures and set collision filters
        b2Fixture* fixture = body->GetFixtureList();
        while (fixture != nullptr)
        {
            b2Shape* shape = fixture->GetShape();

            if (shape->GetType() == b2Shape::e_polygon)
            {
                b2Filter collisionFilter;
                collisionFilter.categoryBits = bc2d.CollisionCategory;
                collisionFilter.maskBits = bc2d.CollisionMask;
                collisionFilter.groupIndex = 0;
                fixture->SetFilterData(collisionFilter);
            }

            fixture = fixture->GetNext();
        }
    }

    static void Component_BoxCollider2D_Get_CollisionFilter(UUID uuid, u16* category, u16* mask)
    {
        Entity entity = GetEntity(uuid);

        auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
        *category = bc2d.CollisionCategory;
        *mask = bc2d.CollisionMask;
    }

#pragma endregion

#pragma region CircleCollider2DComponent

    // Offset
    static void Component_CircleCollider2D_Get_Offset(UUID uuid, glm::vec2* outOffset)
    {
        Entity entity = GetEntity(uuid);

        *outOffset = entity.GetComponent<CircleCollider2DComponent>().Offset;
    }
    static void Component_CircleCollider2D_Set_Offset(UUID uuid, glm::vec2* offset)
    {
        Entity entity = GetEntity(uuid);

        entity.GetComponent<CircleCollider2DComponent>().Offset = *offset;
    }

    // Radius
    static float Component_CircleCollider2D_Get_Radius(UUID uuid)
    {
        Entity entity = GetEntity(uuid);

        return entity.GetComponent<CircleCollider2DComponent>().Radius;
    }
    static void Component_CircleCollider2D_Set_Radius(UUID uuid, f32* radius)
    {
        Entity entity = GetEntity(uuid);

        entity.GetComponent<CircleCollider2DComponent>().Radius = *radius;
    }

    static void Component_CircleCollider2D_Set_CollisionFilter(UUID uuid, u16 category, u16 mask)
    {
        Entity entity = GetEntity(uuid);

        auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

        if (cc2d.CollisionCategory == category && cc2d.CollisionMask == mask)
            return;

        cc2d.CollisionCategory = category;
        cc2d.CollisionMask = mask;

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;

        // Iterate through all fixtures and set collision filters
        b2Fixture* fixture = body->GetFixtureList();
        while (fixture != nullptr)
        {
            b2Shape* shape = fixture->GetShape();

            if (shape->GetType() == b2Shape::e_circle)
            {
                b2Filter collisionFilter;
                collisionFilter.categoryBits = cc2d.CollisionCategory;
                collisionFilter.maskBits = cc2d.CollisionMask;
                collisionFilter.groupIndex = 0;
                fixture->SetFilterData(collisionFilter);
            }

            fixture = fixture->GetNext();
        }
    }

    static void Component_CircleCollider2D_Get_CollisionFilter(UUID uuid, u16* category, u16* mask)
    {
        Entity entity = GetEntity(uuid);

        auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
        *category = cc2d.CollisionCategory;
        *mask = cc2d.CollisionMask;
    }

#pragma endregion

#pragma endregion

    void InternalCalls::Init()
    {
        // Application
        TBO_REGISTER_FUNCTION(Application_GetWidth);
        TBO_REGISTER_FUNCTION(Application_GetHeight);

        // Logging
        TBO_REGISTER_FUNCTION(Log_String);

        // Scene
        TBO_REGISTER_FUNCTION(Scene_CreateEntity);
        TBO_REGISTER_FUNCTION(Scene_DestroyEntity);
        TBO_REGISTER_FUNCTION(Scene_ScreenToWorldPosition);
        TBO_REGISTER_FUNCTION(Scene_WorldToScreenPosition);

        // Entity
        TBO_REGISTER_FUNCTION(Entity_FindEntityByName);
        TBO_REGISTER_FUNCTION(Entity_Get_Instance);
        TBO_REGISTER_FUNCTION(Entity_Get_Name);
        TBO_REGISTER_FUNCTION(Entity_Set_Name);
        TBO_REGISTER_FUNCTION(Entity_Has_Component);
        TBO_REGISTER_FUNCTION(Entity_Add_Component);
        TBO_REGISTER_FUNCTION(Entity_Remove_Component);
        TBO_REGISTER_FUNCTION(Entity_Get_Children);

        // Prefab
        TBO_REGISTER_FUNCTION(Entity_InstantiatePrefabWithTranslation);
        TBO_REGISTER_FUNCTION(Entity_InstantiateChildPrefabWithTranslation);

        // Input
        TBO_REGISTER_FUNCTION(Input_IsKeyPressed);
        TBO_REGISTER_FUNCTION(Input_IsKeyReleased);
        TBO_REGISTER_FUNCTION(Input_IsMouseButtonPressed);
        TBO_REGISTER_FUNCTION(Input_IsMouseButtonReleased);
        TBO_REGISTER_FUNCTION(Input_GetMousePosition);
        TBO_REGISTER_FUNCTION(Input_SetCursorMode);

        // Transform
        TBO_REGISTER_FUNCTION(Component_Transform_Get_Translation);
        TBO_REGISTER_FUNCTION(Component_Transform_Set_Translation);
        TBO_REGISTER_FUNCTION(Component_Transform_Get_Rotation);
        TBO_REGISTER_FUNCTION(Component_Transform_Set_Rotation);
        TBO_REGISTER_FUNCTION(Component_Transform_Get_Scale);
        TBO_REGISTER_FUNCTION(Component_Transform_Set_Scale);

        // SpriteRenderer
        TBO_REGISTER_FUNCTION(Component_SpriteRenderer_Get_Color);
        TBO_REGISTER_FUNCTION(Component_SpriteRenderer_Set_Color);
        TBO_REGISTER_FUNCTION(Component_SpriteRenderer_SetSpriteBounds);

        // CircleRenderer

        // Text
        TBO_REGISTER_FUNCTION(Component_Text_Set_Text);
        TBO_REGISTER_FUNCTION(Component_Text_Get_Text);
        TBO_REGISTER_FUNCTION(Component_Text_Get_Color);
        TBO_REGISTER_FUNCTION(Component_Text_Set_Color);

        // Audio Source
        TBO_REGISTER_FUNCTION(Component_AudioSource_Get_Gain);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Set_Gain);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Get_PlayOnStart);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Set_PlayOnStart);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Set_Loop);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Get_Loop);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Play);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Stop);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Resume);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Pause);
        TBO_REGISTER_FUNCTION(Component_AudioSource_IsPlaying);

        // Audio Listener
        TBO_REGISTER_FUNCTION(Component_AudioListener_Get_IsPrimary);
        TBO_REGISTER_FUNCTION(Component_AudioListener_Set_IsPrimary);

        // Physics2D
        TBO_REGISTER_FUNCTION(Physics2D_RayCast);

        // RigidBody2D
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_ApplyLinearImpulse);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_ApplyLinearImpulseToCenter);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_ApplyForceToCenter);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_ApplyTorque);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Set_GravityScale);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Get_GravityScale);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Get_BodyType);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Set_BodyType);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Set_LinearVelocity);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Get_LinearVelocity);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Set_Enabled);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Get_Enabled);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Set_ContactEnabled);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Get_ContactEnabled);

        // BoxCollider2D
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Get_Offset);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Set_Offset);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Get_Size);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Set_Size);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Get_IsSensor);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Set_IsSensor);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Set_CollisionFilter);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Get_CollisionFilter);

        // CircleCollider2D
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Get_Offset);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Set_Offset);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Get_Radius);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Set_Radius);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Set_CollisionFilter);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Get_CollisionFilter);

        // Register components in AllComponents struct
        RegisterComponents();
    }

    template<typename... Component>
    static void RegisterComponent(ComponentGroup<Component...>)
    {
        ([&]()
        {
            std::string_view typeName = typeid(Component).name();
            size_t pos = typeName.find_last_of(':');
            std::string_view structName = typeName.substr(pos + 1);
            std::string managedTypename = fmt::format("Turbo.{}", structName);

            MonoType* type = mono_reflection_type_from_name(managedTypename.data(), Script::GetCoreAssemblyImage());
            if (!type)
            {
                TBO_ENGINE_ERROR("Could not find component type {0}", managedTypename);
                return;
            }

            s_EntityHasComponentFuncs[type] = [](Entity entity) { return entity.HasComponent<Component>(); };
            s_EntityAddComponentFuncs[type] = [](Entity entity) { entity.AddComponent<Component>(); };
            s_EntityRemoveComponentFuncs[type] = [](Entity entity) { entity.RemoveComponent<Component>(); };
        }(), ...);
    }

    void InternalCalls::RegisterComponents()
    {
        s_EntityHasComponentFuncs.clear();

        RegisterComponent(AllComponents{});
    }

}
