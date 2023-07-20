#include "tbopch.h"
#include "Scene.h"

#include "Entity.h"
#include "SceneDrawList.h"
#include "SceneCamera.h"

#include "Turbo/Asset/AssetManager.h"
#include "Turbo/Audio/Audio.h"
#include "Turbo/Core/KeyCodes.h"
#include "Turbo/Debug/ScopeTimer.h"
#include "Turbo/Script/Script.h"
#include "Turbo/Renderer/Mesh.h"

#include "Turbo/Physics/Physics2D.h" // <-- TODO: Remove
#include "Turbo/Physics/PhysicsWorld2D.h"

namespace Turbo
{
    namespace Utils
    {
        template<typename... Components>
        static void CopyComponent(entt::registry& dst, entt::registry& src, const EntityMap& enttMap)
        {
            ([&]()
            {
                auto view = src.view<Components>();

                for (auto srcEntity : view)
                {
                    entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);
                    auto& srcComponent = src.get<Components>(srcEntity);

                    dst.emplace_or_replace<Components>(dstEntity, srcComponent);
                }
            }(), ...);

        }

        template<typename... Components>
        static void CopyComponents(ComponentGroup<Components...>, entt::registry& dst, entt::registry& src, const EntityMap& enttMap)
        {
            CopyComponent<Components...>(dst, src, enttMap);
        }

        template<typename... Component>
        static void CopyComponentIfExists(Entity dst, Entity src)
        {
            ([&]()
            {
                if (src.HasComponent<Component>())
                    dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
            }(), ...);
        }

