#include "tbopch.h"
#include "Scene.h"

#include "Turbo/Core/KeyCodes.h"

#include "Turbo/Scene/SceneCamera.h"
#include "Turbo/Renderer/SceneRenderer.h"

#include "Turbo/Scene/Entity.h"

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
                    entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).Uuid);
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

    void Scene::OnEditorUpdate(Time_T ts)
    {
    }

    void Scene::OnEditorRender(Ref<SceneRenderer> renderer, const Camera& editor_camera)
    {
        renderer->BeginRender();

        // Render entites

        // 2D Rendering
        {
            Ref<Renderer2D> renderer2d = renderer->GetRenderer2D();
            {
                renderer2d->Begin(editor_camera);
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();

                    for (auto& entity : view)
                    {
                        auto& [transform, src] = view.get<TransformComponent, SpriteRendererComponent>(entity);

                        if (src.SubTexture)
                            renderer2d->DrawSprite(transform.GetMatrix(), src.Color, src.SubTexture, src.Tiling, (u32)entity);
                        else
                            renderer2d->DrawSprite(transform.GetMatrix(), src.Color, src.Texture, src.Tiling, (u32)entity);

                    }
                }

                renderer2d->End();
            }
        }

        renderer->EndRender();
    }

    void Scene::OnRuntimeStart()
    {
        CreatePhysicsWorld2D();
    }

    void Scene::OnRuntimeStop()
    {
        delete m_PhysicsWorld;
        m_PhysicsWorld = nullptr;
    }

    void Scene::OnRuntimeUpdate(Time_T ts)
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

            if(rb2d.FixedRotation == false)
                transform.Rotation.z = body->GetAngle();
        }
    }

    void Scene::OnRuntimeRender(Ref<SceneRenderer> renderer)
    {
        renderer->BeginRender();

        // 2D Rendering
        {
            Ref<Renderer2D> renderer2d = renderer->GetRenderer2D();

            // Find entity with camera component
            Entity cameraEntity;
            auto& view = GetAllEntitiesWith<CameraComponent>();
            for (auto entity : view)
            {
                auto& camera = view.get<CameraComponent>(entity);

                if (camera.Primary)
                {
                    cameraEntity = { entity, this };
                }
            }

            // Camera does exists
            if (cameraEntity)
            {
                auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
                camera.SetViewMatrix(glm::inverse(cameraEntity.Transform().GetMatrix()));

                renderer2d->Begin(camera);
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();

                    for (auto& entity : view)
                    {
                        auto& [transform, src] = view.get<TransformComponent, SpriteRendererComponent>(entity);

                        if (src.SubTexture)
                            renderer2d->DrawSprite(transform.GetMatrix(), src.Color, src.SubTexture, src.Tiling, (u32)entity);
                        else
                            renderer2d->DrawSprite(transform.GetMatrix(), src.Color, src.Texture, src.Tiling, (u32)entity);

                    }
                }

                renderer2d->End();
            }
        }

        renderer->EndRender();
    }

    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
        Ref<Scene> new_scene = Ref<Scene>::Create();

        new_scene->m_ViewportWidth = other->m_ViewportWidth;
        new_scene->m_ViewportHeight = other->m_ViewportHeight;

        std::unordered_map<UUID, entt::entity> entity_map;

        auto& view = other->m_Registry.view<IDComponent>();

        for (auto& it = view.rbegin(); it != view.rend(); ++it)
        {
            UUID uuid = other->m_Registry.get<IDComponent>(*it).Uuid;

            //TBO_CORE_ASSERT(entityMap.find(uuid) != entityMap.end());

            const auto& name = other->m_Registry.get<TagComponent>(*it).Tag;

            entity_map[uuid] = (entt::entity)new_scene->CreateEntityWithUUID(uuid, name);
        }

        // Copy components (except IDComponent and TagComponent)
        Utils::CopyComponents(AllComponents{}, new_scene->m_Registry, other->m_Registry, entity_map, new_scene);

        return new_scene;
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
        auto view = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity = { e, this };
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

            glm::vec2 offset = { 0.0f, 0.0f };

            if (entity.HasComponent<BoxCollider2DComponent>())
            {
                auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
                offset = bc2d.Offset;
            }

            // Copy position from transform component
            b2BodyDef bodyDef;
            bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type);
            bodyDef.position.Set(transform.Translation.x + offset.x, transform.Translation.y + offset.y);
            bodyDef.angle = transform.Rotation.z;

            // Create body
            b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
            body->SetFixedRotation(rb2d.FixedRotation);
            body->SetEnabled(rb2d.Enabled);
            if (rb2d.Gravity == false)
                body->SetGravityScale(0.0f);

            if (entity.HasComponent<BoxCollider2DComponent>())
            {
                auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

                b2PolygonShape boxShape;
                boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);
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
                circleShape.m_p = { cc2d.Offset.x, cc2d.Offset.y };

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

            rb2d.RuntimeBody = body;
        }
    }

	Entity Scene::GetEntityByUUID(UUID uuid)
    {
        auto& view = GetAllEntitiesWith<IDComponent>();
        for (auto e : view)
        {
            auto& id = m_Registry.get<IDComponent>(e).Uuid;

            if (id == uuid)
                return Entity{ e, this };
        }

        return Entity{};
    }

	// Events ------------------------------------------------------------------------------------

    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
        static_assert(sizeof(T) == 0);
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
        if(m_ViewportWidth > 0 && m_ViewportHeight > 0)
            component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
    }

    template<>
    void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
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
