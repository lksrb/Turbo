#include "tbopch.h"
#include "Scene.h"

#include "Entity.h"
#include "SceneRenderer.h"
#include "SceneCamera.h"

#include "Turbo/Audio/Audio.h"
#include "Turbo/Core/KeyCodes.h"
#include "Turbo/Script/Script.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_shape.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_fixture.h>

namespace Turbo
{
    namespace Utils
    {
        static b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType)
        {
            switch (bodyType)
            {
                case Rigidbody2DComponent::BodyType::Static:    return b2_staticBody;
                case Rigidbody2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
                case Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
            }

            TBO_ENGINE_ASSERT(false, "Unknown body type");
            return b2_staticBody;
        }

        template<typename... Components>
        static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap, const Ref<Scene>& scene)
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
        static void CopyComponents(ComponentGroup<Components...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap, const Ref<Scene>& scene)
        {
            CopyComponent<Components...>(dst, src, enttMap, scene);
        }

    }

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
        delete m_PhysicsWorld;
    }

    void Scene::OnEditorUpdate(FTime ts)
    {
    }

    void Scene::OnEditorRender(Ref<SceneRenderer> renderer, const Camera& editorCamera)
    {
        renderer->BeginRender();

        // Render entites

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
        CreatePhysicsWorld2D();

        Audio::OnRuntimeStart(this);

        Script::OnRuntimeStart(this);

        // Call OnStart function in each script
        auto& scripts = GetAllEntitiesWith<ScriptComponent>();
        for (auto& e : scripts)
        {
            Entity entity = { e, this };
            Script::InvokeEntityOnStart(entity);
        }

        // Process each audio source component

        m_Running = true;
    }

    void Scene::OnRuntimeStop()
    {
        Script::OnRuntimeStop();

        delete m_PhysicsWorld;
        m_PhysicsWorld = nullptr;

        m_Running = false;
    }

    void Scene::OnRuntimeUpdate(FTime ts)
    {
        constexpr i32 velocityIterations = 6;
        constexpr i32 positionIterations = 2;
        m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

        // Retrieve transform from Box2D
        auto rigidbodiesView = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : rigidbodiesView)
        {
            Entity entity = { e, this };
            auto& transform = entity.Transform();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            const auto& position = body->GetPosition();
            transform.Translation.x = position.x;
            transform.Translation.y = position.y;

            if (rb2d.FixedRotation == false)
                transform.Rotation.z = body->GetAngle();
        }


        // Find primary audio listener
        Entity audioListenerEntity;
        {
            auto& view = GetAllEntitiesWith<AudioListenerComponent>();
            for (auto entity : view)
            {
                auto& audioListenerComponent = view.get<AudioListenerComponent>(entity);

                if (audioListenerComponent.Primary)
                {
                    audioListenerEntity = { entity, this };
                    m_PrimaryAudioListenerEntity = entity;
                    break; // First audio listener wins
                }
            }
        }

        // Audio listener exists
        if (audioListenerEntity)
        {
            // Updates audio listener positions for 3D spacial calculation

            auto& transform = audioListenerEntity.Transform();
            Audio::UpdateAudioListener(transform.Translation, transform.Rotation, transform.Scale);

            auto& view = GetAllEntitiesWith<TransformComponent, AudioSourceComponent>();

            for (auto e : view)
            {
                Entity entity = { e, this };

                auto& [transform, audioSourceComponent] = entity.GetComponents<TransformComponent, AudioSourceComponent>();
                if (audioSourceComponent.Spatial)
                {
                    glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
                    if (entity.HasComponent<Rigidbody2DComponent>())
                    {
                        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
                        b2Body* body = (b2Body*)rb2d.RuntimeBody;
                        const auto& linearVelocity = body->GetLinearVelocity();

                        // 2D velocity
                        velocity = { linearVelocity.x, linearVelocity.x, 0.0f };
                    }

                    Audio::CalculateSpatial(audioSourceComponent.Clip, transform.Translation, transform.Rotation, velocity);
                }
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

        // Find entity with camera component
        Entity cameraEntity;
        auto& cameraComponentView = GetAllEntitiesWith<CameraComponent>();
        for (auto entity : cameraComponentView)
        {
            auto& camera = cameraComponentView.get<CameraComponent>(entity);

            if (camera.Primary)
            {
                cameraEntity = { entity, this };
                m_PrimaryCameraEntity = entity;
                break; // First primary camera wins
            }
        }

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

                        auto& tag = m_Registry.get<TagComponent>(entity).Tag;
                        if (tag == "Sprite")
                        {
                            glm::mat4 rotation = glm::toMat4(glm::quat(transform.Rotation));

                            glm::vec3 orientFront;
                            orientFront.x = cos(transform.Rotation.x) * sin(transform.Rotation.y);
                            orientFront.y = -sin(transform.Rotation.x);
                            orientFront.z = cos(transform.Rotation.x) * cos(transform.Rotation.y);

                            // TODO: Orientation

                            TBO_ENGINE_TRACE(orientFront);
                        }

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

                        glm::vec3 scale = transform.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

                        glm::mat4 offsetTransform = glm::translate(glm::mat4(1.0f), transform.Translation)
                            * glm::rotate(glm::mat4(1.0f), transform.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
                            * glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.Offset, 0.001f))
                            * glm::scale(glm::mat4(1.0f), scale);

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

    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
        Ref<Scene> newScene = Ref<Scene>::Create();

        newScene->m_ViewportWidth = other->m_ViewportWidth;
        newScene->m_ViewportHeight = other->m_ViewportHeight;

        std::unordered_map<UUID, entt::entity> entityMap;

        auto& view = other->m_Registry.view<IDComponent>();

        for (auto& it = view.rbegin(); it != view.rend(); ++it)
        {
            UUID uuid = other->m_Registry.get<IDComponent>(*it).ID;

            //TBO_CORE_ASSERT(entityMap.find(uuid) != entityMap.end());

            const auto& name = other->m_Registry.get<TagComponent>(*it).Tag;

            entityMap[uuid] = (entt::entity)newScene->CreateEntityWithUUID(uuid, name);
        }

        // Copy components (except IDComponent and TagComponent)
        Utils::CopyComponents(AllComponents{}, newScene->m_Registry, other->m_Registry, entityMap, newScene);

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

        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
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

    void Scene::CreatePhysicsWorld2D()
    {
        m_PhysicsWorld = new b2World({ 0.0f, -9.8f });

        // View all physics actors
        auto& view = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity = { e, this };
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

            // Copy position from transform component
            b2BodyDef bodyDef;
            bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type);
            bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
            bodyDef.angle = transform.Rotation.z;

            // Create body
            b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
            body->SetFixedRotation(rb2d.FixedRotation);
            body->SetEnabled(rb2d.Enabled);
            if (!rb2d.Gravity)
                body->SetGravityScale(0.0f);
            rb2d.RuntimeBody = body;

            if (entity.HasComponent<BoxCollider2DComponent>())
            {
                auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

                b2PolygonShape boxShape;
                boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y, b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);
                b2FixtureDef fixtureDef;
                fixtureDef.shape = &boxShape;
                fixtureDef.density = bc2d.Density;
                fixtureDef.friction = bc2d.Friction;
                fixtureDef.restitution = bc2d.Restitution;
                fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
                fixtureDef.isSensor = bc2d.IsSensor;
                //fixtureDef.filter.categoryBits = bc2d.CategoryBits; // <- Is that category
                //fixtureDef.filter.maskBits = bc2d.MaskBits;		// <- Collides with other categories
                body->CreateFixture(&fixtureDef);
            }
            else if (entity.HasComponent<CircleCollider2DComponent>())
            {
                auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

                b2CircleShape circleShape;
                circleShape.m_radius = transform.Scale.x * cc2d.Radius;
                circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &circleShape;
                fixtureDef.density = cc2d.Density;
                fixtureDef.friction = cc2d.Friction;
                fixtureDef.restitution = cc2d.Restitution;
                fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
                fixtureDef.isSensor = cc2d.IsSensor;
                //fixtureDef.filter.categoryBits = cc2d.CategoryBits; // <- Is that category
                //fixtureDef.filter.maskBits = cc2d.MaskBits;		// <- Collides with other categories
                body->CreateFixture(&fixtureDef);
            }

        }
    }

    Entity Scene::FindEntityByUUID(UUID uuid)
    {
        auto& view = GetAllEntitiesWith<IDComponent>(); // TODO: Maybe not necessary
        for (auto e : view)
        {
            auto& id = m_Registry.get<IDComponent>(e).ID;

            if (id == uuid)
                return Entity{ e, this };
        }

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
