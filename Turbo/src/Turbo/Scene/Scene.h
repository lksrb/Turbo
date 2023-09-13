#pragma once

#include "Turbo/Core/Time.h"
#include "Turbo/Core/Memory.h"

#include "Turbo/Scene/Components.h"

#include <map>
#include <entt.hpp>

namespace Turbo {

    class Entity;
    class SceneDrawList;
    class PhysicsWorld;
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

        Scene(bool isEditorScene = false, bool initialized = true, u64 capacity = 200);
        ~Scene();

        void OnRuntimeStart();
        void OnRuntimeStop();

        void OnEditorUpdate(OwnedRef<SceneDrawList> drawList, const Camera& editorCamera, FTime ts);
        void OnRuntimeUpdate(OwnedRef<SceneDrawList> drawList, FTime ts);

        static Ref<Scene> Copy(Ref<Scene> other);

        Entity CreateEntity(std::string_view tag = "");
        Entity CreateEntityWithUUID(UUID uuid, std::string_view tag = "");
        void DestroyEntity(Entity entity, bool excludeChildren = false, bool first = true);
        Entity DuplicateEntity(Entity entity);

        // Light copy of an entity - Only components are copied
        void CopyComponents(Entity dst, Entity src);
        
        Entity CreatePrefabEntity(Entity prefabEntity, Entity parent, const glm::vec3* translation = nullptr, const glm::vec3* rotation = nullptr, const glm::vec3* scale = nullptr);

        void SetViewportOffset(i32 x, i32 y);
        void SetViewportSize(u32 width, u32 height);

        template<typename F>
        void AddToDrawList(F&& func)
        {
            m_DebugRendererCallbacks.push_back(std::forward<F>(func));
        }

        template<typename F>
        void AddToPostUpdate(F&& func) { m_PostUpdateFuncs.emplace_back(std::move(func)); }

        bool Contains(Entity entity) const;

        const FTime TimeSinceStart() const { return m_TimeSinceStart; }

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

        // Groups can own components, that means OwnedComponent can only be used by the same type of group.
        template<typename OwnedComponent, typename...Components>
        inline auto GroupAllEntitiesWith()
        {
            return m_Registry.group<OwnedComponent>(entt::get<Components...>);
        }

        Entity FindEntityByUUID(UUID uuid);
        UUID FindUUIDByEntity(entt::entity entity);
        Entity FindEntityByName(std::string_view name);

        Entity FindPrimaryCameraEntity();
        Entity FindPrimaryAudioListenerEntity();

        Ref<PhysicsWorld2D> GetPhysicsWorld2D() const;
        Ref<PhysicsWorld> GetPhysicsWorld() const;

        Scene::Statistics GetStatistics() const { return m_Statistics; }

        bool IsRunning() const { return m_Running; }

        glm::mat4 GetWorldSpaceTransformMatrix(Entity entity);
        void ConvertToLocalSpace(Entity entity);
        void ConvertToWorldSpace(Entity entity);
        TransformComponent GetWorldSpaceTransform(Entity entity);
        static Ref<Scene> CreateEmpty(u64 capacity) { return Ref<Scene>::Create(true, false, capacity); }
    private:
        void RenderScene(OwnedRef<SceneDrawList> drawList);
    private:
        void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity);
        void OnScriptComponentDestroy(entt::registry& registry, entt::entity entity);
        void OnAudioSourceComponentConstruct(entt::registry& registry, entt::entity entity);
        void OnAudioSourceComponentDestroy(entt::registry& registry, entt::entity entity);
    private:
        entt::registry m_Registry;

        entt::entity m_PrimaryAudioListenerEntity = entt::null;
        entt::entity m_PrimaryCameraEntity = entt::null;

        entt::entity m_SceneEntity = entt::null;
        UUID m_SceneID;

        EntityMap m_EntityIDMap;
        UUIDMap m_UUIDMap;

        std::vector<std::function<void(OwnedRef<SceneDrawList>)>> m_DebugRendererCallbacks;
        std::vector<std::function<void()>> m_PostUpdateFuncs;

        bool m_IsEditorScene = false;
        bool m_Running = false;

        Scene::Statistics m_Statistics;
        FTime m_TimeSinceStart = 0.0f;

        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;
        i32 m_ViewportX = 0, m_ViewportY = 0;

        friend class Entity;
        friend class SceneSerializer;
        friend class Project;
        friend class ProjectSerializer;
        friend class PrefabHandler;
        friend class AssetManager;
    };
}
