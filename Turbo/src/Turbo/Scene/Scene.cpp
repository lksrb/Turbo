#include "tbopch.h"
#include "Scene.h"

#include "Entity.h"
#include "SceneRenderer.h"
#include "SceneCamera.h"

#include "Turbo/Audio/Audio.h"
#include "Turbo/Core/KeyCodes.h"
#include "Turbo/Script/Script.h"

#include "Turbo/Physics/Physics2D.h"

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

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
        delete m_PhysicsScene2D;
    }

    void Scene::OnEditorUpdate(FTime ts)
    {
    }

    void Scene::OnEditorRender(Ref<SceneRenderer> renderer, const Camera& editorCamera)
    {
        renderer->BeginRender();

        // 2D Rendering
        {
            Ref<Renderer2D> renderer2d = renderer->GetRenderer2D();
            {
                renderer2d->Begin2D(editorCamera);

                // Quads
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();

                    for (auto& entity : view)
                    {
                        auto& [transform, src] = view.get<TransformComponent, SpriteRendererComponent>(entity);

                        if (src.SubTexture)
                            renderer2d->DrawSprite(transform.GetMatrix(), src.Color, src.SubTexture, src.Tiling, (i32)entity);
                        else
                            renderer2d->DrawSprite(transform.GetMatrix(), src.Color, src.Texture, src.Tiling, (i32)entity);

                    }
                }

                // Circles
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();

                    for (auto& entity : view)
                    {
                        auto& [transform, crc] = view.get<TransformComponent, CircleRendererComponent>(entity);
                        renderer2d->DrawCircle(transform.GetMatrix(), crc.Color, crc.Thickness, crc.Fade, (i32)entity);
                    }
                }

                // Text
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, TextComponent>();

                    for (auto& entity : view)
                    {
                        auto& [transform, tc] = view.get<TransformComponent, TextComponent>(entity);
                        renderer2d->DrawString(transform.GetMatrix(), tc.Color, tc.FontAsset, tc.Text, tc.KerningOffset, tc.LineSpacing);
                    }
                }

                // Debug lines
                if (ShowPhysics2DColliders)
                {
                    // Box collider
                    auto& bview = GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
                    for (auto& entity : bview)
                    {
                        auto& [transform, bc2d] = bview.get<TransformComponent, BoxCollider2DComponent>(entity);

                        const glm::vec3& translation = transform.Translation + glm::vec3(bc2d.Offset, 0.0f);
                        const glm::vec3& scale = transform.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

                        const glm::mat4& offsetTransform =
                            glm::translate(glm::mat4(1.0f), translation) *
                            glm::rotate(glm::mat4(1.0f), transform.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
                            glm::scale(glm::mat4(1.0f), scale);

                        renderer2d->DrawRect(offsetTransform, { 0.0f, 1.0f, 0.0f, 1.0f }, (i32)entity);
                    }

                    // Circle collider
                    auto& cview = GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
                    for (auto& entity : cview)
                    {
                        auto& [transform, cc2d] = cview.get<TransformComponent, CircleCollider2DComponent>(entity);

                        const glm::vec3& translation = transform.Translation + glm::vec3(cc2d.Offset, 0.0f);
                        const glm::vec3& scale = transform.Scale * glm::vec3(cc2d.Radius * 2.0f);

                        const glm::mat4& offsetTransform =
                            glm::translate(glm::mat4(1.0f), translation) *
                            glm::scale(glm::mat4(1.0f), scale);

                        renderer2d->DrawCircle(offsetTransform, { 0.0f, 1.0f, 0.0f, 1.0f }, 0.035, 0.005f, (i32)entity);
                    }
                }

                renderer2d->End2D();
            }
        }

        renderer->EndRender();
    }

    void Scene::OnRuntimeStart()
    {
        m_PhysicsScene2D = new PhysicsScene2D(this);

        Audio::OnRuntimeStart(this);
        Script::OnRuntimeStart(this);

        // Call OnStart function in each script
        auto& scripts = GetAllEntitiesWith<ScriptComponent>();
        for (auto& e : scripts)
        {
            Entity entity = { e, this };
            Script::InvokeEntityOnStart(entity);
        }

        // Only play audio when listener is present (obviously)
        Entity audioListener = FindPrimaryAudioListenerEntity();
        if (audioListener)
        {
            // Send event to every audio source that scene has started
            auto& audioSourcesView = GetAllEntitiesWith<AudioSourceComponent>();
            for (auto& e : audioSourcesView)
            {
                auto& audioSource = audioSourcesView.get<AudioSourceComponent>(e);

                const auto& audioClip = audioSource.Clip;

                if (audioClip && audioSource.PlayOnStart)
                {
                    Audio::Play(audioClip, audioSource.Loop);
                }
            }
        }
        m_Running = true;
    }

    void Scene::OnRuntimeStop()
    {
        Script::OnRuntimeStop();
        Audio::OnRuntimeStop();

        delete m_PhysicsScene2D;
        m_PhysicsScene2D = nullptr;

        m_Running = false;
    }

    void Scene::OnRuntimeUpdate(FTime ts)
    {
        // Update 2D Physics
        m_PhysicsScene2D->OnUpdate(ts);

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

                auto& [transform, audioSourceComponent] = entity.GetComponents<TransformComponent, AudioSourceComponent>();
                if (!audioSourceComponent.Clip)
                    continue;

                // Update volume/gain for each source
                Audio::SetGain(audioSourceComponent.Clip, audioSourceComponent.Gain);

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

                Audio::CalculateSpatial(audioSourceComponent.Clip, transform.Translation, transform.Rotation, velocity);
            }
        }

        // Call OnUpdate function in each script 
        auto& view = GetAllEntitiesWith<ScriptComponent>();
        for (auto& e : view)
        {
            Entity entity = { e, this };
            Script::InvokeEntityOnUpdate(entity, ts);
        }
    }

    void Scene::OnRuntimeRender(Ref<SceneRenderer> renderer)
    {
        renderer->BeginRender();

        Entity cameraEntity = FindPrimaryCameraEntity();

        // 2D Rendering
        {
            Ref<Renderer2D> renderer2d = renderer->GetRenderer2D();

            // Camera does exists
            if (cameraEntity)
            {
                auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
                camera.SetViewMatrix(glm::inverse(cameraEntity.Transform().GetMatrix()));

                renderer2d->Begin2D(camera);
                // Sprites
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();

                    for (auto& entity : view)
                    {
                        auto& [transform, src] = view.get<TransformComponent, SpriteRendererComponent>(entity);

                        if (src.SubTexture)
                            renderer2d->DrawSprite(transform.GetMatrix(), src.Color, src.SubTexture, src.Tiling, (i32)entity);
                        else
                            renderer2d->DrawSprite(transform.GetMatrix(), src.Color, src.Texture, src.Tiling, (i32)entity);

                    }
                }

                // Circles
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();

                    for (auto& entity : view)
                    {
                        auto& [transform, crc] = view.get<TransformComponent, CircleRendererComponent>(entity);
                        renderer2d->DrawCircle(transform.GetMatrix(), crc.Color, crc.Thickness, crc.Fade, (i32)entity);
                    }
                }

                // Text
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, TextComponent>();

                    for (auto& entity : view)
                    {
                        auto& [transform, tc] = view.get<TransformComponent, TextComponent>(entity);
                        renderer2d->DrawString(transform.GetMatrix(), tc.Color, tc.FontAsset, tc.Text, tc.KerningOffset, tc.LineSpacing);
                    }
                }

                // Debug lines
                if (ShowPhysics2DColliders)
                {
                    f32 zOffset = camera.GetProjectionType() == Camera::ProjectionType::Perspective ? 0.001f : 0.0f; // TODO: Think about this

                    // Box collider
                    auto& bview = GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
                    for (auto& entity : bview)
                    {
                        auto& [transform, bc2d] = bview.get<TransformComponent, BoxCollider2DComponent>(entity);

                        glm::vec3 scale = transform.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

                        glm::mat4 offsetTransform = glm::translate(glm::mat4(1.0f), transform.Translation)
                            * glm::rotate(glm::mat4(1.0f), transform.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
                            * glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.Offset, zOffset))
                            * glm::scale(glm::mat4(1.0f), scale);

                        renderer2d->DrawRect(offsetTransform, { 0.0f, 1.0f, 0.0f, 1.0f }, (i32)entity);
                    }

                    // Circle collider
                    auto& cview = GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
                    for (auto& entity : cview)
                    {
                        auto& [transform, cc2d] = cview.get<TransformComponent, CircleCollider2DComponent>(entity);

                        const glm::vec3& translation = transform.Translation + glm::vec3(cc2d.Offset, zOffset);
                        const glm::vec3& scale = transform.Scale * glm::vec3(cc2d.Radius * 2.0f);

                        const glm::mat4& offsetTransform =
                            glm::translate(glm::mat4(1.0f), translation) *
                            glm::scale(glm::mat4(1.0f), scale);

                        renderer2d->DrawCircle(offsetTransform, { 0.0f, 1.0f, 0.0f, 1.0f }, 0.035, 0.005f, (i32)entity);
                    }
                }

                renderer2d->End2D();
            }
        }

        renderer->EndRender();
    }

    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
        Ref<Scene> newScene = Ref<Scene>::Create();

        newScene->m_ViewportWidth = other->m_ViewportWidth;
        newScene->m_ViewportHeight = other->m_ViewportHeight;

        auto& view = other->m_Registry.view<IDComponent>();

        for (auto& it = view.rbegin(); it != view.rend(); ++it)
        {
            const auto& id = other->m_Registry.get<IDComponent>(*it).ID;
            const auto& name = other->m_Registry.get<TagComponent>(*it).Tag;
            newScene->m_EntityIDMap[id] = (entt::entity)newScene->CreateEntityWithUUID(id, name);
        }

        // Copy components (except IDComponent and TagComponent)
        Utils::CopyComponents(AllComponents{}, newScene->m_Registry, other->m_Registry, newScene->m_EntityIDMap);

        // Copy settings
        newScene->ShowPhysics2DColliders = other->ShowPhysics2DColliders;

        return newScene;
    }

    Entity Scene::CreateEntity(const std::string& tag)
    {
        return CreateEntityWithUUID(UUID(), tag);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& tag)
    {
        Entity entity = { m_Registry.create(), this };
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();

        auto& tagComponent = entity.AddComponent<TagComponent>();
        tagComponent.Tag = tag.empty() ? "Entity" : tag;

        m_EntityIDMap[uuid] = (entt::entity)entity;

        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_EntityIDMap.erase(entity.GetUUID());
        m_Registry.destroy(entity);
    }

	Entity Scene::DuplicateEntity(Entity entity)
	{
        Entity duplicated = CreateEntity(entity.GetName());
        // Copy components
        Utils::CopyComponentIfExists(AllComponents{}, duplicated, entity);
        return duplicated;
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
                m_PrimaryCameraEntity = entity;

                // First primary camera wins
                return { entity, this };
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

    Entity Scene::FindEntityByUUID(UUID uuid)
    {
        auto& it = m_EntityIDMap.find(uuid);

        if (it != m_EntityIDMap.end())
            return Entity{ m_EntityIDMap.at(uuid), this};
        
        return Entity{};
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

    Entity Scene::GetCameraEntity()
    {
        return Entity{ m_PrimaryCameraEntity, this };
    }

    // Events ------------------------------------------------------------------------------------

    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
        static_assert(sizeof(T) == 0);
    }

    template<>
    void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
    {
        if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
            component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
    }

    template<>
    void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TextComponent>(Entity entity, TextComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<AudioSourceComponent>(Entity entity, AudioSourceComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<AudioListenerComponent>(Entity entity, AudioListenerComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
    {
    }
}
