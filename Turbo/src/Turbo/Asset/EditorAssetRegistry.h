#pragma once

#include "AssetRegistryBase.h"

namespace Turbo
{
    class EditorAssetRegistry : public AssetRegistryBase
    {
    public:
        EditorAssetRegistry() = default;
        ~EditorAssetRegistry();
        
        // FIXME: Bad solution for this
        template<typename T, typename... Args>
        Ref<T> RecreateAsset(AssetHandle handle, Args&&... args)
        {
            if (!IsAssetLoaded(handle))
                return nullptr;

            Ref<Asset> asset;
            if constexpr (std::is_same_v<Texture2D, T>)
            {
                asset = Texture2D::Create(std::forward<Args>(args)...);
                if (!asset.As<Texture2D>()->IsLoaded())
                    return nullptr;
            }
            
            // Serialize metadata
            auto& metadata = GetAssetMetadata(handle);
            Asset::Serialize(metadata, asset);

            m_LoadedAssets[handle] = asset;
            return asset;
        }
        void Init() override;
        void ImportAsset(const std::filesystem::path& filepath) override;
        bool IsAssetHandleValid(AssetHandle handle) const override;
        bool IsAssetLoaded(AssetHandle handle) const override;
        Ref<Asset> GetAsset(AssetHandle handle) override;
        const AssetMetadata& GetAssetMetadata(AssetHandle handle) const override;

        const AssetRegistry& GetRegisteredAssets() const { return m_AssetRegistry; }
        const AssetMap& GetLoadedAssets() const { return m_LoadedAssets; }
    private:
        void LoadDefaultAssets();
        bool Deserialize();
        bool Serialize();

        AssetMap m_MemoryOnlyAssets;
        AssetRegistry m_AssetRegistry;
        AssetMap m_LoadedAssets;
    };
}
