#pragma once

#include <entt.hpp>

#include "Turbo/Scene/Entity.h"
#include "Turbo/Scene/Scene.h"

namespace Turbo
{
    class Entity
    {
    public:
        Entity() = default;
        Entity(const Entity& other);
        Entity(entt::entity handle, Scene* scene);
        Entity(Entity&& other) noexcept;
        ~Entity() = default;

        template<typename Component, typename... Args>
        Component& AddComponent(Args&&... args)
        {
            TBO_ENGINE_ASSERT(!HasComponent<Component>(), "Entity already has this component!");
            Component& component = m_Scene->m_Registry.emplace<Component>(m_Handle, std::forward<Args>(args)...);
            return component;
        }

        template<typename Component, typename... Args>
        Component& ReplaceCompoment(Args&&... args)
        {
            Component& component = m_Scene->m_Registry.replace<Component>(m_Handle, std::forward<Args>(args)...);
            return component;
        }

        template<typename Component, typename... Args>
        Component& AddOrReplaceComponent(Args&&... args)
        {
            Component& component = m_Scene->m_Registry.emplace_or_replace<Component>(m_Handle, std::forward<Args>(args)...);
            return component;
        }

        template<typename... Components>
        decltype(auto) GetComponent()
        {
            TBO_ENGINE_ASSERT(HasComponent<Components...>(), "Entity does not have this component!");
            return m_Scene->m_Registry.get<Components...>(m_Handle);
        }

        template<typename... Components>
        decltype(auto) GetComponent() const
        {
            TBO_ENGINE_ASSERT(HasComponent<Components...>(), "Entity does not have this component!");
            return m_Scene->m_Registry.get<Components...>(m_Handle);
        }


        template<typename Component>
        void RemoveComponent()
        {
            TBO_ENGINE_ASSERT(HasComponent<Component>(), "Entity does not have this component!");
            m_Scene->m_Registry.remove<Component>(m_Handle);
        }

        template<typename... Components>
        bool HasComponent() const
        {
            TBO_ENGINE_ASSERT(IsValid(), "Entity is not valid!");
            return m_Scene->m_Registry.all_of<Components...>(m_Handle);
        }

        template<typename... Components>
        bool HasAnyComponent() const
        {
            TBO_ENGINE_ASSERT(IsValid(), "Entity is not valid!");
            return m_Scene->m_Registry.any_of<Components...>(m_Handle);
        }

        void SetParent(Entity newParent);
        void RemoveChild(Entity child);
        void UnParent();

        UUID GetUUID() { return GetComponent<IDComponent>().ID; }
        void SetParentUUID(UUID parentUUID) { GetComponent<RelationshipComponent>().Parent = parentUUID; }
        UUID GetParentUUID() { return GetComponent<RelationshipComponent>().Parent; }
        Entity GetParent() { return m_Scene->FindEntityByUUID(GetParentUUID()); }
        bool HasParent() { return GetParentUUID() != 0; }
        std::vector<UUID>& GetChildren() { return GetComponent<RelationshipComponent>().Children; }
        bool HasChildren() { return GetChildren().size() != 0; }

        const std::string& GetName() { return GetComponent<TagComponent>().Tag; }
        void SetName(const std::string& name) { GetComponent<TagComponent>().Tag = name; }
        TransformComponent& Transform() { return GetComponent<TransformComponent>(); }
        bool IsValid() const { return m_Handle != entt::null && m_Scene != nullptr; }

        // Operators
        inline Entity operator=(const Entity& other)
        {
            m_Handle = other.m_Handle;
            m_Scene = other.m_Scene;
            return *this;
        }

        inline Entity& operator=(Entity&& other) noexcept
        {
            m_Handle = std::move(other.m_Handle);
            m_Scene = std::move(other.m_Scene);
            return *this;
        }

        operator bool() const { return IsValid(); }
        operator uint32_t() const { return (uint32_t)m_Handle; }
        operator entt::entity() const { return m_Handle; }

        bool operator==(const Entity& other) const { return m_Handle == other.m_Handle && m_Scene == other.m_Scene; }
        bool operator!=(const Entity& other) const { return (*this == other) == false; }
    private:
        entt::entity m_Handle = entt::null;
        Scene* m_Scene = nullptr;

        friend class Scene;
        friend class Prefab;

        // FIXME: Maybe not ideal
        friend class AssetManager;
    };
}

