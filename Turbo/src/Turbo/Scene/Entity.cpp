#include "tbopch.h"
#include "Entity.h"

namespace Turbo {

    Entity::Entity(const Entity& other)
        : m_Handle(other.m_Handle), m_Scene(other.m_Scene)
    {
    }

    Entity::Entity(Entity&& other) noexcept
    {
        m_Handle = std::move(other.m_Handle);
        m_Scene = std::move(other.m_Scene);
    }

    Entity::Entity(entt::entity handle, Scene* scene)
        : m_Handle(handle), m_Scene(scene)
    {
    }

    void Entity::SetParent(Entity newParent)
    {
        if (!newParent)
            return;

        Entity currentParent = GetParent();

        if (newParent == currentParent)
            return;

        // Unparent from current parent
        UnParent();

        // Inform child entity that his parent has changed
        SetParentUUID(newParent.GetUUID());

        auto& children = newParent.GetChildren();
        if (std::find(children.begin(), children.end(), GetUUID()) == children.end())
        {
            children.emplace_back(GetUUID());
            m_Scene->ConvertToLocalSpace(*this); // This might be a problem with nested hierarchy
        }

    }

    void Entity::UnParent()
    {
        Entity parent = GetParent();
        if (!parent)
            return;

        m_Scene->ConvertToWorldSpace(*this);

        parent.RemoveChild(*this);
        SetParentUUID(0);
    }

    void Entity::RemoveChild(Entity childEntity)
    {
        auto& children = GetChildren();
        auto it = std::find(children.begin(), children.end(), childEntity.GetUUID());
        children.erase(it);
    }

}
