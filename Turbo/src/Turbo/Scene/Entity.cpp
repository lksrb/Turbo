#include "tbopch.h"
#include "Entity.h"

namespace Turbo
{
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
        Entity currentParent = GetParent();

        if (newParent == currentParent)
            return;

        // Unparent from current parent
        UnParent();

        // Inform child entity that his parent has changed
        UUID newParentUUID = newParent.GetUUID();
        SetParentUUID(newParentUUID);
        auto& children = newParent.GetChildren();
        children.push_back(GetUUID());
    }

    void Entity::UnParent()
    {
        Entity parent = GetParent();
        if (!parent)
            return;

        parent.RemoveChild(*this);
        SetParentUUID(0);
    }

    void Entity::RemoveChild(Entity childEntity)
    {
        auto& children = GetChildren();
        auto& it = std::find(children.begin(), children.end(), childEntity.GetUUID());
        children.erase(it);
    }

}
