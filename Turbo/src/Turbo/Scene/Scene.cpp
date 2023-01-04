#include "tbopch.h"
#include "Scene.h"

#include "Turbo/Core/KeyCodes.h"

#include "Turbo/Scene/SceneCamera.h"
#include "Turbo/Renderer/SceneRenderer.h"

#include "Turbo/Scene/Entity.h"

namespace Turbo
{
    Scene::Scene(const Config& config)
        : m_Config(config), m_Running(false), m_ViewportWidth(0), m_ViewportHeight(0)
    {
    }

    Scene::~Scene()
    {
        
    }

    void Scene::OnRuntimeUpdate(Time_T ts)
    {
        // TODO: Physics
    }

    void Scene::OnRuntimeRender(SceneRenderer* renderer)
    {
        // Render entites

        // 2D Rendering
        {
            Renderer2D& renderer2d = renderer->GetRenderer2D();
            
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

                renderer2d.Begin(camera);
                {
                    auto& view = GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();

                    for (auto& entity : view)
                    {
                        auto& [transform, src] = view.get<TransformComponent, SpriteRendererComponent>(entity);
                        renderer2d.DrawSprite(transform.GetMatrix(), src.Color, src.Texture, src.Tiling, (u32)entity);
                    }
                }

                renderer2d.End();
            }
        }
    }

    Entity Scene::CreateEntity(const FString64& tag)
    {
        return CreateEntityWithUUID(UUID(), tag);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const FString64& tag)
    {
        Entity entity = { m_Registry.create(), this };
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();

        auto& tagComponent = entity.AddComponent<TagComponent>();
        tagComponent.Tag = tag.Empty() ? "Entity" : tag;

        return entity;
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

    //template<>
    //void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
    //{
    //}

    template<>
    void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
    {
    }

    //template<>
    //void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
    //{
    //}

    //template<>
    //void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
    //{
    //}

    template<>
    void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
    {
    }

    //template<>
    //void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
    //{
    //}


}
