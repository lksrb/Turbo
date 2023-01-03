#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Core/Time.h"
#include "Turbo/Core/Filepath.h"

#include "Turbo/Scene/Components.h"

#include <entt.hpp>

namespace Turbo
{
    class Entity;
    class SceneRenderer;

    class Scene
    {
    public:
        struct Config
        {
            Filepath RelativePath;
            FString64 Name;
        };

        Scene(const Config& config);
        ~Scene();

        void OnEditorUpdate(Time_T ts) {}
        void OnEditorRender(SceneRenderer* renderer) {}

        void OnRuntimeUpdate(Time_T ts);
        void OnRuntimeRender(SceneRenderer* renderer);

        Entity CreateEntity(const FString64& tag = "");
        Entity CreateEntityWithUUID(UUID uuid, const FString64& tag = "");

        void OnViewportResize(u32 width, u32 height);

        const FString64& GetName() const { return m_Config.Name; }

        template<typename... Components>
        inline auto GetAllEntitiesWith()
        {
            return m_Registry.view<Components...>();
        }

        template<typename T>
        void OnComponentAdded(Entity entity, T& component);
    private:
        entt::registry m_Registry;

        bool m_Running;

        u32 m_ViewportWidth;
        u32 m_ViewportHeight;

        Scene::Config m_Config;

        friend class Entity;
        friend class SceneSerializer;
        friend class ProjectSerializer;
    };
}
