#include "tbopch.h"
#include "Prefab.h"

namespace Turbo {

    Prefab::Prefab(Entity entity) 
        : Prefab(entity.GetChildren().size() + 1)
    {
        CreatePrefabFromEntity(entity);
    }

    Prefab::Prefab(u64 sceneCapacity) 
        : m_Scene(Scene::CreateEmpty(sceneCapacity))
    {
    }

    Prefab::~Prefab()
    {
    }

    Entity Prefab::CreatePrefabFromEntity(Entity entity, bool first)
    {
        Entity prefabEntity = m_Scene->CreateEntity(entity.GetName());

        // RelationshipComponent is not valid anymore after this
        m_Scene->CopyEntity(entity, prefabEntity);

        if (first)
        {
            m_Entity = prefabEntity;

            // Reset parent UUID since is no longer valid
            prefabEntity.SetParentUUID(0);

            // If prefab is a child, convert it back to world space
            prefabEntity.Transform() = entity.m_Scene->GetWorldSpaceTransform(entity);
        }

        // Map source's scene UUIDs to prefab scene
        auto& children = prefabEntity.GetChildren();
        for (auto& childUUID : children)
        {
            Entity child = entity.m_Scene->FindEntityByUUID(childUUID);

            Entity newEntity = CreatePrefabFromEntity(child, false);

            newEntity.SetParentUUID(prefabEntity.GetUUID());
            childUUID = newEntity.GetUUID();
        }

        return prefabEntity;
    }

}
