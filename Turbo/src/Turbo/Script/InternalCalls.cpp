#include "tbopch.h"
#include "InternalCalls.h"

#include "Script.h"
#include "ScriptInstance.h"

#include "Turbo/Renderer/SceneDrawList.h"

#include "Turbo/Physics/PhysicsWorld2D.h"
#include "Turbo/Physics/PhysicsWorld.h"
#include "Turbo/Physics/JoltUtils.h"

#include "Turbo/Audio/Audio.h"
#include "Turbo/Asset/AssetManager.h"
#include "Turbo/Core/Application.h"
#include "Turbo/Core/Input.h"
#include "Turbo/Core/Math.h"

#include "Turbo/Solution/Project.h"

#include "Turbo/Scene/Entity.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

#define TBO_REGISTER_FUNCTION(name) mono_add_internal_call("Turbo.InternalCalls::" #name, (const void*)IC::name);

namespace Turbo {

    static Entity GetEntity(u64 uuid)
    {
        auto context = Script::GetCurrentScene();
        Entity entity = context->FindEntityByUUID(uuid);

        if (!entity) [[unlikely]]
            TBO_CONSOLE_ERROR("Entity doesnt exist!");

            return entity;
    }

    static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;
    static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityAddComponentFuncs;
    static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityRemoveComponentFuncs;

    namespace IC {

#pragma region Application

        static u32 Application_GetWidth()
        {
            auto context = Script::GetCurrentScene();
            return context->GetViewportWidth();
        }

        static u32 Application_GetHeight()
        {
            auto context = Script::GetCurrentScene();
            return context->GetViewportHeight();
        }

        static void Application_Close()
        {
            auto context = Script::GetCurrentScene();
            Application::Get().Close();
        }

#pragma endregion

#pragma region Assets

        // @return Asset handle
        static u64 Assets_Load_Prefab(MonoString* path)
        {
            char* cPath = mono_string_to_utf8(path);
            Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(cPath);
            mono_free(cPath);

            return prefab ? static_cast<u64>(prefab->Handle) : 0;

        }
#pragma endregion

#pragma region Debug

        static void DebugRenderer_DrawLine(glm::vec3* start, glm::vec3* end, glm::vec4* color)
        {
            Ref<Scene> context = Script::GetCurrentScene();
            context->AddToDrawList([s = *start, e = *end, c = *color](Ref<SceneDrawList> drawList)
            {
                drawList->AddLine(s, e, c);
            });
        }

        static void DebugRenderer_DrawCircle(glm::vec3* position, glm::vec3* rotation, float radius, glm::vec4* color)
        {
            Ref<Scene> context = Script::GetCurrentScene();
            context->AddToDrawList([p = *position, r = *rotation, radius, c = *color](Ref<SceneDrawList> drawList)
            {
                drawList->AddDebugCircle(p, r, radius, c);
            });
        }

        static void DebugRenderer_DrawBox(glm::vec3* position, glm::vec3* rotation, glm::vec3* scale, glm::vec4* color)
        {
            Ref<Scene> context = Script::GetCurrentScene();
            context->AddToDrawList([p = *position, r = *rotation, s = *scale, c = *color](Ref<SceneDrawList> drawList)
            {
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), p)
                    * glm::toMat4(glm::quat(r))
                    * glm::scale(glm::mat4(1.0f), s);

                drawList->AddBoxWireframe(transform, c);
            });
        }

#pragma endregion

#pragma region Logging

        static void Log_String(u32 level, MonoString* string)
        {
            char* msg = mono_string_to_utf8(string);

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

            mono_free(msg);
        }

#pragma endregion

#pragma region Input

        static bool Input_IsKeyDown(u32 key)
        {
            return Input::IsKeyPressed(key);
        }
        static bool Input_IsKeyUp(u32 key)
        {
            return Input::IsKeyReleased(key);
        }
        static bool Input_IsMouseButtonDown(u32 key)
        {
            return Input::IsMouseButtonPressed(key);
        }
        static bool Input_IsMouseButtonUp(u32 key)
        {
            return Input::IsMouseButtonReleased(key);
        }
        static void Input_GetMousePosition(glm::vec2* mousePosition)
        {
            auto [x, y] = Input::GetMousePosition();
            mousePosition->x = (f32)x;
            mousePosition->y = (f32)y;
        }

        static void Input_SetCursorMode(u32 cursorMode)
        {
            Input::SetCursorMode((CursorMode)cursorMode);
        }

#pragma endregion

