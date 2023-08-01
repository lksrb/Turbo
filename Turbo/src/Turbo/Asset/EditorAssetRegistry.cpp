#include "tbopch.h"
#include "EditorAssetRegistry.h"

#include "Turbo/Core/FileSystem.h"
#include "Turbo/Solution/Project.h"

#include "Turbo/Renderer/Texture.h"

#include <yaml-cpp/yaml.h>

namespace Turbo
{
    namespace Utils
    {
        static AssetType GetAssetTypeFromExtension(const std::filesystem::path& extension)
        {
            if (extension == ".png")   return AssetType_Texture2D;
            if (extension == ".fbx" || extension == ".gltf")   return AssetType_MeshSource;
            if (extension == ".tmesh") return AssetType_StaticMesh;

            TBO_ENGINE_ASSERT(false, "Unsupported extension!");

            return AssetType_Count;
        }

        static std::filesystem::path GetSecondaryAssetExtension(AssetType secondaryAssetType)
        {
            // Those are special cases where we want to have separate file to refence an asset

            switch (secondaryAssetType)
            {
                case AssetType_Texture2D:  return ".ttex"; // TODO:
                case AssetType_StaticMesh: return ".tmesh";
            }

            TBO_ENGINE_ASSERT(false, "Unknown primrary asset type!");
            return "";
        }


        static AssetClassification GetClassification(AssetType type)
        {
            switch (type)
            {
                case AssetType_Texture2D:  return AssetClassification_Secondary;
                case AssetType_MeshSource: return AssetClassification_Primary;
                case AssetType_StaticMesh: return AssetClassification_Secondary;
            }

            TBO_ENGINE_ASSERT(false);

            return AssetClassification_None;
        }

        static std::filesystem::path GetMetadataPath(const std::filesystem::path& filepath)
        {
            auto assetsPath = Project::GetAssetsPath();
            auto metadataPath = std::filesystem::relative(filepath, assetsPath);
            return metadataPath;
        }

        static std::filesystem::path GetSecondaryAssetPath(const std::filesystem::path& filepath, AssetType secondaryAssetType)
        {
            auto extension = Utils::GetSecondaryAssetExtension(secondaryAssetType);
            auto metadataPath = Utils::GetMetadataPath(filepath);
            return FileSystem::ReplaceExtension(metadataPath, extension);
        }

        static AssetType GetSecondaryAssetTypeFromPrimary(AssetType primary)
        {
            switch (primary)
            {
                case AssetType_Texture2D:  return AssetType_Texture2D;
                case AssetType_MeshSource: return AssetType_StaticMesh;
            }

            TBO_ENGINE_ASSERT(false);

            return AssetType_Count;
        }
    }

    static std::array<AssetMetadata, 2> s_DefaultAssetPaths = {
        AssetMetadata{ "Meshes/Default/Cube.fbx", AssetType_MeshSource },
        AssetMetadata{ "Meshes/Default/Sphere.fbx", AssetType_MeshSource }
    };

    EditorAssetRegistry::~EditorAssetRegistry()
    {
        Serialize();
    }

    void EditorAssetRegistry::Init()
    {
        Deserialize();
        LoadDefaultAssets();
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
            m_LoadedAssets[asset->Handle] = asset;
            m_AssetRegistry[asset->Handle] = metadata;
        }

