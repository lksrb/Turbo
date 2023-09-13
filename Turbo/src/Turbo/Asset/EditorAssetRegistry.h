#pragma once

#include "AssetIO.h"
#include "AssetRegistryBase.h"

#include "Turbo/Renderer/Texture.h"
#include "Turbo/Renderer/Mesh.h"

#include <unordered_map>

namespace Turbo {

    class Project;
    class Entity;

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

        template<typename T, typename... Args>
        Ref<T> CreateAsset2(const std::filesystem::path& path, Args&&... args)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Class must be derived from \"Asset\" base class!");

            Ref<Asset> asset;
            if constexpr (std::is_same_v<Prefab, T>)
            {
                asset = Ref<Prefab>::Create(std::forward<Args>(args)...);
            }

            if (!asset)
                return nullptr;

            AssetMetadata metadata = CreateMetadata(path);
            m_AssetRegistry[asset->Handle] = metadata;
            m_LoadedAssets[asset->Handle] = asset;
            m_PathRegistry[metadata.FilePath] = asset->Handle;

            Asset::Serialize(metadata, asset);
            return asset;
        }

        bool RemoveAsset(AssetHandle handle);

        template<typename TAsset>
        Ref<Asset> ImportAsset2(const std::filesystem::path& filepath)
        {
            static_assert(std::is_base_of<Asset, TAsset>::value, "Class must be derived from \"Asset\" base class!");

            AssetMetadata metadata;
            metadata.FilePath = std::filesystem::relative(filepath, Project::GetAssetsPath());
            metadata.Type = TAsset::GetStaticAssetType();
            metadata.IsLoaded = true;

            // Is there a better solution?
            if (IsAssetImported(metadata.FilePath))
            {
                TBO_CONSOLE_WARN("Asset already loaded!");
                return GetAsset(m_PathRegistry.at(metadata.FilePath));
            }

            Ref<TAsset> asset = AssetIO::Import<TAsset>(filepath);
            if (!asset)
            {
                TBO_ENGINE_ERROR("Could not load source asset!");
                return nullptr;
            }

            m_LoadedAssets[asset->Handle] = asset;
            m_AssetRegistry[asset->Handle] = metadata;
            m_PathRegistry[metadata.FilePath] = asset->Handle;

            return asset;
        }

        AssetHandle ImportAsset(const std::filesystem::path& filepath) override;
        bool IsAssetImported(const std::filesystem::path& filepath) const;
        bool IsAssetHandleValid(AssetHandle handle) const override;
        bool IsAssetLoaded(AssetHandle handle) const override;
        Ref<Asset> GetAsset(AssetHandle handle) override;
        Ref<Asset> GetAsset(const std::filesystem::path& filepath) override;

        const AssetMetadata& GetAssetMetadata(AssetHandle handle) const override;

        const AssetRegistry& GetRegisteredAssets() const { return m_AssetRegistry; }
        const AssetMap& GetLoadedAssets() const { return m_LoadedAssets; }
    private:
        AssetMetadata CreateMetadata(const std::filesystem::path& filepath);
        void RegisterAsset(const Ref<Asset>& asset, const AssetMetadata& metadata);
        AssetHandle GetHandleFromPath(const std::filesystem::path& filepath);
        bool Deserialize();
        bool Serialize();

        std::unordered_map<std::filesystem::path, AssetHandle> m_PathRegistry;
        AssetRegistry m_AssetRegistry;
        AssetMap m_LoadedAssets;

        friend class AssetIO;
    };
}