#pragma region Physics2D

        static u64 Physics2D_RayCast(glm::vec2 a, glm::vec2 b)
        {
            auto context = Script::GetCurrentScene();

            Ref<PhysicsWorld2D> physicsWorld2d = context->GetPhysicsWorld2D();

            Entity hitEntity = physicsWorld2d->RayCast(a, b);
            if (hitEntity)
                return hitEntity.GetUUID();

            return 0;
        }

#pragma endregion

#pragma region Physics

        static void Physics_CastRay(glm::vec3* origin, glm::vec3* direction, f32 length, u32 rayTarget, CastRayResult* outResult)
        {
            if (glm::isinf(length))
            {
                length = length >= 0 ? std::numeric_limits<float>::max() : std::numeric_limits<float>::lowest();
            }

            Ref<PhysicsWorld> physicsWorld = Script::GetCurrentScene()->GetPhysicsWorld();
            *outResult = physicsWorld->CastRay(Ray(*origin, (*direction) * length), static_cast<RayTarget>(rayTarget));
        }

        static MonoArray* Physics_CastRayAll(glm::vec3* origin, glm::vec3* direction, f32 length)
        {
            if (glm::isinf(length))
            {
                length = length >= 0 ? std::numeric_limits<float>::max() : std::numeric_limits<float>::lowest();
            }

            Ref<PhysicsWorld> physicsWorld = Script::GetCurrentScene()->GetPhysicsWorld();
            Ray ray(*origin, (*direction) * length);

            std::vector<CastRayResult> results = physicsWorld->CastRay(Ray(*origin, (*direction) * length));

            MonoArray* castRayResults = nullptr;

            if (results.size())
            {
                castRayResults = mono_array_new(Script::GetAppDomain(), Script::GetMonoClassFromName("Turbo", "InternalCastRayResult"), results.size());

                for (u64 i = 0; i < results.size(); ++i)
                {
                    mono_array_set(castRayResults, CastRayResult, i, results[i]);
                }
            }

            return castRayResults;
        }

#pragma endregion