        template<typename... Component>
        static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
        {
            CopyComponentIfExists<Component...>(dst, src);
        }
    }

    struct SceneComponent
    {
        UUID SceneID;
    };

    struct PhysicsWorld2DComponent
    {
        std::unique_ptr<PhysicsWorld2D> World;
    };

    Scene::Scene()
    {
        m_SceneEntity = m_Registry.create();
        m_Registry.emplace<SceneComponent>(m_SceneEntity, m_SceneID);

        m_Registry.on_construct<AudioSourceComponent>().connect<&Scene::OnAudioSourceComponentConstruct>(this);
        m_Registry.on_destroy<AudioSourceComponent>().connect<&Scene::OnAudioSourceComponentDestroy>(this);
        m_Registry.on_construct<ScriptComponent>().connect<&Scene::OnScriptComponentConstruct>(this);
        m_Registry.on_destroy<ScriptComponent>().connect<&Scene::OnScriptComponentDestroy>(this);
        m_Registry.on_construct<Rigidbody2DComponent>().connect<&Scene::OnRigidBody2DComponentConstruct>(this);
        m_Registry.on_destroy<Rigidbody2DComponent>().connect<&Scene::OnRigidBody2DComponentDestroy>(this);
        m_Registry.on_construct<BoxCollider2DComponent>().connect<&Scene::OnBoxCollider2DComponentConstruct>(this);
        m_Registry.on_update<BoxCollider2DComponent>().connect<&Scene::OnBoxCollider2DComponentUpdate>(this);
        m_Registry.on_destroy<BoxCollider2DComponent>().connect<&Scene::OnBoxCollider2DComponentDestroy>(this);
        m_Registry.on_construct<CircleCollider2DComponent>().connect<&Scene::OnCircleCollider2DComponentConstruct>(this);
        m_Registry.on_update<CircleCollider2DComponent>().connect<&Scene::OnCircleCollider2DComponentUpdate>(this);
        m_Registry.on_destroy<CircleCollider2DComponent>().connect<&Scene::OnCircleCollider2DComponentDestroy>(this);

        m_Registry.reserve(200);
    }

    Scene::~Scene()
    {
        m_Registry.clear();

        m_Registry.on_construct<ScriptComponent>().disconnect(this);
        m_Registry.on_destroy<ScriptComponent>().disconnect(this);

        m_Registry.on_construct<AudioSourceComponent>().disconnect(this);
        m_Registry.on_destroy<AudioSourceComponent>().disconnect(this);

        m_Registry.on_construct<Rigidbody2DComponent>().disconnect(this);
        m_Registry.on_destroy<Rigidbody2DComponent>().disconnect(this);
    }

    void Scene::OnRuntimeStart()
    {
        m_Running = true;

        // Find primary camera
        m_PrimaryCameraEntity = FindPrimaryCameraEntity();

        // Physics 2D
        auto& physicsWorld2d = m_Registry.emplace<PhysicsWorld2DComponent>(m_SceneEntity, std::make_unique<PhysicsWorld2D>(glm::vec2{ 0.0f, -9.8f })).World;

        {
            auto& view = GetAllEntitiesWith<Rigidbody2DComponent>();
            for (auto e : view)
            {
                Entity entity = { e, this };
                physicsWorld2d->ConstructBody(entity);
            }
        }

        {
            auto& view = GetAllEntitiesWith<BoxCollider2DComponent>();
            for (auto e : view)
            {
                Entity entity = { e, this };
                physicsWorld2d->ConstructBoxCollider(entity);
            }
        }

        {
            auto& view = GetAllEntitiesWith<CircleCollider2DComponent>();
            for (auto e : view)
            {
                Entity entity = { e, this };
                physicsWorld2d->ConstructCircleCollider(entity);
            }
        }

        Audio::OnRuntimeStart(this);
        Script::OnRuntimeStart(this);

        // Instantiate script instances and sets field instances
        auto& scripts = GetAllEntitiesWith<IDComponent, ScriptComponent>();
        for (auto& e : scripts)
        {
            Entity entity = { e, this };
            Script::CreateScriptInstance(entity);
        }

        // Call OnCreate function in each script
        for (auto e : scripts)
        {
            Entity entity = { e, this };
            Script::InvokeEntityOnCreate(entity);
        }

        // Only play audio when listener is present (obviously)
        Entity audioListener = FindPrimaryAudioListenerEntity();
        if (audioListener)
        {
            // Send event to every audio source that scene has started
            auto& audioSourcesView = GetAllEntitiesWith<IDComponent, AudioSourceComponent>();
            for (auto& e : audioSourcesView)
            {
                auto& [id, audioSource] = audioSourcesView.get<IDComponent, AudioSourceComponent>(e);

                if (audioSource.PlayOnAwake)
                {
                    Audio::Play(id.ID, audioSource.Loop);
                }
            }
        }
    }

    void Scene::OnRuntimeStop()
    {
        Script::OnRuntimeStop();
        Audio::OnRuntimeStop();

        m_Running = false;
    }

    void Scene::OnEditorUpdate(Ref<SceneDrawList> drawList, const Camera& editorCamera, FTime ts)
    {
        // Post-Update for physics actors creation, ...
        for (auto& func : m_PostUpdateFuncs)
            func();

        m_PostUpdateFuncs.clear();

        // Render

        SceneRendererData rendererData = {};
        rendererData.ViewProjectionMatrix = editorCamera.GetViewProjection();
        rendererData.InversedViewMatrix = glm::inverse(editorCamera.GetViewMatrix());
        drawList->SetSceneData(rendererData);

        // Test cube rendering
        {
            static f32 s_Time = 0.0f;

            TransformComponent transformComponent;
            //transformComponent.Rotation.z = glm::radians(180.0f);
            //transformComponent.Rotation.y = glm::radians(-90.0f);
            //transformComponent.Rotation.x = glm::radians(90.0f);
            //transformComponent.Scale *= 1.5f;
            TransformComponent transformComponent2;
            TransformComponent transformComponent5;
            //transformComponent2.Scale *= 0.2f;

            // Light
            transformComponent2.Translation.x = 4.0f * glm::sin(s_Time += ts * 0.2f);
            transformComponent2.Translation.z = 4.0f * glm::cos(s_Time += ts * 0.2f);

            transformComponent5.Translation.x = transformComponent2.Translation.x;
            transformComponent5.Translation.y = -transformComponent2.Translation.z;
            //transformComponent2.Translation.y = 2.0f * glm::sin(s_Time += ts * 0.2f);

            f32 radius = 10.0f;
            f32 fallOff = 0.1f;
            f32 intensity = 7.0f;

            static Ref<StaticMesh> s_TestMesh;
            static Ref<StaticMesh> s_Backpack;

            if (!s_TestMesh)
            {
                //s_TestMesh = Ref<StaticMesh>::Create("Assets/Meshes/Cube.fbx");
            }

            if (!s_Backpack)
            {
                //s_Backpack = Ref<StaticMesh>::Create("Assets/Meshes/Backpack/Backpack.fbx");
            }

            TransformComponent transformComponent4;

            transformComponent4.Translation.y = 5;

            //drawList->AddStaticMesh(s_Backpack, transformComponent.GetTransform(), 3);
           /* drawList->AddStaticMesh(s_Backpack, transformComponent4.GetTransform(), 3);

            // Cube
            {
                drawList->AddStaticMesh(s_TestMesh, transformComponent2.GetTransform(), 3);
                drawList->AddStaticMesh(s_TestMesh, transformComponent5.GetTransform(), 3);
            }*/

            // Render another mesh
            TransformComponent transformComponent3;
            transformComponent3.Translation.x = 2;
            transformComponent3.Translation.y = 2;
            //drawList->AddStaticMesh(s_TestMesh2, transformComponent3.GetTransform(), 3);
            transformComponent3.Translation.x = -2;
            //drawList->AddStaticMesh(s_TestMesh2, transformComponent3.GetTransform(), 3);
            drawList->AddPointLight(transformComponent2.Translation, intensity, radius, fallOff);
            drawList->AddPointLight(transformComponent5.Translation, intensity, radius, fallOff);

            // Other duplicate
            //drawList->AddStaticMesh(s_TestMesh, transformComponent2.GetTransform(), 3);
        }

      /*  // Static meshes
        {
            auto& view = GetAllEntitiesWith<TransformComponent, StaticMeshRendererComponent>();
            for (auto entity : view)
            {
                auto& [transform, src] = view.get<TransformComponent, StaticMeshRendererComponent>(entity);
                auto mesh = AssetManager::GetAsset<Texture2D>(src.Texture);
                drawList->AddStaticMesh(transform.GetTransform(), src.Color, texture, src.TextureCoords, src.Tiling, (i32)entity);
            }
        }*/

        // 2D Rendering
        {
            // Quads
            {
                auto& view = GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();

                for (auto entity : view)
                {
                    auto& [transform, src] = view.get<TransformComponent, SpriteRendererComponent>(entity);
                    auto texture = AssetManager::GetAsset<Texture2D>(src.Texture);
                    drawList->AddSprite(transform.GetTransform(), src.Color, texture, src.TextureCoords, src.Tiling, (i32)entity);
                }
            }

            // Circles
            {
                auto& view = GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();

                for (auto entity : view)
                {
                    auto& [transform, crc] = view.get<TransformComponent, CircleRendererComponent>(entity);
                    drawList->AddCircle(transform.GetTransform(), crc.Color, crc.Thickness, crc.Fade, (i32)entity);
                }
            }

            // Text
            {
                auto& view = GetAllEntitiesWith<TransformComponent, TextComponent>();

                for (auto entity : view)
                {
                    auto& [transform, tc] = view.get<TransformComponent, TextComponent>(entity);
                    drawList->AddString(transform.GetTransform(), tc.Color, tc.FontAsset, tc.Text, tc.KerningOffset, tc.LineSpacing);
                }
            }

            // Lines
            {
                auto& view = GetAllEntitiesWith<TransformComponent, LineRendererComponent>();

                for (auto entity : view)
                {
                    auto& [transform, lrc] = view.get<TransformComponent, LineRendererComponent>(entity);
                    drawList->AddLine(lrc.Position0, lrc.Position1, lrc.Color, (i32)entity);
                }
            }
        }
    }

    void Scene::OnRuntimeUpdate(Ref<SceneDrawList> drawList, FTime ts)
    {
        Script::OnNewFrame(ts);

        // Update 2D Physics
        {
            auto& world = m_Registry.get<PhysicsWorld2DComponent>(m_SceneEntity).World;
            world->Step(ts);

            // Retrieve transform from Box2D and copy settings to it
            auto& view = GetAllEntitiesWith<Rigidbody2DComponent>();
            for (auto e : view)
            {
                Entity entity = { e, this };
                world->RetrieveTransform(entity);
            }
        }

        // Find primary audio listener
        Entity audioListenerEntity = FindPrimaryAudioListenerEntity();

        // Audio listener exists
        if (audioListenerEntity)
        {
            auto& transform = audioListenerEntity.Transform();

            // Calculate velocity from last position
            glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
            {
                static glm::vec3 s_LastTranslation = {}; // FIXME: Not ideal
                velocity = (transform.Translation - s_LastTranslation) / ts.s();
                s_LastTranslation = transform.Translation;

                if (audioListenerEntity.HasComponent<Rigidbody2DComponent>())
                {
                    auto& rb2d = audioListenerEntity.GetComponent<Rigidbody2DComponent>();
                    b2Body* body = (b2Body*)rb2d.RuntimeBody;
                    b2Vec2 b2Vel = body->GetLinearVelocity();
                    velocity = { b2Vel.x, b2Vel.y, 0.0f };
                }
            }

            // Updates audio listener positions for 3D spacial calculation
            Audio::UpdateAudioListener(transform.Translation, transform.Rotation, velocity);

            // Audio sources
            auto& view = GetAllEntitiesWith<TransformComponent, AudioSourceComponent>();
            for (auto e : view)
            {
                Entity entity = { e, this };

                auto& [id, transform, audioSourceComponent] = entity.GetComponents<IDComponent, TransformComponent, AudioSourceComponent>();

                if (audioSourceComponent.AudioPath.empty())
                    continue;

                // Update volume/gain for each source
                Audio::SetGain(id.ID, audioSourceComponent.Gain);

                // If audio is not spatial, skip
                if (!audioSourceComponent.Spatial)
                    continue;

                glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
                if (entity.HasComponent<Rigidbody2DComponent>())
                {
                    auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
                    b2Body* body = (b2Body*)rb2d.RuntimeBody;
                    const auto& linearVelocity = body->GetLinearVelocity();

                    // 2D velocity
                    velocity = { linearVelocity.x, linearVelocity.y, 0.0f };
                }

                Audio::CalculateSpatial(id.ID, transform.Translation, transform.Rotation, velocity);
            }
        }

        // Call OnUpdate function in each script 
        auto& view = GetAllEntitiesWith<ScriptComponent>();
        for (auto e : view)
        {
            Entity entity = { e, this };
            Script::InvokeEntityOnUpdate(entity);
        }

        // Post-Update for physics actors creation, ...
        {
            //Debug::ScopeTimer timer("Post update funcs");
            for (auto& func : m_PostUpdateFuncs)
                func();

            m_PostUpdateFuncs.clear();
        }

        // Render
        {
            Entity cameraEntity = FindPrimaryCameraEntity();

            if (!cameraEntity)
                return;

            SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
            glm::mat4 inversedCameraTransform = cameraEntity.Transform().GetTransform();
            camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
            camera.SetViewMatrix(glm::inverse(cameraEntity.Transform().GetTransform()));

            SceneRendererData rendererData = {};
            rendererData.ViewProjectionMatrix = camera.GetViewProjection();
            rendererData.InversedViewMatrix = inversedCameraTransform;
            drawList->SetSceneData(rendererData);

            // Static meshes
            {

            }

            // 2D Rendering
            {
                // Sprites
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();

                    for (auto entity : view)
                    {
                        auto& [transform, src] = view.get<TransformComponent, SpriteRendererComponent>(entity);
                        auto texture = AssetManager::GetAsset<Texture2D>(src.Texture);
                        drawList->AddSprite(transform.GetTransform(), src.Color, texture, src.TextureCoords, src.Tiling, (i32)entity);
                    }
                }

                // Circles
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();

                    for (auto entity : view)
                    {
                        auto& [transform, crc] = view.get<TransformComponent, CircleRendererComponent>(entity);
                        drawList->AddCircle(transform.GetTransform(), crc.Color, crc.Thickness, crc.Fade, (i32)entity);
                    }
                }

                // Text
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, TextComponent>();

                    for (auto entity : view)
                    {
                        auto& [transform, tc] = view.get<TransformComponent, TextComponent>(entity);
                        drawList->AddString(transform.GetTransform(), tc.Color, tc.FontAsset, tc.Text, tc.KerningOffset, tc.LineSpacing);
                    }
                }

                // Lines
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, LineRendererComponent>();

                    for (auto entity : view)
                    {
                        auto& [transform, lrc] = view.get<TransformComponent, LineRendererComponent>(entity);
                        drawList->AddLine(lrc.Position0, lrc.Position1, lrc.Color, (i32)entity);
                    }
                }
            }
        }
    }

    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
        Ref<Scene> newScene = Ref<Scene>::Create();

        newScene->m_ViewportWidth = other->m_ViewportWidth;
        newScene->m_ViewportHeight = other->m_ViewportHeight;
        newScene->m_ViewportX = other->m_ViewportX;
        newScene->m_ViewportY = other->m_ViewportY;

        auto& view = other->m_Registry.view<IDComponent>();

        for (auto& it = view.rbegin(); it != view.rend(); ++it)
        {
            const auto& id = other->m_Registry.get<IDComponent>(*it).ID;
            const auto& name = other->m_Registry.get<TagComponent>(*it).Tag;
            newScene->m_EntityIDMap[id] = (entt::entity)newScene->CreateEntityWithUUID(id, name);
            newScene->m_UUIDMap[*it] = id;
        }

        // Copy components (except IDComponent and TagComponent)
        Utils::CopyComponents(AllComponents{}, newScene->m_Registry, other->m_Registry, newScene->m_EntityIDMap);

        return newScene;
    }

    Entity Scene::CreateEntity(const std::string& tag)
    {
        return CreateEntityWithUUID(UUID(), tag);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& tag)
    {
        //TBO_ENGINE_ASSERT(m_Statistics.CurrentEntities != 150);

        Entity entity = { m_Registry.create(), this };
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<RelationshipComponent>();

        auto& tagComponent = entity.AddComponent<TagComponent>();
        tagComponent.Tag = tag.empty() ? "Entity" : tag;

        m_EntityIDMap[uuid] = (entt::entity)entity;
        m_UUIDMap[(entt::entity)entity] = uuid;

        m_Statistics.CurrentEntities++;
        m_Statistics.MaxEntities = (u32)m_Registry.size();

        return entity;
    }

    void Scene::DestroyEntity(Entity entity, bool excludeChildren, bool first)
    {
        if (!m_Registry.valid(entity))
        {
            // FIXME: For some reason we are destroying entities that are no longer valid, this is weeeeeeeeeeeeeeeeeeird
            return;
        }

        if (!excludeChildren)
        {
            for (size_t i = 0; i < entity.GetChildren().size(); ++i)
            {
                UUID childUUID = entity.GetChildren()[i];
                TBO_ENGINE_ASSERT(m_EntityIDMap.find(childUUID) != m_EntityIDMap.end());
                Entity child = { m_EntityIDMap.at(childUUID), this };
                DestroyEntity(child, excludeChildren, false);
            }
        }

        if (first)
        {
            if (Entity parent = entity.GetParent(); parent)
            {
                parent.RemoveChild(entity);
            }
        }

        m_EntityIDMap.erase(entity.GetUUID());
        m_Registry.destroy(entity.m_Handle);
        m_UUIDMap.erase(entity);

        m_Statistics.CurrentEntities--;
    }

    Entity Scene::DuplicateEntity(Entity entity)
    {
        Entity duplicated = CreateEntity(entity.GetName());
        CopyEntity(entity, duplicated);
        return duplicated;
    }

    void Scene::CopyEntity(Entity src, Entity dst)
    {
        // Copy components
        Utils::CopyComponentIfExists(AllComponents{}, dst, src);

        // Signal entity's parent that an this entity has been duplicated
        Entity parent = src.GetParent();
        if (parent)
            parent.GetChildren().push_back(dst.GetUUID());
    }

    void Scene::SetViewportOffset(i32 x, i32 y)
    {
        m_ViewportX = x;
        m_ViewportY = y;
    }

    void Scene::SetViewportSize(u32 width, u32 height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        // Resize our non-FixedAspectRatio cameras
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto& cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.FixedAspectRatio)
                cameraComponent.Camera.SetViewportSize(width, height);
        }
    }

    Entity Scene::FindPrimaryCameraEntity()
    {
        // Find entity with camera component
        auto& cameraComponentView = GetAllEntitiesWith<CameraComponent>();
        for (auto entity : cameraComponentView)
        {
            auto& camera = cameraComponentView.get<CameraComponent>(entity);

            if (camera.IsPrimary)
            {
                // First primary camera wins
                m_PrimaryCameraEntity = entity;
                return { m_PrimaryCameraEntity, this };
            }
        }
        return {};
    }

    Entity Scene::FindPrimaryAudioListenerEntity()
    {
        auto& view = GetAllEntitiesWith<AudioListenerComponent>();
        for (auto entity : view)
        {
            auto& audioListenerComponent = view.get<AudioListenerComponent>(entity);

            if (audioListenerComponent.IsPrimary)
            {
                m_PrimaryAudioListenerEntity = entity;

                // First audio listener wins
                return { entity, this };
            }
        }
        return {};
    }

    PhysicsWorld2D* Scene::GetPhysicsWorld2D()
    {
        return m_Registry.get<PhysicsWorld2DComponent>(m_SceneEntity).World.get();
    }

    Entity Scene::FindEntityByUUID(UUID uuid)
    {
        auto& it = m_EntityIDMap.find(uuid);

        if (it != m_EntityIDMap.end())
            return Entity{ m_EntityIDMap.at(uuid), this };

        return Entity{};
    }

    UUID Scene::FindUUIDByEntity(entt::entity entity)
    {
        auto& it = m_UUIDMap.find(entity);

        if (it != m_UUIDMap.end())
            return m_UUIDMap.at(entity);

        return 0;
    }

    Entity Scene::FindEntityByName(const std::string& name)
    {
        auto& view = GetAllEntitiesWith<TagComponent>();
        for (auto e : view)
        {
            auto& tag = m_Registry.get<TagComponent>(e).Tag;

            if (tag == name)
                return Entity{ e, this };
        }

        return Entity{};
    }

    // ---- EnTT callbacks ----

    void Scene::OnScriptComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        if (!m_Running)
            return;

        Entity scriptEntity = { entity, this };
        Script::CreateScriptInstance(scriptEntity);
    }

    void Scene::OnScriptComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        if (!m_Running)
            return;

        Entity scriptEntity = { entity, this };
        Script::DestroyScriptInstance(scriptEntity);
    }

    void Scene::OnAudioSourceComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        auto& audioSource = registry.get<AudioSourceComponent>(entity);
        auto& uuid = registry.get<IDComponent>(entity).ID;

        Audio::Register(uuid, audioSource.AudioPath);

        if (!m_Running)
            return;

        if (audioSource.PlayOnAwake)
        {
            Audio::Play(uuid, audioSource.Loop);
        }
    }

    void Scene::OnAudioSourceComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        auto& audioSource = registry.get<AudioSourceComponent>(entity);
        UUID uuid = FindUUIDByEntity(entity);
        Audio::UnRegister(uuid);
    }

    void Scene::OnRigidBody2DComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        if (!m_Running)
            return;

        auto& physicsWorld2d = m_Registry.get<PhysicsWorld2DComponent>(m_SceneEntity).World;

        Entity e = { entity, this };
        physicsWorld2d->ConstructBody(e);
    }

    // On replacing component
    void Scene::OnBoxCollider2DComponentUpdate(entt::registry& registry, entt::entity entity)
    {
        if (!m_Running)
            return;

        // Destroys original box collider
        OnBoxCollider2DComponentDestroy(registry, entity);

        // Create new one
        OnBoxCollider2DComponentConstruct(registry, entity);
    }

    void Scene::OnRigidBody2DComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        if (!m_Running)
            return;

        auto& physicsWorld2d = registry.get<PhysicsWorld2DComponent>(m_SceneEntity).World;

        Entity e = { entity, this };
        physicsWorld2d->DestroyPhysicsBody(e);
    }

    void Scene::OnBoxCollider2DComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        if (!m_Running || !registry.all_of<Rigidbody2DComponent>(entity))
            return;

        auto& physicsWorld2d = registry.get<PhysicsWorld2DComponent>(m_SceneEntity).World;

        Entity e = { entity, this };
        physicsWorld2d->ConstructBoxCollider(e);
    }

    void Scene::OnBoxCollider2DComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        if (!m_Running || !registry.all_of<Rigidbody2DComponent>(entity))
            return;

        auto& physicsWorld2d = registry.get<PhysicsWorld2DComponent>(m_SceneEntity).World;

        Entity e = { entity, this };
        physicsWorld2d->DestroyBoxCollider(e);
    }

    void Scene::OnCircleCollider2DComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        if (!m_Running || !registry.all_of<Rigidbody2DComponent>(entity))
            return;

        auto& physicsWorld2d = registry.get<PhysicsWorld2DComponent>(m_SceneEntity).World;

        Entity e = { entity, this };
        physicsWorld2d->ConstructCircleCollider(e);
    }

    void Scene::OnCircleCollider2DComponentUpdate(entt::registry& registry, entt::entity entity)
    {
        if (!m_Running)
            return;

        // Destroys original box collider
        OnCircleCollider2DComponentDestroy(registry, entity);

        // Create new one
        OnCircleCollider2DComponentConstruct(registry, entity);
    }

    void Scene::OnCircleCollider2DComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        if (!m_Running || !registry.all_of<Rigidbody2DComponent>(entity))
            return;

        auto& physicsWorld2d = registry.get<PhysicsWorld2DComponent>(m_SceneEntity).World;

        Entity e = { entity, this };
        physicsWorld2d->DestroyCircleCollilder(e);
    }
}
