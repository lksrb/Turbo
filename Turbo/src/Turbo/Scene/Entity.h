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
        Entity(const Entity& other)
            : m_Handle(other.m_Handle), m_Scene(other.m_Scene)
        {
        }

        Entity(entt::entity handle, Scene* scene)
            : m_Handle(handle), m_Scene(scene)
        {
        }

        Entity(Entity&& other) noexcept
        {
            m_Handle = std::move(other.m_Handle);
            m_Scene = std::move(other.m_Scene);
        }

        ~Entity() {}

        template<typename Component, typename... Args>
        Component& AddComponent(Args&&... args)
        {
            TBO_ENGINE_ASSERT(!HasComponent<Component>(), "Entity already has this component!");
            Component& component = m_Scene->m_Registry.emplace<Component>(m_Handle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<Component>(*this, component);
            return component;
        }

        template<typename Component, typename... Args>
        Component& AddCustomComponent(Args&&... args)
        {
            TBO_ENGINE_ASSERT(!HasComponent<Component>(), "Entity already has this component!");
            Component& component = m_Scene->m_Registry.emplace<Component>(m_Handle, std::forward<Args>(args)...);
            return component;
        }

        template<typename Component, typename... Args>
        Component& AddOrReplaceComponent(Args&&... args)
        {
            Component& component = m_Scene->m_Registry.emplace_or_replace<Component>(m_Handle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<Component>(*this, component);
            return component;
        }

        template<typename Component>
        Component& GetComponent()
        {
            TBO_ENGINE_ASSERT(HasComponent<Component>(), "Entity does not have this component!");
            return m_Scene->m_Registry.get<Component>(m_Handle);
        }

        template<typename... Components>
        decltype(auto) GetComponents()
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

        template<typename... Component>
        bool HasComponent()
        {
            TBO_ENGINE_ASSERT(IsValid(), "Entity is not valid!");
            return m_Scene->m_Registry.all_of<Component...>(m_Handle);
        }

        void SetParent(Entity newParent)
        {
            Entity currentParent = GetParent();
            if (newParent == currentParent)
                return;

            // Entity has a parent => remove it from parent's children list
            if (currentParent)
                currentParent.RemoveChild(*this);

            // Inform child entity that his parent has changed
            SetParentUUID(newParent.GetUUID());

            newParent.GetChildren().push_back(GetUUID());
        }
        
        void RemoveChild(Entity childEntity)
        {
            auto& children = GetChildren();
            auto& it = std::find(children.begin(), children.end(), childEntity.GetUUID());
            children.erase(it);
        }

        UUID GetUUID() { return GetComponent<IDComponent>().ID; }
        void SetParentUUID(UUID parentUUID) { GetComponent<RelationshipComponent>().Parent = parentUUID; }
        UUID GetParentUUID() { return GetComponent<RelationshipComponent>().Parent; }
        Entity GetParent() { return m_Scene->FindEntityByUUID(GetParentUUID()); }
        std::vector<UUID>& GetChildren() { return GetComponent<RelationshipComponent>().Children; }
        bool HasChildren() { return GetChildren().size() != 0; }

        const std::string& GetName() { return GetComponent<TagComponent>().Tag; }
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

        operator bool() const { return m_Handle != entt::null && m_Scene != nullptr; }
        operator uint32_t() const { return (uint32_t)m_Handle; }
        operator entt::entity() const { return m_Handle; }

        bool operator==(const Entity& other) const { return m_Handle == other.m_Handle && m_Scene == other.m_Scene; }
        bool operator!=(const Entity& other) const { return (*this == other) == false; }
    private:
        entt::entity m_Handle = entt::null;
        Scene* m_Scene = nullptr;
    };
}

