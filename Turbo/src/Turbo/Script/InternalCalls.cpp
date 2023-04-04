#include "tbopch.h"
#include "InternalCalls.h"

#include "Script.h"
#include "Turbo/Physics/Physics2D.h"

#include "Turbo/Core/Input.h"
#include "Turbo/Scene/Entity.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>


#define TBO_REGISTER_FUNCTION(name) mono_add_internal_call("Turbo.InternalCalls::" #name, name);

namespace Turbo
{
    extern Script::Data* g_Data;

    // =============================================================================
    //                                  Logging                                   
    // =============================================================================

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

    // =============================================================================
    //                                  Input                                   
    // =============================================================================

    static inline bool Input_IsKeyPressed(uint32_t key)
    {
        return Input::IsKeyPressed(key);
    }
    static inline bool Input_IsKeyReleased(uint32_t key)
    {
        return Input::IsKeyReleased(key);
    }
    static inline bool Input_IsMouseButtonPressed(uint32_t key)
    {
        return Input::IsMouseButtonPressed(key);
    }
    static inline bool Input_IsMouseButtonReleased(uint32_t key)
    {
        return Input::IsMouseButtonReleased(key);
    }

    // =============================================================================
    //                                  Entity                                   
    // =============================================================================

    static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

    static u64 Entity_FindEntityByName(MonoString* name)
    {
        char* cString = mono_string_to_utf8(name);
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByName(cString);

        TBO_ENGINE_ASSERT(entity);
        mono_free(cString);

        return entity.GetUUID();
    }
    static MonoObject* Entity_Instance_Get(UUID uuid)
    {
        Ref<ScriptInstance> instance = Script::FindEntityInstance(uuid);
        TBO_ENGINE_ASSERT(instance);

        return instance->GetInstance();
    }
    static bool Entity_Has_Component(u64 uuid, MonoReflectionType* reflectionType)
    {
        Scene* context = Script::GetCurrentScene();

        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        MonoType* component_type = mono_reflection_type_get_type(reflectionType);
        TBO_ENGINE_ASSERT(s_EntityHasComponentFuncs.find(component_type) != s_EntityHasComponentFuncs.end());

        return s_EntityHasComponentFuncs.at(component_type)(entity);
    }

    // =============================================================================
    //                                  Components                                   
    // =============================================================================
#pragma region TransformComponent

// Translation
// Translation
// Translation
    static void Component_Transform_Get_Translation(UUID uuid, glm::vec3* outTranslation)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        *outTranslation = entity.GetComponent<TransformComponent>().Translation;
    }

    static void Component_Transform_Set_Translation(UUID uuid, glm::vec3* translation)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        entity.GetComponent<TransformComponent>().Translation = *translation;
    }

    // Rotation
    // Rotation
    // Rotation
    static void Component_Transform_Get_Rotation(UUID uuid, glm::vec3* outRotation)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        *outRotation = entity.GetComponent<TransformComponent>().Rotation;
    }

    static void Component_Transform_Set_Rotation(UUID uuid, glm::vec3* rotation)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        entity.GetComponent<TransformComponent>().Rotation = *rotation;
    }

    // Scale
    // Scale
    // Scale
    static void Component_Transform_Get_Scale(UUID uuid, glm::vec3* outScale)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        *outScale = entity.GetComponent<TransformComponent>().Scale;
    }

    static void Component_Transform_Set_Scale(UUID uuid, glm::vec3* scale)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        entity.GetComponent<TransformComponent>().Scale = *scale;
    }

#pragma endregion

#pragma region SpriteRendererComponent

    static void Component_SpriteRenderer_Get_Color(UUID uuid, glm::vec4* out_color)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        *out_color = entity.GetComponent<SpriteRendererComponent>().Color;
    }
    static void Component_SpriteRenderer_Set_Color(UUID uuid, glm::vec4* color)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        entity.GetComponent<SpriteRendererComponent>().Color = *color;
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

        Scene* context = Script::GetCurrentScene();

        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        entity.GetComponent<TextComponent>().Text = string;
    }

    static MonoString* Component_Text_Get_Text(u64 uuid)
    {
        Scene* context = Script::GetCurrentScene();

        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        const std::string& text = entity.GetComponent<TextComponent>().Text;
        MonoString* monoString = mono_string_new(g_Data->AppDomain, text.c_str());
        return monoString;
    }
#pragma endregion

#pragma region AudioSourceComponent

    static f32 Component_AudioSource_Get_Gain(u64 uuid)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
        return audioSourceComponent.Gain;
    }

    static void Component_AudioSource_Set_Gain(u64 uuid, f32 gain)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
        audioSourceComponent.Gain = gain;
    }

    static bool Component_AudioSource_Get_PlayOnStart(u64 uuid)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
        return audioSourceComponent.PlayOnStart;
    }

    static void Component_AudioSource_Set_PlayOnStart(u64 uuid, bool playOnStart)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
        audioSourceComponent.PlayOnStart = playOnStart;
    }

    static bool Component_AudioSource_Get_Loop(u64 uuid)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
        return audioSourceComponent.Loop;
    }

    static void Component_AudioSource_Set_Loop(u64 uuid, bool loop)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
        audioSourceComponent.Loop = loop;
    }

#pragma endregion

