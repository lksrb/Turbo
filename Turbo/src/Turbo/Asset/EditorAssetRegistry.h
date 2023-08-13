#pragma once

#include "AssetRegistryBase.h"

#include "Turbo/Renderer/Texture.h"
#include "Turbo/Renderer/Mesh.h"

#include <unordered_map>

namespace Turbo {

    class EditorAssetRegistry : public AssetRegistryBase
    {
    public:
        EditorAssetRegistry();
        ~EditorAssetRegistry();

        // FIXME: Bad solution for this
        template<typename T, typename... Args>
        Ref<T> RecreateAsset(AssetHandle handle, Args&&... args)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Class must be derived from \"Asset\" base class!");
            static_assert(std::is_base_of<Asset, Texture2D>::value, "Unknown asset type!");

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
        template<typename T, typename... Args>
        Ref<T> CreateAsset(const std::filesystem::path& path, Args... args)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Class must be derived from \"Asset\" base class!");
            static_assert(std::is_base_of<Asset, StaticMesh>::value, "Unknown asset type!");

            Ref<Asset> asset;
            if constexpr (std::is_same_v<StaticMesh, T>)
            {
                auto sourceAsset = GetAsset(args...);
                asset = Ref<StaticMesh>::Create(sourceAsset);
            }

            AssetMetadata metadata = CreateMetadata(path);
            m_AssetRegistry[asset->Handle] = metadata;
            m_LoadedAssets[asset->Handle] = asset;
            m_PathRegistry[metadata.FilePath] = asset->Handle;

            Asset::Serialize(metadata, asset);

            return asset;
        }

        AssetHandle ImportAsset(const std::filesystem::path& filepath) override;
        bool IsAssetImported(const std::filesystem::path& filepath);
        bool IsAssetHandleValid(AssetHandle handle) const override;
        bool IsAssetLoaded(AssetHandle handle) const override;
        Ref<Asset> GetAsset(AssetHandle handle) override;
        const AssetMetadata& GetAssetMetadata(AssetHandle handle) const override;

        const AssetRegistry& GetRegisteredAssets() const { return m_AssetRegistry; }
        const AssetMap& GetLoadedAssets() const { return m_LoadedAssets; }
    private:
        AssetMetadata CreateMetadata(const std::filesystem::path& filepath);
        bool Deserialize();
        bool Serialize();

        std::unordered_map<std::filesystem::path, AssetHandle> m_PathRegistry;
        AssetRegistry m_AssetRegistry;
        AssetMap m_LoadedAssets;
    };
}
