#include "tbopch.h"
#include "Scene.h"

#include "Entity.h"
#include "SceneCamera.h"

#include "Turbo/Audio/Audio.h"
#include "Turbo/Asset/AssetManager.h"
#include "Turbo/Debug/ScopeTimer.h"
#include "Turbo/Script/Script.h"
#include "Turbo/Renderer/Mesh.h"
#include "Turbo/Renderer/MaterialAsset.h"
#include "Turbo/Renderer/SceneDrawList.h"

#include "Turbo/Physics/PhysicsWorld.h"
#include "Turbo/Physics/PhysicsWorld2D.h"

namespace Turbo {

    namespace Utils {

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
        Ref<PhysicsWorld2D> World;
    };

    struct PhysicsWorldComponent
    {
        Ref<PhysicsWorld> World;
    };

    Scene::Scene(bool isEditorScene, bool initialized, u64 capacity)
        : m_IsEditorScene(isEditorScene)
    {
        m_SceneEntity = m_Registry.create();
        m_Registry.emplace<SceneComponent>(m_SceneEntity, m_SceneID);

        if (!initialized)
            return;

        m_Registry.reserve(capacity);

        m_Registry.on_construct<AudioSourceComponent>().connect<&Scene::OnAudioSourceComponentConstruct>(this);
        m_Registry.on_destroy<AudioSourceComponent>().connect<&Scene::OnAudioSourceComponentDestroy>(this);
        m_Registry.on_construct<ScriptComponent>().connect<&Scene::OnScriptComponentConstruct>(this);
        m_Registry.on_destroy<ScriptComponent>().connect<&Scene::OnScriptComponentDestroy>(this);
    }

    Scene::~Scene()
    {
        m_Registry.clear();

        m_Registry.on_construct<ScriptComponent>().disconnect(this);
        m_Registry.on_destroy<ScriptComponent>().disconnect(this);

        m_Registry.on_construct<AudioSourceComponent>().disconnect(this);
        m_Registry.on_destroy<AudioSourceComponent>().disconnect(this);
    }

    void Scene::OnRuntimeStart()
    {
        TBO_PROFILE_FUNC();

        m_Running = true;

        // Find primary camera
        m_PrimaryCameraEntity = FindPrimaryCameraEntity();

        // NOTE: Physics 2D and 3D are separate, grouping them does not provide any benefit
        // Physics 2D
        auto& physicsWorld2d = m_Registry.emplace<PhysicsWorld2DComponent>(m_SceneEntity, Ref<PhysicsWorld2D>::Create(this)).World;
        physicsWorld2d->OnRuntimeStart();

        // Physics 3D
        auto& physicsWorld3d = m_Registry.emplace<PhysicsWorldComponent>(m_SceneEntity, Ref<PhysicsWorld>::Create(this)).World;
        physicsWorld3d->OnRuntimeStart();

        Audio::OnRuntimeStart(this);
        Script::OnRuntimeStart(this);

        // Instantiate script instances and sets field instances

        auto scripts = m_Registry.view<ScriptComponent>();
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
            auto audioSources = m_Registry.group<AudioSourceComponent>(entt::get<IDComponent>);
            for (auto& e : audioSources)
            {
                const auto& [id, audioSource] = audioSources.get<IDComponent, AudioSourceComponent>(e);

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

        GetPhysicsWorld()->OnRuntimeStop();
        GetPhysicsWorld2D()->OnRuntimeStop();

        m_Running = false;
    }

    void Scene::OnEditorUpdate(Ref<SceneDrawList> drawList, const Camera& editorCamera, FTime ts)
    {
        TBO_PROFILE_FUNC();

        // Post-Update for physics actors creation, ...
        for (auto& func : m_PostUpdateFuncs)
            func();

        m_PostUpdateFuncs.clear();

        // Render
        {
            glm::mat4 cameraView = editorCamera.GetViewMatrix();

            SceneRendererData rendererData = {};
            rendererData.ViewProjectionMatrix = editorCamera.GetViewProjection();
            rendererData.ViewMatrix = cameraView;
            rendererData.InversedViewMatrix = glm::inverse(cameraView);
            rendererData.InversedViewProjectionMatrix = glm::inverse(rendererData.ViewProjectionMatrix);
            drawList->SetSceneData(rendererData);

            RenderScene(drawList);
        }
    }

    void Scene::OnRuntimeUpdate(Ref<SceneDrawList> drawList, FTime ts)
    {
        TBO_PROFILE_FUNC();

        Script::OnNewFrame(ts);

        // Update 3D physics
        GetPhysicsWorld()->Simulate(ts);

        // Update 2D physics
        auto physicsWorld2d = GetPhysicsWorld2D();
        physicsWorld2d->Simulate(ts);

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
                    glm::vec2 vel2d = physicsWorld2d->RetrieveLinearVelocity(audioListenerEntity);
                    velocity = { vel2d.x, vel2d.y, 0.0f };
                }
            }

