#pragma once

#include "Turbo/Asset/Asset.h"

#include "Material.h"

namespace Turbo {

    class MaterialAsset : public Asset
    {
    public:
        MaterialAsset();

        void SetAlbedoColor(const glm::vec3& albedoColor);

        AssetType GetAssetType() const override { return AssetType_Count; }
    private:
        Ref<Material> m_Material;
    };

}
