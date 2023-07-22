#pragma once

#include "AssetRegistryBase.h"
#include "AssetImporter.h"

namespace Turbo
{
    class EditorAssetRegistry : public AssetRegistryBase
    {
    public:
        EditorAssetRegistry();
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
            TBO_ENGINE_ASSERT(AssetImporter::Serialize(metadata, asset));

            m_LoadedAssets[handle] = asset;
            return asset;
        }

        void ImportAsset(const std::filesystem::path& filepath) override;
        bool IsAssetHandleValid(AssetHandle handle) const override;
        bool IsAssetLoaded(AssetHandle handle) const override;
        Ref<Asset> GetAsset(AssetHandle handle) override;
        const AssetMetadata& GetAssetMetadata(AssetHandle handle) const override;

        const AssetRegistry& GetRegisteredAssets() const { return m_AssetRegistry; }
        const AssetMap& GetLoadedAssets() const { return m_LoadedAssets; }
    private:
        bool Deserialize();
        bool Serialize();

        AssetRegistry m_AssetRegistry;
        AssetMap m_LoadedAssets;
    };
}
