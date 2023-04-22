#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Core/Time.h"

#include "Turbo/Scene/Components.h"

#include <map>

#include <entt.hpp>

namespace Turbo
{
    class Entity;
    class SceneRenderer;
    class Renderer2D;

    using EntityMap = std::unordered_map<UUID, entt::entity>;
    class Scene
    {
    public:
        Scene();
        ~Scene();

        // Accessible variables
        bool ShowPhysics2DColliders = false;

        void OnEditorUpdate(FTime ts);
        void OnEditorRender(Ref<SceneRenderer> renderer, const Camera& editorCamera);

        void OnRuntimeStart();
        void OnRuntimeStop();
        void OnRuntimeUpdate(FTime ts);
        void OnRuntimeRender(Ref<SceneRenderer> renderer);

        static Ref<Scene> Copy(Ref<Scene> other);

        Entity CreateEntity(const std::string& tag = "");
        Entity CreateEntityWithUUID(UUID uuid, const  std::string& tag = "");
        void DestroyEntity(Entity entity);
        Entity DuplicateEntity(Entity entity);

        void SetViewportOffset(u32 x, u32 y);
        void SetViewportSize(u32 width, u32 height);

        // Editor only
        u32 GetViewportX() const { return m_ViewportX; }
        u32 GetViewportY() const { return m_ViewportY; }

        u32 GetViewportWidth() const { return m_ViewportWidth; }
        u32 GetViewportHeight() const { return m_ViewportHeight; }

        template<typename... Components>
        inline auto GetAllEntitiesWith()
        {
            return m_Registry.view<Components...>();
        }

        template<typename Func>
        inline void EachEntity(Func&& func)
        {
            m_Registry.each([&](auto id)
            {
                Entity entity = { id, this };
                func(entity);
            });
        }

        Entity FindEntityByUUID(UUID uuid);
        Entity FindEntityByName(const std::string& name);

        Entity GetPrimaryCameraEntity();

        bool IsRunning() const { return m_Running; }

        template<typename T>
        void OnComponentAdded(Entity entity, T& component);
    private:
        Entity FindPrimaryCameraEntity();
        Entity FindPrimaryAudioListenerEntity();

        void ClearDeletedEntities();
    private:
        entt::registry m_Registry;

        entt::entity m_PrimaryAudioListenerEntity = entt::null;
        entt::entity m_PrimaryCameraEntity = entt::null;

        entt::entity m_SceneEntity = entt::null;
        UUID m_SceneID;

        EntityMap m_EntityIDMap;

        std::vector<entt::entity> m_DestroyedEntities;

        bool m_Running = false;

        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;
        u32 m_ViewportX = 0, m_ViewportY = 0;

        friend class Entity;
        friend class SceneSerializer;
        friend class PhysicsScene2D;
        friend class Project;
        friend class ProjectSerializer;
    };
}
