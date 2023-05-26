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
        if (currentParent)
            currentParent.RemoveChild(*this);

        // Inform child entity that his parent has changed
        SetParentUUID(newParent.GetUUID());

        if (newParent)
        {
            auto& children = newParent.GetChildren();
            if (std::find(children.begin(), children.end(), GetUUID()) == children.end())
                children.emplace_back(GetUUID());
        }
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