            // Updates audio listener positions for 3D spacial calculation
            Audio::UpdateAudioListener(transform.Translation, transform.Rotation, velocity);

            // Audio sources
            auto group = m_Registry.group<AudioSourceComponent>(entt::get<TransformComponent, IDComponent>);
            for (auto e : group)
            {
                Entity entity = { e, this };

                const auto& [transform, id, audioSourceComponent] = group.get<TransformComponent, IDComponent, AudioSourceComponent>(entity);

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
                    glm::vec2 vel2d = physicsWorld2d->RetrieveLinearVelocity(entity);
                    velocity = { vel2d.x, vel2d.y, 0.0f };
                }

                Audio::CalculateSpatial(id.ID, transform.Translation, transform.Rotation, velocity);
            }
        }

        {
            TBO_PROFILE_SCOPE("Script::OnUpdate");
            // Call OnUpdate function in each script 
            auto view = m_Registry.view<ScriptComponent>();
            for (auto e : view)
            {
                Entity entity = { e, this };
                Script::InvokeEntityOnUpdate(entity);
            }
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
            glm::mat4 cameraTransform = GetWorldSpaceTransformMatrix(cameraEntity);
            camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
            camera.SetViewMatrix(glm::inverse(cameraTransform));

            SceneRendererData rendererData = {};
            rendererData.ViewProjectionMatrix = camera.GetViewProjection();
            rendererData.InversedViewMatrix = cameraTransform;
            rendererData.ViewMatrix = glm::inverse(cameraTransform);
            rendererData.InversedViewProjectionMatrix = glm::inverse(rendererData.ViewProjectionMatrix);
            drawList->SetSceneData(rendererData);

            // Debug renderer
            for (auto& func : m_DebugRendererCallbacks)
            {
                func(drawList);
            }
            m_DebugRendererCallbacks.clear();

            RenderScene(drawList);
        }

        m_TimeSinceStart += ts;
    }

    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
        Ref<Scene> newScene = Ref<Scene>::Create();

        newScene->m_ViewportWidth = other->m_ViewportWidth;
        newScene->m_ViewportHeight = other->m_ViewportHeight;
        newScene->m_ViewportX = other->m_ViewportX;
        newScene->m_ViewportY = other->m_ViewportY;

        auto view = other->m_Registry.view<IDComponent>();

        for (auto it = view.rbegin(); it != view.rend(); ++it)
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
            TBO_ENGINE_ERROR("[Scene::DestroyEntity] Entity is not valid!");
            // FIXME: For some reason we are destroying entities that are no longer valid, this is weeeeeeeeeeeeeeeeeeird
            return;
        }

        if (entity.HasComponent<RigidbodyComponent>())
        {
            GetPhysicsWorld()->DestroyRigidbody(entity);
        }

        if (entity.HasComponent<Rigidbody2DComponent>())
        {
            GetPhysicsWorld2D()->DestroyRigidbody(entity);
        }

        if (!excludeChildren)
        {
            for (u64 i = 0; i < entity.GetChildren().size(); ++i)
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
        CopyComponents(entity, duplicated);

        // Signal entity's parent that an this entity has been duplicated
        Entity parent = entity.GetParent();
        if (parent)
            parent.GetChildren().push_back(duplicated.GetUUID());

        return duplicated;
    }

    void Scene::CopyComponents(Entity src, Entity dst)
    {
        Utils::CopyComponentIfExists(AllComponents{}, dst, src);
    }

    Entity Scene::CreatePrefabEntity(Entity prefabEntity, Entity parent, const glm::vec3* translation, const glm::vec3* rotation, const glm::vec3* scale)
    {
        TBO_ENGINE_ASSERT(prefabEntity.HasComponent<PrefabComponent>());
            
        Entity entity = CreateEntity(prefabEntity.GetName());

        // Copy components
        Utils::CopyComponentIfExists(AllComponents{}, entity, prefabEntity);
        
        // Override parent (if exists)
        if (parent && prefabEntity.HasParent())
        {
            entity.SetParentUUID(parent.GetUUID());
        }

        // Override transforms
        auto& transform = entity.Transform();
        if (translation) transform.Translation = *translation;
        if (rotation)    transform.Rotation = *rotation;
        if (scale)       transform.Scale = *scale;

        // Map prefab scene UUIDs to this scene
        auto& children = entity.GetChildren();
        for (UUID& childUUID : children)
        {
            Entity prefabChild = prefabEntity.m_Scene->FindEntityByUUID(childUUID);
            Entity newChildEntity = CreatePrefabEntity(prefabChild, entity);
            childUUID = newChildEntity.GetUUID();
        }

        // Create physics actor
        if (!m_IsEditorScene)
        {
            if (entity.HasComponent<RigidbodyComponent>())
            {
                GetPhysicsWorld()->CreateRigidbody(entity);
            }

            if (entity.HasComponent<Rigidbody2DComponent>())
            {
                GetPhysicsWorld2D()->CreateRigidbody(entity);
            }
        }

         // Duplicate script entity
        if (entity.HasComponent<ScriptComponent>())
        {
            Script::DuplicateRuntimeScriptEntity(entity, prefabEntity);
        }

        return entity;
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

    bool Scene::Contains(Entity entity) const
    {
        return m_Registry.valid(entity);
    }

    Entity Scene::FindPrimaryCameraEntity()
    {
        // Find entity with camera component
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto& camera = view.get<CameraComponent>(entity);

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
        auto group = m_Registry.view<AudioListenerComponent>();
        for (auto entity : group)
        {
            auto& audioListenerComponent = group.get<AudioListenerComponent>(entity);

            if (audioListenerComponent.IsPrimary)
            {
                m_PrimaryAudioListenerEntity = entity;

                // First audio listener wins
                return { entity, this };
            }
        }
        return {};
    }

    Ref<PhysicsWorld2D> Scene::GetPhysicsWorld2D() const
    {
        return m_Registry.get<PhysicsWorld2DComponent>(m_SceneEntity).World;
    }

    Ref<PhysicsWorld> Scene::GetPhysicsWorld() const
    {
        return  m_Registry.get<PhysicsWorldComponent>(m_SceneEntity).World;
    }

    Entity Scene::FindEntityByUUID(UUID uuid)
    {
        auto it = m_EntityIDMap.find(uuid);

        if (it != m_EntityIDMap.end())
            return Entity{ m_EntityIDMap.at(uuid), this };

        return Entity{};
    }

    UUID Scene::FindUUIDByEntity(entt::entity entity)
    {
        auto it = m_UUIDMap.find(entity);

        if (it != m_UUIDMap.end())
            return m_UUIDMap.at(entity);

        return 0;
    }

    Entity Scene::FindEntityByName(std::string_view name)
    {
        auto view = m_Registry.view<TagComponent>();
        for (auto e : view)
        {
            auto& tag = view.get<TagComponent>(e).Tag;

            if (tag == name)
                return Entity{ e, this };
        }

        return Entity{};
    }

    glm::mat4 Scene::GetWorldSpaceTransformMatrix(Entity entity)
    {
        glm::mat4 transform(1.0f);

        Entity parent = FindEntityByUUID(entity.GetParentUUID());
        if (parent)
            transform = GetWorldSpaceTransformMatrix(parent);

        return transform * entity.Transform().GetTransform();
    }

    void Scene::ConvertToLocalSpace(Entity entity)
    {
        Entity parent = FindEntityByUUID(entity.GetParentUUID());

        if (!parent)
            return;

        auto& transform = entity.Transform();
        glm::mat4 parentTransform = GetWorldSpaceTransformMatrix(parent);

        glm::mat4 localTransform = glm::inverse(parentTransform) * transform.GetTransform();
        Math::DecomposeTransform(localTransform, transform.Translation, transform.Rotation, transform.Scale);
    }

    void Scene::ConvertToWorldSpace(Entity entity)
    {
        Entity parent = FindEntityByUUID(entity.GetParentUUID());

        if (!parent)
            return;

        glm::mat4 transform = GetWorldSpaceTransformMatrix(entity);
        auto& entityTransform = entity.Transform();
        Math::DecomposeTransform(transform, entityTransform.Translation, entityTransform.Rotation, entityTransform.Scale);
    }

    // TODO: Implement WorldTransformComponent
    TransformComponent Scene::GetWorldSpaceTransform(Entity entity)
    {
        TBO_PROFILE_FUNC();

        glm::mat4 worldTransform = GetWorldSpaceTransformMatrix(entity);
        TransformComponent transform;
        transform.SetTransform(worldTransform);
        return transform;
    }

    void Scene::RenderScene(Ref<SceneDrawList> drawList)
    {
        TBO_PROFILE_FUNC();

        // Directional lights
        {
            auto view = m_Registry.view<DirectionalLightComponent>();
            for (auto entity : view)
            {
                const auto& dl = view.get<DirectionalLightComponent>(entity);
                auto transform = GetWorldSpaceTransform({ entity, this });
                glm::vec3 direction = -glm::normalize(glm::mat3(transform.GetTransform()) * glm::vec3(0.0f, 0.0f, 1.0f));
                drawList->AddDirectionalLight(direction, dl.Radiance, dl.Intensity);
            }
        }

        // Point lights
        {
            auto view = m_Registry.view<PointLightComponent>();
            for (auto entity : view)
            {
                const auto& plc = view.get<PointLightComponent>(entity);
                auto transform = GetWorldSpaceTransform({ entity, this });
                // Rotation does not matter, but i think scale will matter
                // TODO: Figure out how to composose scale of the ligth and intesity
                drawList->AddPointLight(GetWorldSpaceTransform({ entity, this }).Translation, plc.Radiance, plc.Intensity, plc.Radius, plc.FallOff);
            }
        }

        // Spotlights
        {
            auto view = m_Registry.view<SpotLightComponent>();
            for (auto entity : view)
            {
                const auto& slc = view.get<SpotLightComponent>(entity);
                auto transform = GetWorldSpaceTransform({ entity, this });
                glm::vec3 direction = -glm::normalize(glm::mat3(transform.GetTransform()) * glm::vec3(0.0f, 0.0f, 1.0f));
                drawList->AddSpotLight(transform.Translation, direction, slc.Radiance, slc.Intensity, slc.InnerCone, slc.OuterCone);
            }
        }

        // Static meshes
        {
            auto view = m_Registry.view<StaticMeshRendererComponent>();

            for (auto entity : view)
            {
                const auto& smr = view.get<StaticMeshRendererComponent>(entity);
                auto mesh = AssetManager::GetAsset<StaticMesh>(smr.Mesh);

                // Discard meshes without asset
                if (mesh == nullptr)
                    continue;

                auto material = AssetManager::GetAsset<MaterialAsset>(smr.Material);
                drawList->AddStaticMesh(mesh, material, GetWorldSpaceTransformMatrix({ entity, this }), (i32)entity);
            }
        }

        // 2D Rendering
        {
            // Sprites
            {
                auto view = m_Registry.view<SpriteRendererComponent>();

                for (auto entity : view)
                {
                    const auto& src = view.get<SpriteRendererComponent>(entity);
                    auto texture = AssetManager::GetAsset<Texture2D>(src.Texture);
                    drawList->AddSprite(GetWorldSpaceTransformMatrix({ entity, this }), src.Color, texture, src.TextureCoords, src.Tiling, (i32)entity);
                }
            }

            // Circles
            {
                auto view = m_Registry.view<CircleRendererComponent>();

                for (auto entity : view)
                {
                    const auto& crc = view.get<CircleRendererComponent>(entity);
                    drawList->AddCircle(GetWorldSpaceTransformMatrix({ entity, this }), crc.Color, crc.Thickness, crc.Fade, (i32)entity);
                }
            }

            // Text
            {
                auto view = m_Registry.view<TextComponent>();

                for (auto entity : view)
                {
                    const auto& tc = view.get<TextComponent>(entity);
                    drawList->AddString(GetWorldSpaceTransformMatrix({ entity, this }), tc.Color, tc.FontAsset, tc.Text, tc.KerningOffset, tc.LineSpacing);
                }
            }

            // Lines
            {
                // TODO: How should lines behave when relationship component takes effect?
                auto group = m_Registry.group<LineRendererComponent>(entt::get<TransformComponent>);

                for (auto entity : group)
                {
                    const auto& [transform, lrc] = group.get<TransformComponent, LineRendererComponent>(entity);
                    drawList->AddLine(transform.Translation, lrc.Destination, lrc.Color, (i32)entity);
                }
            }
        }
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
   
}
