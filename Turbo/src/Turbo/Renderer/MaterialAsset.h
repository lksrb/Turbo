#pragma once

#include "Turbo/Asset/Asset.h"

#include "Material.h"

namespace Turbo {

    class MaterialAsset : public Asset
    {
    public:
        MaterialAsset();

        void SetAlbedoColor(const glm::vec3& albedoColor);
        glm::vec3 GetAlbedoColor() const { return m_AlbedoColor; }

        AssetType GetAssetType() const override { return AssetType_MaterialAsset; }
        static constexpr AssetType GetStaticAssetType() { return AssetType_MaterialAsset; }
    private:
        glm::vec3 m_AlbedoColor;

        Ref<Material> m_Material;
    };

}
