#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Core/Time.h"

#include "Turbo/Scene/Components.h"

#include <map>

#include <entt.hpp>

namespace Turbo
{
    class Entity;
    class SceneDrawList;
    class DrawList2D;
    class PhysicsWorld2D;

    using EntityMap = std::unordered_map<UUID, entt::entity>;
    using UUIDMap = std::map<entt::entity, UUID>;

    class Scene : public RefCounted
    {
    public:
        struct Statistics
        {
            u32 MaxEntities = 0;
            u32 CurrentEntities = 0;
        };

        Scene();
        ~Scene();

        void OnRuntimeStart();
        void OnRuntimeStop();

        void OnEditorUpdate(Ref<SceneDrawList> drawList, const Camera& editorCamera, FTime ts);
        void OnRuntimeUpdate(Ref<SceneDrawList> drawList, FTime ts);

        static Ref<Scene> Copy(Ref<Scene> other);

        Entity CreateEntity(const std::string& tag = "");
        Entity CreateEntityWithUUID(UUID uuid, const  std::string& tag = "");
        void DestroyEntity(Entity entity, bool excludeChildren = false, bool first = true);
        Entity DuplicateEntity(Entity entity);
        void CopyEntity(Entity src, Entity dst);

        void SetViewportOffset(i32 x, i32 y);
        void SetViewportSize(u32 width, u32 height);

        auto& GetPostUpdateFuncs() { return m_PostUpdateFuncs; }

        bool Contains(Entity entity) const;

        // Editor only
        i32 GetViewportX() const { return m_ViewportX; }
        i32 GetViewportY() const { return m_ViewportY; }

        u32 GetViewportWidth() const { return m_ViewportWidth; }
        u32 GetViewportHeight() const { return m_ViewportHeight; }

        template<typename... Components>
        inline auto GetAllEntitiesWith()
        {
            return m_Registry.view<Components...>();
        }

        template<typename Component, typename...Components>
        inline auto GroupAllEntitiesWith()
        {
            return m_Registry.group<Component>(entt::get<Components...>);
        }

        Entity FindEntityByUUID(UUID uuid);
        UUID FindUUIDByEntity(entt::entity entity);
        Entity FindEntityByName(const std::string& name);

        Entity FindPrimaryCameraEntity();
        Entity FindPrimaryAudioListenerEntity();

        PhysicsWorld2D* GetPhysicsWorld2D();

        Scene::Statistics GetStatistics() const { return m_Statistics; }

        bool IsRunning() const { return m_Running; }

        glm::mat4 GetWorldSpaceTransformMatrix(Entity entity);
        void ConvertToLocalSpace(Entity entity);
        void ConvertToWorldSpace(Entity entity);
        TransformComponent GetWorldSpaceTransform(Entity entity);
    private:
        void RenderScene(Ref<SceneDrawList> drawList);
    private:
        void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity);
        void OnScriptComponentDestroy(entt::registry& registry, entt::entity entity);
        void OnAudioSourceComponentConstruct(entt::registry& registry, entt::entity entity);
        void OnAudioSourceComponentDestroy(entt::registry& registry, entt::entity entity);
        void OnRigidBody2DComponentConstruct(entt::registry& registry, entt::entity entity);
        void OnRigidBody2DComponentDestroy(entt::registry& registry, entt::entity entity);
        void OnBoxCollider2DComponentConstruct(entt::registry& registry, entt::entity entity);
        void OnBoxCollider2DComponentUpdate(entt::registry& registry, entt::entity entity);
        void OnBoxCollider2DComponentDestroy(entt::registry& registry, entt::entity entity);
        void OnCircleCollider2DComponentConstruct(entt::registry& registry, entt::entity entity);
        void OnCircleCollider2DComponentUpdate(entt::registry& registry, entt::entity entity);
        void OnCircleCollider2DComponentDestroy(entt::registry& registry, entt::entity entity);
    private:
        entt::registry m_Registry;

        entt::entity m_PrimaryAudioListenerEntity = entt::null;
        entt::entity m_PrimaryCameraEntity = entt::null;

        entt::entity m_SceneEntity = entt::null;
        UUID m_SceneID;

        EntityMap m_EntityIDMap;
        UUIDMap m_UUIDMap;

        std::vector<std::function<void()>> m_PostUpdateFuncs;

        bool m_Running = false;

        Scene::Statistics m_Statistics;

        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;
        i32 m_ViewportX = 0, m_ViewportY = 0;

        friend class Entity;
        friend class SceneSerializer;
        friend class PhysicsScene2D;
        friend class Project;
        friend class ProjectSerializer;
        friend class AssetManager;
    };
}
