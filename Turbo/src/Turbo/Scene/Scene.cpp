#include "tbopch.h"
#include "Scene.h"

#include "Entity.h"
#include "SceneRenderer.h"
#include "SceneCamera.h"

#include "Turbo/Audio/Audio.h"
#include "Turbo/Core/KeyCodes.h"
#include "Turbo/Script/Script.h"

#include "Turbo/Physics/Physics2D.h"

#include <box2d/b2_body.h>
#include <box2d/b2_world.h>
#include <box2d/b2_shape.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_fixture.h>

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

    class ContactListener2D : public b2ContactListener
    {
    private:
        void BeginContact(b2Contact* contact) override
        {
            Scene* context = Script::GetCurrentScene();

            b2Fixture* fixtureA = contact->GetFixtureA();
            b2Fixture* fixtureB = contact->GetFixtureB();

            Entity entityA = context->FindEntityByUUID((u64)fixtureA->GetUserData().pointer);
            Entity entityB = context->FindEntityByUUID((u64)fixtureB->GetUserData().pointer);

            // Acts like a begin contact, FIXME: Temporary? 
            if (entityA && entityA.HasComponent<ScriptComponent>())
                Script::InvokeEntityOnBeginCollision2D(entityA, entityB, fixtureA->IsSensor());

            if (entityB && entityB.HasComponent<ScriptComponent>())
                Script::InvokeEntityOnBeginCollision2D(entityB, entityA, fixtureB->IsSensor());
#if 0
            // Update physics settings
            auto& rb2dA = entityA.GetComponent<Rigidbody2DComponent>();
            auto& rb2dB = entityB.GetComponent<Rigidbody2DComponent>();
            contact->SetEnabled(rb2dA.ContactEnabled && rb2dB.ContactEnabled);
#endif
        }

        void EndContact(b2Contact* contact) override
        {
            Scene* context = Script::GetCurrentScene();

            b2Fixture* fixtureA = contact->GetFixtureA();
            b2Fixture* fixtureB = contact->GetFixtureB();

            Entity entityA = context->FindEntityByUUID((u64)fixtureA->GetUserData().pointer);
            Entity entityB = context->FindEntityByUUID((u64)fixtureB->GetUserData().pointer);

            if (entityA && entityA.HasComponent<ScriptComponent>())
                Script::InvokeEntityOnEndCollision2D(entityA, entityB, fixtureA->IsSensor());

            if (entityB && entityB.HasComponent<ScriptComponent>())
                Script::InvokeEntityOnEndCollision2D(entityB, entityA, fixtureB->IsSensor());
        }

        void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override
        {
        }

        void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override
        {
        }
    };

    static ContactListener2D s_ContactListener;

    struct SceneComponent
    {
        UUID SceneID;
    };

    struct Box2DWorldComponent
    {
        std::unique_ptr<b2World> World;
    };

    Scene::Scene()
    {
        m_SceneEntity = m_Registry.create();
        m_Registry.emplace<SceneComponent>(m_SceneEntity, m_SceneID);
    }

    Scene::~Scene()
    {
    }

    void Scene::OnEditorUpdate(FTime ts)
    {
        ClearDeletedEntities();
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

                    for (auto entity : view)
                    {
                        auto& [transform, src] = view.get<TransformComponent, SpriteRendererComponent>(entity);

                        renderer2d->DrawSprite(transform.GetMatrix(), src.Color, src.SubTexture, src.Tiling, (i32)entity);

                    }
                }

                // Circles
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();

                    for (auto entity : view)
                    {
                        auto& [transform, crc] = view.get<TransformComponent, CircleRendererComponent>(entity);
                        renderer2d->DrawCircle(transform.GetMatrix(), crc.Color, crc.Thickness, crc.Fade, (i32)entity);
                    }
                }

                // Text
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, TextComponent>();

                    for (auto entity : view)
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
                    for (auto entity : bview)
                    {
                        auto& [transform, bc2d] = bview.get<TransformComponent, BoxCollider2DComponent>(entity);

                        glm::vec3 translation = transform.Translation + glm::vec3(bc2d.Offset, 0.0f);
                        glm::vec3 scale = transform.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

                        const glm::mat4& offsetTransform =
                            glm::translate(glm::mat4(1.0f), translation) *
                            glm::rotate(glm::mat4(1.0f), transform.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
                            glm::scale(glm::mat4(1.0f), scale);

                        renderer2d->DrawRect(offsetTransform, { 0.0f, 1.0f, 0.0f, 1.0f }, (i32)entity);
                    }

                    // Circle collider
                    auto& cview = GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
                    for (auto entity : cview)
                    {
                        auto& [transform, cc2d] = cview.get<TransformComponent, CircleCollider2DComponent>(entity);

                        glm::vec3 translation = transform.Translation + glm::vec3(cc2d.Offset, 0.0f);
                        glm::vec3 scale = transform.Scale * glm::vec3(cc2d.Radius * 2.0f);

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
        auto& world = m_Registry.emplace<Box2DWorldComponent>(m_SceneEntity, std::make_unique<b2World>(b2Vec2{ 0.0f, -9.8f })).World;
        world->SetContactListener(&s_ContactListener);
        {
            auto& view = GetAllEntitiesWith<Rigidbody2DComponent>();
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
                b2Body* body = world->CreateBody(&bodyDef);
                body->SetFixedRotation(rb2d.FixedRotation);
                body->SetEnabled(rb2d.Enabled);
                body->SetGravityScale(rb2d.GravityScale);
                //body->SetBullet(rb2d.IsBullet);
                body->SetBullet(true);

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
                    b2FixtureUserData data;
                    data.pointer = (u64)entity.GetUUID();
                    fixtureDef.userData = data;
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
                    b2FixtureUserData data;
                    data.pointer = (u64)entity.GetUUID();
                    fixtureDef.userData = data;
                    //fixtureDef.filter.categoryBits = cc2d.CategoryBits; // <- Is that category
                    //fixtureDef.filter.maskBits = cc2d.MaskBits;		// <- Collides with other categories
                    body->CreateFixture(&fixtureDef);
                }
            }
        }

        Audio::OnRuntimeStart(this);
        Script::OnRuntimeStart(this);

        // Call OnCreate function in each script
        auto& scripts = GetAllEntitiesWith<ScriptComponent>();
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

        m_Running = false;
    }

    void Scene::OnRuntimeUpdate(FTime ts)
    {
        ClearDeletedEntities();

        // Update 2D Physics
        {
            auto& world = m_Registry.get<Box2DWorldComponent>(m_SceneEntity).World;

            constexpr i32 velocityIterations = 6;
            constexpr i32 positionIterations = 2;
            world->Step(ts, velocityIterations, positionIterations);

            // Retrieve transform from Box2D and copy settings to it
            auto& view = GetAllEntitiesWith<Rigidbody2DComponent>();
            for (auto e : view)
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

                // Copy settings from the component, 
                // NOTE: Settings are from last frame, not sure if its good or bad
                body->SetEnabled(rb2d.Enabled);
                body->SetType(Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type));
                body->SetGravityScale(rb2d.GravityScale);
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
        for (auto e : view)
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
                SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
                camera.SetViewMatrix(glm::inverse(cameraEntity.Transform().GetMatrix()));

                renderer2d->Begin2D(camera);
                // Sprites
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();

                    for (auto entity : view)
                    {
                        auto& [transform, src] = view.get<TransformComponent, SpriteRendererComponent>(entity);

                        renderer2d->DrawSprite(transform.GetMatrix(), src.Color, src.SubTexture, src.Tiling, (i32)entity);
                    }
                }

                // Circles
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();

                    for (auto entity : view)
                    {
                        auto& [transform, crc] = view.get<TransformComponent, CircleRendererComponent>(entity);
                        renderer2d->DrawCircle(transform.GetMatrix(), crc.Color, crc.Thickness, crc.Fade, (i32)entity);
                    }
                }

                // Text
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, TextComponent>();

                    for (auto entity : view)
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
                    for (auto entity : bview)
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
                    for (auto entity : cview)
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
        newScene->m_ViewportX = other->m_ViewportX;
        newScene->m_ViewportY = other->m_ViewportY;

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
        entity.AddComponent<RelationshipComponent>().Parent = uuid; // Itself - means its the root

        auto& tagComponent = entity.AddComponent<TagComponent>();
        tagComponent.Tag = tag.empty() ? "Entity" : tag;

        m_EntityIDMap[uuid] = (entt::entity)entity;

        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        UUID uuid = entity.GetUUID();
        TBO_ENGINE_ASSERT(m_EntityIDMap.find(uuid) != m_EntityIDMap.end());

        m_DestroyedEntities.push_back(entity);

        auto& relationship = entity.GetComponent<RelationshipComponent>();
        for (auto entityUUID : relationship.Children)
        {
            TBO_ENGINE_ASSERT(m_EntityIDMap.find(entityUUID) != m_EntityIDMap.end());

            Entity entity = { m_EntityIDMap.at(entityUUID), this };
            DestroyEntity(entity);
        }
    }

    Entity Scene::DuplicateEntity(Entity entity)
    {
        Entity duplicated = CreateEntity(entity.GetName());
        // Copy components
        Utils::CopyComponentIfExists(AllComponents{}, duplicated, entity);
        return duplicated;
    }

    void Scene::SetViewportOffset(u32 x, u32 y)
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

    void Scene::ClearDeletedEntities()
    {
        for (auto e : m_DestroyedEntities)
        {
            Entity entity = { e, this };

            if (entity.HasComponent<ScriptComponent>())
                Script::DestroyScriptInstance(entity);

            m_EntityIDMap.erase(entity.GetUUID());
            m_Registry.destroy(entity);
        }

        m_DestroyedEntities.clear();
    }

    Entity Scene::FindEntityByUUID(UUID uuid)
    {
        auto& it = m_EntityIDMap.find(uuid);

        if (it != m_EntityIDMap.end())
            return Entity{ m_EntityIDMap.at(uuid), this };

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

    Entity Scene::GetPrimaryCameraEntity()
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
        if (!m_Running)
            return;

        // Create and instantiate entity script
        Script::InvokeEntityOnCreate(entity);
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
        if (!m_Running)
            return;

        auto& transform = entity.GetComponent<TransformComponent>();
        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        auto& world = m_Registry.get<Box2DWorldComponent>(m_SceneEntity).World;

        // Copy position from transform component
        b2BodyDef bodyDef;
        bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type);
        bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
        bodyDef.angle = transform.Rotation.z;

        // Create body
        b2Body* body = world->CreateBody(&bodyDef);
        body->SetFixedRotation(rb2d.FixedRotation);
        body->SetEnabled(rb2d.Enabled);
        body->SetGravityScale(rb2d.GravityScale);
        rb2d.RuntimeBody = body;
    }

    template<>
    void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
    {
        if (!m_Running || !entity.HasComponent<Rigidbody2DComponent>())
            return;

        auto& transform = entity.GetComponent<TransformComponent>();
        auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
        b2Body* body = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody;

        b2PolygonShape boxShape;
        boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y, b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &boxShape;
        fixtureDef.density = bc2d.Density;
        fixtureDef.friction = bc2d.Friction;
        fixtureDef.restitution = bc2d.Restitution;
        fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
        fixtureDef.isSensor = bc2d.IsSensor;
        b2FixtureUserData data;
        data.pointer = (u64)entity.GetUUID();
        fixtureDef.userData = data;
        //fixtureDef.filter.categoryBits = bc2d.CategoryBits; // <- Is that category
        //fixtureDef.filter.maskBits = bc2d.MaskBits;		// <- Collides with other categories
        body->CreateFixture(&fixtureDef);
    }

    template<>
    void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<RelationshipComponent>(Entity entity, RelationshipComponent& component)
    {
    }
}
