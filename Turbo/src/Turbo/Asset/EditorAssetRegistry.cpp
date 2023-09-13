#include "tbopch.h"
#include "EditorAssetRegistry.h"

#include "Turbo/Scene/Entity.h"
#include "Turbo/Solution/Project.h"

#include <yaml-cpp/yaml.h>

namespace Turbo {

    namespace Utils {

        static AssetType GetAssetTypeFromExtension(const std::filesystem::path& extension)
        {
            if (extension == ".png")   return AssetType_Texture2D;
            if (extension == ".fbx" || extension == ".gltf")   return AssetType_MeshSource;
            if (extension == ".tmesh") return AssetType_StaticMesh;
            if (extension == ".tprefab") return AssetType_Prefab;

            TBO_ENGINE_ASSERT(false, "Unsupported extension!");

            return AssetType_Count;
        }

        static std::filesystem::path GetMetadataPath(const std::filesystem::path& filepath)
        {
            auto assetsPath = Project::GetAssetsPath();
            auto metadataPath = std::filesystem::relative(filepath, assetsPath);
            return metadataPath;
        }
    }

    EditorAssetRegistry::EditorAssetRegistry()
    {
        Deserialize();
    }

    EditorAssetRegistry::~EditorAssetRegistry()
    {
        Serialize();
    }

    bool EditorAssetRegistry::IsAssetHandleValid(AssetHandle handle) const
    {
        return handle != 0 && m_AssetRegistry.find(handle) != m_AssetRegistry.end();
    }

    bool EditorAssetRegistry::IsAssetLoaded(AssetHandle handle) const
    {
        return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
    }

    Ref<Asset> EditorAssetRegistry::GetAsset(AssetHandle handle)
    {
        if (!IsAssetHandleValid(handle))
            return nullptr;

        if (!IsAssetLoaded(handle))
        {
            auto& metadata = m_AssetRegistry.at(handle);
            Ref<Asset> asset = Asset::TryLoad(metadata);
            if (!asset)
                return nullptr;

            metadata.IsLoaded = true;
            asset->Handle = handle;
            m_LoadedAssets[handle] = asset;
            m_AssetRegistry[handle] = metadata;
        }

        return m_LoadedAssets.at(handle);
    }

    Ref<Asset> EditorAssetRegistry::GetAsset(const std::filesystem::path& filepath)
    {
        return GetAsset(GetHandleFromPath(filepath));
    }

    const AssetMetadata& EditorAssetRegistry::GetAssetMetadata(AssetHandle handle) const
    {
        static AssetMetadata s_NullMetadata;

        if (!IsAssetHandleValid(handle))
            return s_NullMetadata;

        return m_AssetRegistry.at(handle);
    }

    AssetHandle EditorAssetRegistry::ImportAsset(const std::filesystem::path& filepath)
    {
        AssetMetadata metadata = CreateMetadata(filepath);

        // NOTE: This isnt ideal solution but good enough
        if (IsAssetImported(metadata.FilePath))
        {
            //TBO_CONSOLE_WARN("Asset already loaded!");
            return GetAsset(m_PathRegistry.at(metadata.FilePath))->Handle;
        }

        // Create asset source and load it
        Ref<Asset> asset = Asset::TryLoad(metadata);
        if (!asset)
        {
            TBO_ENGINE_ERROR("Could not load source asset!");
            return 0;
        }

        metadata.IsLoaded = true;

        RegisterAsset(asset, metadata);

        return asset->Handle;
    }

    bool EditorAssetRegistry::Deserialize()
    {
        auto path = Project::GetAssetRegistryPath();

        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string());
        }
        catch (YAML::Exception e)
        {
            TBO_ENGINE_ERROR(e.what());
            return false;
        }

        if (!data["AssetRegistry"])
            return false;

        auto assets = data["AssetRegistry"];
        if (assets)
        {
            TBO_ENGINE_TRACE("Asset Registry: ");
            for (auto asset : assets)
            {
                AssetHandle handle = asset["Asset"].as<u64>();
                std::string path = asset["Path"].as<std::string>();
                std::string stringAssetType = asset["Type"].as<std::string>();
                AssetType assetType = Asset::DestringifyAssetType(stringAssetType);

                auto& metadata = m_AssetRegistry[handle];
                metadata.FilePath = path;
                metadata.Type = assetType;
                m_PathRegistry[metadata.FilePath] = handle;

                TBO_ENGINE_TRACE("Asset: {}", (u64)handle);
                TBO_ENGINE_TRACE("   Type: {}", stringAssetType);
                TBO_ENGINE_TRACE("   Path: {}", path);
            }
        }

        return true;
    }

    bool EditorAssetRegistry::Serialize()
    {
        auto path = Project::GetAssetRegistryPath();

        YAML::Emitter out;
        {
            out << YAML::BeginMap;

            // Serialize asset registry
            out << YAML::Key << "AssetRegistry" << YAML::Value;
            out << YAML::BeginSeq;
            for (auto& [handle, metadata] : m_AssetRegistry)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Asset" << YAML::Value << handle;
                out << YAML::Key << "Path" << YAML::Value << metadata.FilePath.generic_string();
                out << YAML::Key << "Type" << YAML::Value << Asset::StringifyAssetType(metadata.Type);
                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
            out << YAML::EndMap;
        }

        std::ofstream stream(path);
        if (!stream)
            return false;

        stream << out.c_str();

        // ... and serialize all loaded assets, if they also have some unique aspects that needs serialization
        for (auto& [handle, asset] : m_LoadedAssets)
        {
            Asset::Serialize(m_AssetRegistry.at(handle), asset);
        }

        TBO_ENGINE_INFO("Asset Registry serialized!");

        return true;
    }

    bool EditorAssetRegistry::IsAssetImported(const std::filesystem::path& filepath) const
    {
        return m_PathRegistry.find(filepath) != m_PathRegistry.end();
    }

    AssetMetadata EditorAssetRegistry::CreateMetadata(const std::filesystem::path& filepath)
    {
        AssetMetadata metadata;
        metadata.Type = Utils::GetAssetTypeFromExtension(filepath.extension());
        metadata.FilePath = Utils::GetMetadataPath(filepath);
        return metadata;
    }

    void EditorAssetRegistry::RegisterAsset(const Ref<Asset>& asset, const AssetMetadata& metadata)
    {
        m_LoadedAssets[asset->Handle] = asset;
        m_AssetRegistry[asset->Handle] = metadata;
        m_PathRegistry[metadata.FilePath] = asset->Handle;
    }

    AssetHandle EditorAssetRegistry::GetHandleFromPath(const std::filesystem::path& filepath)
    {
        auto assetsPath = Project::GetAssetsPath();
        auto path = filepath.is_absolute() ? std::filesystem::relative(filepath, assetsPath) : filepath;
        auto it = m_PathRegistry.find(path);
        return it != m_PathRegistry.end() ? it->second : AssetHandle(0);
    }

    bool EditorAssetRegistry::RemoveAsset(AssetHandle handle)
    {
        if (!IsAssetHandleValid(handle))
            return false;

        m_PathRegistry.erase(GetAssetMetadata(handle).FilePath);
        m_AssetRegistry.erase(handle);
        m_LoadedAssets.erase(handle);

        return true;
    }

}