        return m_LoadedAssets.at(handle);
    }

    const AssetMetadata& EditorAssetRegistry::GetAssetMetadata(AssetHandle handle) const
    {
        static AssetMetadata s_NullMetadata;

        if (!IsAssetHandleValid(handle))
            return s_NullMetadata;

        return m_AssetRegistry.at(handle);
    }

    void EditorAssetRegistry::ImportAsset(const std::filesystem::path& filepath)
    {
        // Create metadata specific to the asset type
        AssetMetadata metadata;
        metadata.Type = Utils::GetAssetTypeFromExtension(filepath.extension());
        metadata.FilePath = Utils::GetMetadataPath(filepath);
        metadata.Classification = Utils::GetClassification(metadata.Type);

        // Primary asset - Does not appear on disk, is only registered in asset registry file, must be loaded first
        // Secondary asset - Is present on disk aswell as is registered in asset registry file, must be loaded second

        if (metadata.Classification == AssetClassification_Primary)
        {
            // Create primary asset and load it
            Ref<Asset> primaryAsset = Asset::TryLoad(metadata);
            TBO_ENGINE_ASSERT(primaryAsset);
            if (primaryAsset)
            {
                metadata.IsLoaded = true;
                m_LoadedAssets[primaryAsset->Handle] = primaryAsset;
                m_AssetRegistry[primaryAsset->Handle] = metadata;
            }

            // Create secondary asset and load it with the primary asset 
            AssetMetadata secondaryMetadata;
            secondaryMetadata.Type = Utils::GetSecondaryAssetTypeFromPrimary(metadata.Type);
            secondaryMetadata.FilePath = Utils::GetSecondaryAssetPath(filepath, secondaryMetadata.Type);
            secondaryMetadata.Classification = AssetClassification_Secondary;

            // Create secondary asset and load it
            auto secondaryAsset = Asset::Create(secondaryMetadata, primaryAsset);
            TBO_ENGINE_ASSERT(secondaryAsset);

            if (secondaryAsset)
            {
                secondaryMetadata.IsLoaded = true;
                m_LoadedAssets[secondaryAsset->Handle] = secondaryAsset;
                m_AssetRegistry[secondaryAsset->Handle] = secondaryMetadata;
            }

            // And serialize it
            Asset::Serialize(secondaryMetadata, secondaryAsset);

        }
        else // metadata.Classification == AssetClassification_Secondary
        {
            // TODO:
            TBO_ENGINE_ASSERT(false);
            Ref<Asset> secondaryAsset = Asset::TryLoad(metadata);
            //Ref<Asset> primaryAsset = GetAsset()
            // Try to get primary asset
            // If the primary asset is present but not load it, load it aswell
            // Create secondary asset
        }
    }

    void EditorAssetRegistry::LoadDefaultAssets()
    {
        for (auto& metadata : s_DefaultAssetPaths)
        {
            Ref<Asset> asset = Asset::TryLoad(metadata);
            if (asset)
            {
                metadata.IsLoaded = true;
                m_MemoryOnlyAssets[asset->Handle] = asset;
            }
        }
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
            TBO_ENGINE_TRACE("Asset Registry deserialization: ");
            for (auto& asset : assets)
            {
                AssetHandle handle = asset["Asset"].as<u64>();
                std::string path = asset["Path"].as<std::string>();
                std::string stringAssetType = asset["Type"].as<std::string>();
                AssetType assetType = Asset::StringToAssetType(stringAssetType);

                auto& metadata = m_AssetRegistry[handle];
                metadata.FilePath = path;
                metadata.Type = assetType;
                metadata.Classification = Utils::GetClassification(metadata.Type);

                TBO_ENGINE_TRACE("Asset: {}", (u64)handle);
                TBO_ENGINE_TRACE("   Type: {}", stringAssetType);
                TBO_ENGINE_TRACE("   Path: {}", path);
            }

            // Load primary assets
            for (auto& [handle, metadata] : m_AssetRegistry)
            {
                if (metadata.Classification == AssetClassification_Primary)
                    m_LoadedAssets[handle] = Asset::TryLoad(metadata);
            }

            // Load secondary assets
            for (auto& [handle, metadata] : m_AssetRegistry)
            {
                if (metadata.Classification == AssetClassification_Secondary)
                    m_LoadedAssets[handle] = Asset::TryLoad(metadata);
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

        TBO_ENGINE_INFO("Asset Registry serialized!");

        return true;
    }

}
