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
            String Name;         
            Filepath RelativePath; 
        };

        Scene(const Scene::Config& config);
        ~Scene();

        void OnEditorUpdate(Time_T ts);
        void OnEditorRender(Ref<SceneRenderer> renderer, const Camera& editor_camera);

        void OnRuntimeStart();
        void OnRuntimeStop();
        void OnRuntimeUpdate(Time_T ts);
        void OnRuntimeRender(Ref<SceneRenderer> renderer);

        static Ref<Scene> Copy(Ref<Scene> other);

        Entity CreateEntity(const String& tag = "");
        Entity CreateEntityWithUUID(UUID uuid, const String& tag = "");
        void DestroyEntity(Entity entity);

        void SetViewportSize(u32 width, u32 height);

        const String& GetName() const { return m_Config.Name; }

        template<typename... Components>
        inline auto GetAllEntitiesWith()
        {
            return m_Registry.view<Components...>();
        }

        template<typename Func>
        void EachEntity(Func&& func)
        {
            m_Registry.each([&](auto id)
            {
                Entity entity = { id, this };
                func(entity);
            });
        }

        Entity GetEntityByUUID(UUID uuid);

        template<typename T>
        void OnComponentAdded(Entity entity, T& component);
    private:
        void CreatePhysicsWorld2D();
    private:
        entt::registry m_Registry;

        bool m_Running = false;

        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;

        b2World* m_PhysicsWorld = nullptr;

        Scene::Config m_Config;

        friend class Entity;
        friend class SceneSerializer;
        friend class Project;
        friend class ProjectSerializer;
    };
}
