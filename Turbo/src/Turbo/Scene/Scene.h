#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Core/Time.h"
#include "Turbo/Core/Filepath.h"

#include "Turbo/Scene/Components.h"

#include <map>

#include <entt.hpp>

class b2World;

namespace Turbo
{
    class Entity;
    class SceneRenderer;

    class Scene
    {
    public:
        struct Config
        {
            // For scene serialization
            FString64 Name;         
            Filepath RelativePath; 
        };

        Scene(const Scene::Config& config);
        ~Scene();

        void OnEditorUpdate(Time_T ts) {}
        void OnEditorRender(SceneRenderer* renderer) {}

        void OnRuntimeStart();
        void OnRuntimeStop();
        void OnRuntimeUpdate(Time_T ts);
        void OnRuntimeRender(SceneRenderer* renderer);

        Entity CreateEntity(const FString64& tag = "");
        Entity CreateEntityWithUUID(UUID uuid, const FString64& tag = "");

        void SetViewportSize(u32 width, u32 height);

        const FString64& GetName() const { return m_Config.Name; }

        template<typename... Components>
        inline auto GetAllEntitiesWith()
        {
            return m_Registry.view<Components...>();
        }

        template<typename T>
        void OnComponentAdded(Entity entity, T& component);
    private:
        void CreatePhysicsWorld2D();
    private:
        entt::registry m_Registry;

        bool m_Running;

        u32 m_ViewportWidth;
        u32 m_ViewportHeight;

        b2World* m_PhysicsWorld;

        Scene::Config m_Config;

        friend class Entity;
        friend class SceneSerializer;
        friend class ProjectSerializer;
    };
}
