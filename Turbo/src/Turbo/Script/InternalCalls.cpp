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
        enum LogLevel : u32
        {
            Info = 0,
            Warn,
            Error,
            Fatal
        };

        char* cstring = mono_string_to_utf8(string);
        std::string msg = cstring;
        mono_free(cstring);

        switch (level)
        {
            case LogLevel::Info:
            {
                TBO_INFO(msg);
                break;
            }
            case LogLevel::Warn:
            {
                TBO_WARN(msg);
                break;
            }
            case LogLevel::Error:
            {
                TBO_ERROR(msg);
                break;
            }
            case LogLevel::Fatal:
            {
                TBO_FATAL(msg);
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
        char* cstring = mono_string_to_utf8(name);
        std::string string_name = cstring;
        mono_free(cstring);

        Scene* context = Script::GetCurrentScene();

        Entity entity = context->FindEntityByName(string_name);
        TBO_ENGINE_ASSERT(entity);

        return entity.GetUUID();
    }
    static MonoObject* Entity_Instance_Get(UUID uuid)
    {
        Ref<ScriptInstance> instance = Script::FindEntityInstance(uuid);
        TBO_ENGINE_ASSERT(instance);

        return instance->GetInstance();
    }
    static bool Entity_Has_Component(u64 uuid, MonoReflectionType* reflection_type)
    {
        Scene* context = Script::GetCurrentScene();

        Entity entity = context->FindEntityByUUID(uuid);
        TBO_ENGINE_ASSERT(entity);

        MonoType* component_type = mono_reflection_type_get_type(reflection_type);
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
