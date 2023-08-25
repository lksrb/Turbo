#pragma once

#include "Turbo/Asset/Asset.h"

#include "Entity.h"

namespace Turbo {
    
    class Entity;
    class Scene;

    class Prefab : public Asset
    {
    public:
        Prefab(u64 sceneCapacity);
        Prefab(Entity entity);
        ~Prefab();
        
        Entity GetPrefabEntity() const { return m_Entity; }

        AssetType GetAssetType() const override { return AssetType_Prefab; }
        static constexpr AssetType GetStaticType() { return AssetType_Prefab; }
    private:
        Entity CreatePrefabFromEntity(Entity entity, bool first = true);

        Entity m_Entity;
        Ref<Scene> m_Scene;

        friend class PrefabHandler;
    };

}