#pragma region Scene

        static u64 Scene_CreateEntity(u64 parentUUID, MonoString* name)
        {
            char* cString = mono_string_to_utf8(name);
            auto context = Script::GetCurrentScene();
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
            auto context = Script::GetCurrentScene().Get(); // FIXME: lambda cannot accept const ref

            context->AddToPostUpdate([context, entity]() { context->DestroyEntity(entity); });
        }

        static void Scene_ScreenToWorldPosition(glm::vec2 screenPosition, glm::vec3* worldPosition) // FIXME: Raycasting
        {
            auto context = Script::GetCurrentScene();
            Entity camera = context->FindPrimaryCameraEntity();

            if (!camera)
            {
                TBO_CONSOLE_ERROR("No active camera!");
                return;
            }

            auto [transform, cc] = camera.GetComponent<TransformComponent, CameraComponent>();

            glm::vec4 viewport =
            {
                context->GetViewportX(),
                context->GetViewportY(),
                context->GetViewportWidth(),
                context->GetViewportHeight()
            };

            *worldPosition = Math::ScreenToRayDirection(screenPosition, viewport, cc.Camera.GetViewProjection());
            //*worldPosition = Math::UnProject(screenPosition, viewport, cc.Camera.GetViewProjection());
        }

        static void Scene_WorldToScreenPosition(glm::vec3 worldPosition, glm::vec2* screenPosition) // FIXME: Raycasting
        {
            auto context = Script::GetCurrentScene();
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

            Entity entity = Script::GetCurrentScene()->FindEntityByName(cString);

            u64 uuid = 0;
            if (entity)
            {
                uuid = entity.GetUUID();
            }
            else
            {
                TBO_CONSOLE_ERROR("Could not find entity with name \"{}\"", cString);
            }

            mono_free(cString);
            return uuid;
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

            // FIXME: Rewrite, this is garbage

            // Allocate new entities
            for (uintptr_t i = 0; i < children.size(); i++)
            {
                Ref<ScriptInstance> instance = Ref<ScriptInstance>::Create(entityClass, children[i]);
                mono_array_setref(monoArray, i, instance->GetMonoInstance());
            }

            return monoArray;
        }

        static void Entity_UnParent(u64 uuid)
        {
            auto context = Script::GetCurrentScene();
            Entity entity = context->FindEntityByUUID(uuid);

            if (!entity) [[unlikely]] {
                TBO_CONSOLE_ERROR("Entity doesnt exist!");
            }

            entity.UnParent();
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
            auto context = Script::GetCurrentScene();

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

        static u64 Entity_InstantiatePrefab(u64 prefabID)
        {
            auto context = Script::GetCurrentScene();

            Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(prefabID);

            if (prefab)
            {
                Entity entity = context->CreatePrefabEntity(prefab->GetPrefabEntity(), {});

                return entity.GetUUID();
            }

            TBO_CONSOLE_ERROR("Could not instantiate prefab!");
            return 0;
        }

        static u64 Entity_InstantiatePrefabWithTranslation(u64 prefabID, glm::vec3* translation)
        {
            auto context = Script::GetCurrentScene();

            Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(prefabID);

            if (prefab)
            {
                Entity entity = context->CreatePrefabEntity(prefab->GetPrefabEntity(), {}, translation);
                return entity.GetUUID();
            }

            TBO_CONSOLE_ERROR("Could not instantiate prefab!");
            return 0;
        }

        static u64 Entity_InstantiateChildPrefabWithTranslation(u64 prefabID, MonoString* monoString, glm::vec3* translation)
        {
            TBO_ENGINE_ASSERT(false);

            auto context = Script::GetCurrentScene();

            Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(prefabID);

            if (prefab)
            {
                Entity entity = context->CreatePrefabEntity(prefab->GetPrefabEntity(), {}, translation);
                return entity.GetUUID();
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

#pragma region ScriptComponent

        static MonoObject* Component_Script_Get_Instance(UUID uuid)
        {
            Ref<ScriptInstance> instance = Script::FindEntityInstance(uuid);
            return instance ? instance->GetMonoInstance() : nullptr;
        }

#pragma endregion

#pragma region LineRendererComponent

        static void Component_LineRenderer_Get_Destination(UUID uuid, glm::vec3* destination)
        {
            Entity entity = GetEntity(uuid);
            if (entity)
                *destination = entity.GetComponent<LineRendererComponent>().Destination;
        }

        static void Component_LineRenderer_Set_Destination(UUID uuid, glm::vec3* destination)
        {
            Entity entity = GetEntity(uuid);

            if (entity)
                entity.GetComponent<LineRendererComponent>().Destination = *destination;
        }

        static void Component_LineRenderer_Get_Color(UUID uuid, glm::vec4* outColor)
        {
            Entity entity = GetEntity(uuid);

            if (entity)
                *outColor = entity.GetComponent<LineRendererComponent>().Color;
        }
        static void Component_LineRenderer_Set_Color(UUID uuid, glm::vec4* color)
        {
            Entity entity = GetEntity(uuid);

            if (entity)
                entity.GetComponent<LineRendererComponent>().Color = *color;
        }

#pragma endregion

#pragma region SpriteRendererComponent

        static void Component_SpriteRenderer_Get_Color(UUID uuid, glm::vec4* outColor)
        {
            Entity entity = GetEntity(uuid);

            *outColor = entity.GetComponent<SpriteRendererComponent>().Color;
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

            if (src.IsSpriteSheet && src.Texture)
            {
                auto texture = AssetManager::GetAsset<Texture2D>(src.Texture);
                src.SpriteCoords = position;
                src.SpriteSize = size;

                glm::vec2 min = { (src.SpriteCoords.x * src.SpriteSize.x) / texture->GetWidth(), (src.SpriteCoords.y * src.SpriteSize.y) / texture->GetHeight() };
                glm::vec2 max = { ((src.SpriteCoords.x + 1) * src.SpriteSize.x) / texture->GetWidth(), ((src.SpriteCoords.y + 1) * src.SpriteSize.y) / texture->GetHeight() };

                src.TextureCoords[0] = { min.x, min.y };
                src.TextureCoords[1] = { max.x, min.y };
                src.TextureCoords[2] = { max.x, max.y };
                src.TextureCoords[3] = { min.x, max.y };

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
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            b2Body* body = (b2Body*)rb2d.RuntimeBody;

            body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(worldPosition->x, worldPosition->y), wake);
        }
        static void Component_Rigidbody2D_ApplyLinearImpulseToCenter(UUID uuid, glm::vec2* impulse, bool wake)
        {
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
        }
        static void Component_Rigidbody2D_ApplyForceToCenter(UUID uuid, glm::vec2* force, bool wake)
        {
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            body->ApplyForceToCenter(b2Vec2(force->x, force->y), wake);
        }
        static void Component_Rigidbody2D_Set_LinearVelocity(UUID uuid, glm::vec2* velocity)
        {
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            body->SetLinearVelocity(b2Vec2(velocity->x, velocity->y));
        }
        static void Component_Rigidbody2D_Get_LinearVelocity(UUID uuid, glm::vec2* velocity)
        {
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            b2Vec2 b2Velocity = body->GetLinearVelocity();
            *velocity = { b2Velocity.x, b2Velocity.y };
        }
        static void Component_Rigidbody2D_ApplyTorque(UUID uuid, f32 torque, bool wake)
        {
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            body->ApplyTorque(torque, wake);
        }

        static f32 Component_Rigidbody2D_Get_GravityScale(UUID uuid)
        {
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            return rb2d.GravityScale;
        }
        static void Component_Rigidbody2D_Set_GravityScale(UUID uuid, f32 gravityScale)
        {
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            rb2d.GravityScale = gravityScale;

            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            body->SetGravityScale(gravityScale);
        }

        static u32 Component_Rigidbody2D_Get_BodyType(UUID uuid)
        {
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            return (u32)rb2d.Type;
        }

        static void Component_Rigidbody2D_Set_BodyType(UUID uuid, u32 type)
        {
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            rb2d.Type = (RigidbodyType)type;

            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            body->SetType(static_cast<b2BodyType>(rb2d.Type));
        }

        static void Component_Rigidbody2D_Set_Enabled(UUID uuid, bool enabled)
        {
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            rb2d.Enabled = enabled;
        }

        static bool Component_Rigidbody2D_Get_Enabled(UUID uuid)
        {
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            return rb2d.Enabled;
        }

        static void Component_Rigidbody2D_Set_ContactEnabled(UUID uuid, bool enabled)
        {
            Entity entity = GetEntity(uuid);

            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            rb2d.ContactEnabled = enabled;

            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            body->SetEnabled(enabled);
        }

        static bool Component_Rigidbody2D_Get_ContactEnabled(UUID uuid)
        {
            Entity entity = GetEntity(uuid);

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

            auto [transform, bc2d] = entity.GetComponent<TransformComponent, BoxCollider2DComponent>();
            if (bc2d.Offset == *offset)
                return;

            bc2d.Offset = *offset;

            b2Fixture* fixture = (b2Fixture*)bc2d.RuntimeFixture;
            b2PolygonShape* polygonShape = (b2PolygonShape*)fixture->GetShape();
            polygonShape->SetAsBox(bc2d.Size.x * glm::abs(transform.Scale.x), bc2d.Size.y * glm::abs(transform.Scale.y), b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);
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

            auto [transform, bc2d] = entity.GetComponent<TransformComponent, BoxCollider2DComponent>();

            if (bc2d.Size == *size)
                return;

            bc2d.Size = *size;

            b2Fixture* fixture = (b2Fixture*)bc2d.RuntimeFixture;
            b2PolygonShape* polygonShape = (b2PolygonShape*)fixture->GetShape();
            polygonShape->SetAsBox(bc2d.Size.x * glm::abs(transform.Scale.x), bc2d.Size.y * glm::abs(transform.Scale.y), b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);
        }

        // IsSensor
        static bool Component_BoxCollider2D_Get_IsTrigger(UUID uuid)
        {
            Entity entity = GetEntity(uuid);

            return entity.GetComponent<BoxCollider2DComponent>().IsTrigger;
        }

        static void Component_BoxCollider2D_Set_IsTrigger(UUID uuid, bool isTrigger)
        {
            Entity entity = GetEntity(uuid);

            auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

            if (bc2d.IsTrigger == isTrigger)
                return;

            bc2d.IsTrigger = isTrigger;
            b2Fixture* fixture = (b2Fixture*)bc2d.RuntimeFixture;
            fixture->SetSensor(bc2d.IsTrigger);
        }

        static void Component_BoxCollider2D_Set_CollisionFilter(UUID uuid, u16 category, u16 mask)
        {
            Entity entity = GetEntity(uuid);

            auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

            if (bc2d.CollisionCategory == category && bc2d.CollisionMask == mask)
                return;

            bc2d.CollisionCategory = category;
            bc2d.CollisionMask = mask;

            if (!entity.HasComponent<Rigidbody2DComponent>())
            {
                TBO_CONSOLE_ERROR("Entity does not have a rigidbody!");
                return;
            }

            b2Fixture* fixture = (b2Fixture*)bc2d.RuntimeFixture;

            b2Filter collisionFilter;
            collisionFilter.categoryBits = bc2d.CollisionCategory;
            collisionFilter.maskBits = bc2d.CollisionMask;
            collisionFilter.groupIndex = bc2d.CollisionGroup;
            fixture->SetFilterData(collisionFilter);
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

            auto [transform, cc2d] = entity.GetComponent<TransformComponent, CircleCollider2DComponent>();
            if (cc2d.Offset == *offset)
                return;

            cc2d.Offset = *offset;

            b2Fixture* fixture = (b2Fixture*)cc2d.RuntimeFixture;
            b2CircleShape* circleShape = (b2CircleShape*)fixture->GetShape();
            circleShape->m_radius = glm::abs(transform.Scale.x) * cc2d.Radius;
            circleShape->m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
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

            auto [transform, cc2d] = entity.GetComponent<TransformComponent, CircleCollider2DComponent>();
            if (cc2d.Radius == *radius)
                return;

            cc2d.Radius = *radius;

            b2Fixture* fixture = (b2Fixture*)cc2d.RuntimeFixture;
            b2CircleShape* circleShape = (b2CircleShape*)fixture->GetShape();
            circleShape->m_radius = transform.Scale.x * cc2d.Radius;
            circleShape->m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
        }

        static void Component_CircleCollider2D_Set_CollisionFilter(UUID uuid, u16 category, u16 mask)
        {
            Entity entity = GetEntity(uuid);

            auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

            if (cc2d.CollisionCategory == category && cc2d.CollisionMask == mask)
                return;

            cc2d.CollisionCategory = category;
            cc2d.CollisionMask = mask;

            b2Fixture* fixture = (b2Fixture*)cc2d.RuntimeFixture;

            b2Filter collisionFilter;
            collisionFilter.categoryBits = cc2d.CollisionCategory;
            collisionFilter.maskBits = cc2d.CollisionMask;
            collisionFilter.groupIndex = cc2d.CollisionGroup;
            fixture->SetFilterData(collisionFilter);
        }

        static void Component_CircleCollider2D_Get_CollisionFilter(UUID uuid, u16* category, u16* mask)
        {
            Entity entity = GetEntity(uuid);

            auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
            *category = cc2d.CollisionCategory;
            *mask = cc2d.CollisionMask;
        }

#pragma endregion

#pragma region RigidBodyComponent

        // Linear velocity

        static u32 Component_Rigidbody_Get_BodyType(UUID uuid)
        {
            Entity entity = GetEntity(uuid);
            auto scene = Script::GetCurrentScene();

            // We can now use the unsafe variant since all contact invocations are processed after the simulation
            auto& bodyInterface = scene->GetPhysicsWorld()->GetBodyInterfaceUnsafe();

            auto bodyId = JPH::BodyID(entity.GetComponent<RigidbodyComponent>().RuntimeBodyHandle);

            return (u32)(bodyInterface.GetMotionType(bodyId));
        }

        static void Component_Rigidbody_Set_BodyType(UUID uuid, u32 rigidbodyType)
        {
            Entity entity = GetEntity(uuid);
            auto scene = Script::GetCurrentScene();

            auto& rb = entity.GetComponent<RigidbodyComponent>();
            if (rb.Type == static_cast<RigidbodyType>(rigidbodyType))
                return;

            rb.Type = static_cast<RigidbodyType>(rigidbodyType);

            // We can now use the unsafe variant since all contact invocations are processed after the simulation
            auto& bodyInterface = scene->GetPhysicsWorld()->GetBodyInterfaceUnsafe();

            auto bodyId = JPH::BodyID(entity.GetComponent<RigidbodyComponent>().RuntimeBodyHandle);
            bodyInterface.SetMotionType(bodyId, static_cast<JPH::EMotionType>(rigidbodyType), JPH::EActivation::DontActivate);
        }

        // Angular velocity
        static void Component_Rigidbody_Get_AngularVelocity(UUID uuid, glm::vec3* velocity)
        {
            Entity entity = GetEntity(uuid);
            auto scene = Script::GetCurrentScene();

            // We can now use the unsafe variant since all contact invocations are processed after the simulation
            auto& bodyInterface = scene->GetPhysicsWorld()->GetBodyInterfaceUnsafe();

            auto bodyId = JPH::BodyID(entity.GetComponent<RigidbodyComponent>().RuntimeBodyHandle);
            JPH::Vec3 bodyVelocity = bodyInterface.GetAngularVelocity(bodyId);
            *velocity = JoltUtils::GetVec3(bodyInterface.GetAngularVelocity(bodyId));
        }

        static void Component_Rigidbody_Set_AngularVelocity(UUID uuid, glm::vec3* velocity)
        {
            Entity entity = GetEntity(uuid);
            auto scene = Script::GetCurrentScene();

            // We can now use the unsafe variant since all contact invocations are processed after the simulation
            auto& bodyInterface = scene->GetPhysicsWorld()->GetBodyInterfaceUnsafe();

            auto bodyId = JPH::BodyID(entity.GetComponent<RigidbodyComponent>().RuntimeBodyHandle);
            bodyInterface.SetAngularVelocity(bodyId, JPH::Vec3(velocity->x, velocity->y, velocity->z));
        }

        // Linear velocity
        static void Component_Rigidbody_Get_LinearVelocity(UUID uuid, glm::vec3* velocity)
        {
            Entity entity = GetEntity(uuid);
            auto scene = Script::GetCurrentScene();

            // We can now use the unsafe variant since all contact invocations are processed after the simulation
            auto& bodyInterface = scene->GetPhysicsWorld()->GetBodyInterfaceUnsafe();

            auto bodyId = JPH::BodyID(entity.GetComponent<RigidbodyComponent>().RuntimeBodyHandle);
            JPH::Vec3 bodyVelocity = bodyInterface.GetLinearVelocity(bodyId);
            *velocity = JoltUtils::GetVec3(bodyInterface.GetLinearVelocity(bodyId));
        }

        static void Component_Rigidbody_Set_LinearVelocity(UUID uuid, glm::vec3* velocity)
        {
            Entity entity = GetEntity(uuid);
            auto scene = Script::GetCurrentScene();

            // We can now use the unsafe variant since all contact invocations are processed after the simulation
            auto& bodyInterface = scene->GetPhysicsWorld()->GetBodyInterfaceUnsafe();

            auto bodyId = JPH::BodyID(entity.GetComponent<RigidbodyComponent>().RuntimeBodyHandle);
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3(velocity->x, velocity->y, velocity->z));
        }

        // Position
        // TODO: Remove this function
        static void Component_Rigidbody_Get_Position(UUID uuid, glm::vec3* outPosition)
        {
            Entity entity = GetEntity(uuid);
            auto scene = Script::GetCurrentScene();

            // We can now use the unsafe variant since all contact invocations are processed after the simulation
            auto& bodyInterface = scene->GetPhysicsWorld()->GetBodyInterfaceUnsafe();

            auto bodyId = JPH::BodyID(entity.GetComponent<RigidbodyComponent>().RuntimeBodyHandle);
            *outPosition = JoltUtils::GetVec3(bodyInterface.GetCenterOfMassPosition(bodyId));
        }

        static void Component_Rigidbody_Set_Position(UUID uuid, glm::vec3* position)
        {
            Entity entity = GetEntity(uuid);

            if (entity.Transform().Translation == *position)
                return;

            auto scene = Script::GetCurrentScene();

            // We can now use the unsafe variant since all contact invocations are processed after the simulation
            auto& bodyInterface = scene->GetPhysicsWorld()->GetBodyInterfaceUnsafe();
            auto bodyId = JPH::BodyID(entity.GetComponent<RigidbodyComponent>().RuntimeBodyHandle);

            //// Also reset linear and angular velocity to avoid jiggering
            //bodyInterface.SetLinearAndAngularVelocity(bodyId, JPH::Vec3::sZero(), JPH::Vec3::sZero());
            bodyInterface.SetPosition(bodyId, JoltUtils::GetVec3(*position), JPH::EActivation::Activate);
        }

        // Rotation
        static void Component_Rigidbody_Get_Rotation(UUID uuid, glm::quat* outRotation)
        {
            Entity entity = GetEntity(uuid);
            auto scene = Script::GetCurrentScene();

            // We can now use the unsafe variant since all contact invocations are processed after the simulation
            auto& bodyInterface = scene->GetPhysicsWorld()->GetBodyInterfaceUnsafe();

            auto bodyId = JPH::BodyID(entity.GetComponent<RigidbodyComponent>().RuntimeBodyHandle);
            *outRotation = JoltUtils::GetQuat(bodyInterface.GetRotation(bodyId));
        }

        static void Component_Rigidbody_Set_Rotation(UUID uuid, glm::quat* rotation)
        {
            Entity entity = GetEntity(uuid);

            auto scene = Script::GetCurrentScene();

            // We can now use the unsafe variant since all contact invocations are processed after the simulation
            auto& bodyInterface = scene->GetPhysicsWorld()->GetBodyInterfaceUnsafe();

            auto bodyId = JPH::BodyID(entity.GetComponent<RigidbodyComponent>().RuntimeBodyHandle);
            bodyInterface.SetRotation(bodyId, JoltUtils::GetQuat(*rotation), JPH::EActivation::Activate);
        }

        // TODO: Abstract this
        static void Component_Rigidbody_Rotate(UUID uuid, glm::vec3* rotation)
        {
            Entity entity = GetEntity(uuid);
            auto scene = Script::GetCurrentScene();

            // We can now use the unsafe variant since all contact invocations are processed after the simulation
            auto& bodyInterface = scene->GetPhysicsWorld()->GetBodyInterfaceUnsafe();

            auto bodyId = JPH::BodyID(entity.GetComponent<RigidbodyComponent>().RuntimeBodyHandle);
            JPH::Quat jphRotation = bodyInterface.GetRotation(bodyId);

            glm::quat qRotation = { jphRotation.GetW(), jphRotation.GetX(), jphRotation.GetY(), jphRotation.GetZ() };
            qRotation *= glm::angleAxis(rotation->x, glm::vec3(1.0f, 0.0f, 0.0f))
                * glm::angleAxis(rotation->y, glm::vec3(0.0f, 1.0f, 0.0f))
                * glm::angleAxis(rotation->z, glm::vec3(0.0f, 0.0f, 1.0f));

            bodyInterface.SetRotation(bodyId, JPH::Quat(qRotation.x, qRotation.y, qRotation.z, qRotation.w), JPH::EActivation::Activate);
        }

        enum ForceMode : u32 { Force = 0, Impulse };

        // TODO: Abstract this
        static void Component_Rigidbody_AddForce(UUID uuid, glm::vec3* force, ForceMode forceMode)
        {
            Entity entity = GetEntity(uuid);

            auto scene = Script::GetCurrentScene();

            auto bodyId = JPH::BodyID(entity.GetComponent<RigidbodyComponent>().RuntimeBodyHandle);
            auto& bodyInterface = scene->GetPhysicsWorld()->GetBodyInterfaceUnsafe();

            if (forceMode == ForceMode::Force)
            {
                bodyInterface.AddForce(bodyId, JoltUtils::GetVec3(*force));
            }
            else if (forceMode == ForceMode::Impulse)
            {
                bodyInterface.AddImpulse(bodyId, JoltUtils::GetVec3(*force));
            }
        }

#pragma endregion

#pragma endregion
    }

    void InternalCalls::Init()
    {
        // Application
        TBO_REGISTER_FUNCTION(Application_GetWidth);
        TBO_REGISTER_FUNCTION(Application_GetHeight);
        TBO_REGISTER_FUNCTION(Application_Close);

        // Asset
        TBO_REGISTER_FUNCTION(Assets_Load_Prefab);

        // Debug
        TBO_REGISTER_FUNCTION(DebugRenderer_DrawLine);
        TBO_REGISTER_FUNCTION(DebugRenderer_DrawCircle);
        TBO_REGISTER_FUNCTION(DebugRenderer_DrawBox);

        // Logging
        TBO_REGISTER_FUNCTION(Log_String);

        // Scene
        TBO_REGISTER_FUNCTION(Scene_CreateEntity);
        TBO_REGISTER_FUNCTION(Scene_DestroyEntity);
        TBO_REGISTER_FUNCTION(Scene_ScreenToWorldPosition);
        TBO_REGISTER_FUNCTION(Scene_WorldToScreenPosition);

        // Entity
        TBO_REGISTER_FUNCTION(Entity_FindEntityByName);
        TBO_REGISTER_FUNCTION(Entity_Get_Name);
        TBO_REGISTER_FUNCTION(Entity_Set_Name);
        TBO_REGISTER_FUNCTION(Entity_Has_Component);
        TBO_REGISTER_FUNCTION(Entity_Add_Component);
        TBO_REGISTER_FUNCTION(Entity_Remove_Component);
        TBO_REGISTER_FUNCTION(Entity_Get_Children);
        TBO_REGISTER_FUNCTION(Entity_UnParent);

        // Prefab
        TBO_REGISTER_FUNCTION(Entity_InstantiatePrefab);
        TBO_REGISTER_FUNCTION(Entity_InstantiatePrefabWithTranslation);
        TBO_REGISTER_FUNCTION(Entity_InstantiateChildPrefabWithTranslation);

        // Input
        TBO_REGISTER_FUNCTION(Input_IsKeyUp);
        TBO_REGISTER_FUNCTION(Input_IsKeyDown);
        TBO_REGISTER_FUNCTION(Input_IsMouseButtonUp);
        TBO_REGISTER_FUNCTION(Input_IsMouseButtonDown);
        TBO_REGISTER_FUNCTION(Input_GetMousePosition);
        TBO_REGISTER_FUNCTION(Input_SetCursorMode);

        // Transform
        TBO_REGISTER_FUNCTION(Component_Transform_Get_Translation);
        TBO_REGISTER_FUNCTION(Component_Transform_Set_Translation);
        TBO_REGISTER_FUNCTION(Component_Transform_Get_Rotation);
        TBO_REGISTER_FUNCTION(Component_Transform_Set_Rotation);
        TBO_REGISTER_FUNCTION(Component_Transform_Get_Scale);
        TBO_REGISTER_FUNCTION(Component_Transform_Set_Scale);

        // Script
        TBO_REGISTER_FUNCTION(Component_Script_Get_Instance);

        // Line Renderer
        TBO_REGISTER_FUNCTION(Component_LineRenderer_Get_Destination);
        TBO_REGISTER_FUNCTION(Component_LineRenderer_Set_Destination);
        TBO_REGISTER_FUNCTION(Component_LineRenderer_Get_Color);
        TBO_REGISTER_FUNCTION(Component_LineRenderer_Set_Color);

        // Sprite Renderer
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

        // Physics 2D
        TBO_REGISTER_FUNCTION(Physics2D_RayCast);

        // Rigidbody2D
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
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Get_IsTrigger);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Set_IsTrigger);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Set_CollisionFilter);
        TBO_REGISTER_FUNCTION(Component_BoxCollider2D_Get_CollisionFilter);

        // CircleCollider2D
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Get_Offset);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Set_Offset);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Get_Radius);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Set_Radius);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Set_CollisionFilter);
        TBO_REGISTER_FUNCTION(Component_CircleCollider2D_Get_CollisionFilter);

        // Physics 3D
        TBO_REGISTER_FUNCTION(Physics_CastRay);
        TBO_REGISTER_FUNCTION(Physics_CastRayAll);

        // Rigidbody
        TBO_REGISTER_FUNCTION(Component_Rigidbody_Get_LinearVelocity);
        TBO_REGISTER_FUNCTION(Component_Rigidbody_Set_LinearVelocity);
        TBO_REGISTER_FUNCTION(Component_Rigidbody_Get_AngularVelocity);
        TBO_REGISTER_FUNCTION(Component_Rigidbody_Set_AngularVelocity);
        TBO_REGISTER_FUNCTION(Component_Rigidbody_Get_Position);
        TBO_REGISTER_FUNCTION(Component_Rigidbody_Set_Position);
        TBO_REGISTER_FUNCTION(Component_Rigidbody_Get_Rotation);
        TBO_REGISTER_FUNCTION(Component_Rigidbody_Set_Rotation);
        TBO_REGISTER_FUNCTION(Component_Rigidbody_Rotate);
        TBO_REGISTER_FUNCTION(Component_Rigidbody_AddForce);

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
            std::string managedTypename = std::format("Turbo.{}", structName);

            MonoType* type = mono_reflection_type_from_name(managedTypename.data(), Script::GetCoreAssemblyImage());
            if (!type)
            {
                TBO_ENGINE_ERROR("Could not find component type {0}", managedTypename);
                return;
            }

            s_EntityHasComponentFuncs[type] = [](Entity entity) { return entity.HasComponent<Component>(); };
            s_EntityAddComponentFuncs[type] = [](Entity entity)
            {
                entity.AddComponent<Component>();

                // TODO: Should having a collider be mandatory for 2D physics? 

                if constexpr (std::is_same_v<Component, Rigidbody2DComponent>)
                {
                    Script::GetCurrentScene()->GetPhysicsWorld2D()->CreateRigidbody(entity);
                }
                else if constexpr (std::is_same_v<Component, RigidbodyComponent>)
                {
                    Script::GetCurrentScene()->GetPhysicsWorld()->CreateRigidbody(entity);
                }
            };
            s_EntityRemoveComponentFuncs[type] = [](Entity entity)
            {
                entity.RemoveComponent<Component>();
            };
        }(), ...);
    }

    void InternalCalls::RegisterComponents()
    {
        s_EntityHasComponentFuncs.clear();

        RegisterComponent(AllComponents{});
    }

}