#pragma region AudioListenerComponent

    static bool Component_AudioListener_Get_IsPrimary(u64 uuid)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& audioListenerComponent = entity.GetComponent<AudioListenerComponent>();

        return audioListenerComponent.IsPrimary;
    }

    static void Component_AudioListener_Set_IsPrimary(u64 uuid, bool isPrimary)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

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
    static void Component_Rigidbody2D_ApplyTorque(UUID uuid, float torque, bool wake)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        body->ApplyTorque(torque, wake);
    }

    static bool Component_Rigidbody2D_Get_Gravity(UUID uuid)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = reinterpret_cast<b2Body*>(rb2d.RuntimeBody);
        return body->GetGravityScale() != 0.0f;
    }
    static void Component_Rigidbody2D_Set_Gravity(UUID uuid, bool gravity)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        body->SetGravityScale(gravity ? 1.0f : 0.0f);
    }

    static Rigidbody2DComponent::BodyType Component_Rigidbody2D_Get_BodyType(UUID uuid)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = reinterpret_cast<b2Body*>(rb2d.RuntimeBody);
        return Utils::Rigidbody2DTypeFromBox2DBody(body->GetType());
    }

    static void Component_Rigidbody2D_Set_BodyType(UUID uuid, Rigidbody2DComponent::BodyType type)
    {
        Scene* scene = Script::GetCurrentScene();
        Entity entity = scene->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = reinterpret_cast<b2Body*>(rb2d.RuntimeBody);
        body->SetType(Utils::Rigidbody2DTypeToBox2DBody(type));
    }

#pragma endregion

#pragma region BoxCollider2DComponent

    // Offset
    static void Component_BoxCollider2D_Get_Offset(UUID uuid, glm::vec2* outOffset)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        *outOffset = entity.GetComponent<BoxCollider2DComponent>().Offset;
    }
    static void Component_BoxCollider2D_Set_Offset(UUID uuid, glm::vec2* offset)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        entity.GetComponent<BoxCollider2DComponent>().Offset = *offset;
    }

    // Size
    static void Component_BoxCollider2D_Get_Size(UUID uuid, glm::vec2* outSize)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        *outSize = entity.GetComponent<BoxCollider2DComponent>().Size;
    }
    static void Component_BoxCollider2D_Set_Size(UUID uuid, glm::vec2* size)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        entity.GetComponent<BoxCollider2DComponent>().Size = *size;
    }

    // Sensor
    static bool Component_BoxCollider2D_Get_IsSensor(UUID uuid)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        return entity.GetComponent<BoxCollider2DComponent>().IsSensor;
    }

#pragma endregion

#pragma region CircleCollider2DComponent

    // Offset
    static void Component_CircleCollider2D_Get_Offset(UUID uuid, glm::vec2* outOffset)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        *outOffset = entity.GetComponent<CircleCollider2DComponent>().Offset;
    }
    static void Component_CircleCollider2D_Set_Offset(UUID uuid, glm::vec2* offset)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        entity.GetComponent<CircleCollider2DComponent>().Offset = *offset;
    }

    // Radius
    static float Component_CircleCollider2D_Get_Radius(UUID uuid)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        return entity.GetComponent<CircleCollider2DComponent>().Radius;
    }
    static void Component_CircleCollider2D_Set_Radius(UUID uuid, f32* radius)
    {
        Scene* context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        TBO_ENGINE_ASSERT(entity);

        entity.GetComponent<CircleCollider2DComponent>().Radius = *radius;
    }
#pragma endregion

    void InternalCalls::Init()
    {
        // General
        TBO_REGISTER_FUNCTION(Log_String);
        TBO_REGISTER_FUNCTION(Entity_FindEntityByName);
        TBO_REGISTER_FUNCTION(Entity_Instance_Get);
        TBO_REGISTER_FUNCTION(Entity_Has_Component);

        // Input
        TBO_REGISTER_FUNCTION(Input_IsKeyPressed);
        TBO_REGISTER_FUNCTION(Input_IsKeyReleased);
        TBO_REGISTER_FUNCTION(Input_IsMouseButtonPressed);
        TBO_REGISTER_FUNCTION(Input_IsMouseButtonReleased);

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

        // CircleRenderer

        // Text
        TBO_REGISTER_FUNCTION(Component_Text_Set_Text);
        TBO_REGISTER_FUNCTION(Component_Text_Get_Text);

        // Audio Source
        TBO_REGISTER_FUNCTION(Component_AudioSource_Get_Gain);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Set_Gain);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Get_PlayOnStart);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Set_PlayOnStart);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Set_Loop);
        TBO_REGISTER_FUNCTION(Component_AudioSource_Get_Loop);

        // Audio Listener
        TBO_REGISTER_FUNCTION(Component_AudioListener_Get_IsPrimary);
        TBO_REGISTER_FUNCTION(Component_AudioListener_Set_IsPrimary);

        // RigidBody2D
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_ApplyLinearImpulse);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_ApplyLinearImpulseToCenter);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_ApplyTorque);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Set_Gravity);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Get_Gravity);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Get_BodyType);
        TBO_REGISTER_FUNCTION(Component_Rigidbody2D_Set_BodyType);

        // BoxCollider2D
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Get_Offset);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Set_Offset);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Get_Size);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Set_Size);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Get_IsSensor);

        // CircleCollider2D
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Get_Offset);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Set_Offset);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Get_Radius);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Set_Radius);

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

            MonoType* type = mono_reflection_type_from_name(managedTypename.data(), g_Data->ScriptCoreAssemblyImage);
            if (!type)
            {
                TBO_ENGINE_ERROR("Could not find component type {0}", managedTypename);
                return;
            }
            s_EntityHasComponentFuncs[type] = [](Entity entity) { return entity.HasComponent<Component>(); };
        }(), ...);
    }

    void InternalCalls::RegisterComponents()
    {
        s_EntityHasComponentFuncs.clear();

        RegisterComponent(AllComponents{});
    }

}
