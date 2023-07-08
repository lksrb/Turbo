#pragma once

#include "AssetRegistryBase.h"

namespace Turbo
{
    class EditorAssetRegistry : public AssetRegistryBase
    {
    public:
        EditorAssetRegistry();
        ~EditorAssetRegistry();
        
        void ImportAsset(const std::filesystem::path& filepath) override;
        bool IsAssetHandleValid(AssetHandle handle) const override;
        bool IsAssetLoaded(AssetHandle handle) const override;
        Ref<Asset> GetAsset(AssetHandle handle) override;
        AssetMetadata GetAssetMetadata(AssetHandle handle) const override;

        const AssetRegistry& GetRegisteredAssets() const { return m_AssetRegistry; }
        const AssetMap& GetLoadedAssets() const { return m_LoadedAssets; }
    private:
        bool Deserialize();
        bool Serialize();

        AssetRegistry m_AssetRegistry;
        AssetMap m_LoadedAssets;
    };
}
