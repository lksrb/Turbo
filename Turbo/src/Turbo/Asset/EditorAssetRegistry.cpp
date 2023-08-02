#include "tbopch.h"
#include "EditorAssetRegistry.h"

#include "Turbo/Core/FileSystem.h"
#include "Turbo/Solution/Project.h"

#include "Turbo/Renderer/Texture.h"

#include <yaml-cpp/yaml.h>

namespace Turbo {

    namespace Utils {

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

        static AssetType GetAssetTypeFromSource(AssetType primary)
        {
            switch (primary)
            {
                case AssetType_Texture2D:  return AssetType_Texture2D;
                case AssetType_MeshSource: return AssetType_StaticMesh;
            }

            TBO_ENGINE_ASSERT(false);

            return AssetType_Count;
        }

        static std::filesystem::path GetPathFromAssetType(std::string_view assetName, AssetType assetType)
        {
            if (assetType == AssetType_StaticMesh) return fmt::format("Meshes/{}.tmesh", assetName);

            TBO_ENGINE_ASSERT(false, "Could not generate path from this assetType!");

            return "";
        }

        static DefaultAsset GetDefaultAssetFromString(std::string_view defaultAsset)
        {
            if (defaultAsset == "Cube") return DefaultAsset_Cube;
            if (defaultAsset == "Sphere") return DefaultAsset_Sphere;

            TBO_ENGINE_ASSERT(false, "Unknown default asset!");

            return DefaultAsset_Count;
        }

        static AssetMetadata CreateMetadata(const std::filesystem::path& filepath)
        {
            AssetMetadata metadata;
            metadata.Type = GetAssetTypeFromExtension(filepath.extension());
            metadata.FilePath = GetMetadataPath(filepath);
            return metadata;
        }
    }

    EditorAssetRegistry::~EditorAssetRegistry()
    {
        Serialize();
    }

    void EditorAssetRegistry::Init()
    {
        LoadDefaultAssets();
        Deserialize();
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
        if (!IsAssetHandleValid(handle) && m_DefaultAssetMap.find(handle) == m_DefaultAssetMap.end())
            return nullptr;

        if (!IsAssetLoaded(handle))
        {
            auto& metadata = m_AssetRegistry.at(handle);
            Ref<Asset> asset = Asset::TryLoad(metadata);
            if (!asset)
                return nullptr;

            metadata.IsLoaded = true;
            asset->Handle = handle;
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
        // Create metadata
        AssetMetadata sourceMetadata = Utils::CreateMetadata(filepath);

        // Create asset source and load it
        Ref<Asset> sourceAsset = Asset::TryLoad(sourceMetadata);
        if (!sourceAsset)
        {
            TBO_ENGINE_ERROR("Could not load source asset!");
            return;
        }

        sourceMetadata.IsLoaded = true;
        m_LoadedAssets[sourceAsset->Handle] = sourceAsset;
        m_AssetRegistry[sourceAsset->Handle] = sourceMetadata;

        // Create asset and load it with the primary asset 
        AssetMetadata metadata;
        metadata.Type = Utils::GetAssetTypeFromSource(sourceMetadata.Type);
        metadata.FilePath = Utils::GetSecondaryAssetPath(filepath, metadata.Type);

        // Create asset and load it
        Ref<Asset> asset = Asset::Create(metadata, sourceAsset);
        if (!asset)
        {
            TBO_ENGINE_ERROR("Could not create source asset!");
            return;
        }

        metadata.IsLoaded = true;
        m_LoadedAssets[asset->Handle] = asset;
        m_AssetRegistry[asset->Handle] = metadata;

        // And serialize it
        Asset::Serialize(metadata, asset);
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

        if (!data["AssetDefaults"])
            return false;

        auto defaults = data["AssetDefaults"];
        TBO_ENGINE_ASSERT(defaults); // TODO: If defaults are not present, load them?

        for (auto node : defaults)
        {
            AssetHandle handle = node["Asset"].as<u64>();
            DefaultAsset defaultAsset = Utils::GetDefaultAssetFromString(node["Name"].as<std::string>());
            Ref<Asset> asset = m_DefaultAssetRegistry[defaultAsset];

            asset->Handle = handle;
            m_LoadedAssets[handle] = asset;
            m_DefaultAssetMap[handle] = asset;
        }


        if (!data["AssetRegistry"])
            return false;

        auto assets = data["AssetRegistry"];
        if (assets)
        {
            TBO_ENGINE_TRACE("Asset Registry deserialization: ");
            for (auto asset : assets)
            {
                AssetHandle handle = asset["Asset"].as<u64>();
                std::string path = asset["Path"].as<std::string>();
                std::string stringAssetType = asset["Type"].as<std::string>();
                AssetType assetType = Asset::StringToAssetType(stringAssetType);

                auto& metadata = m_AssetRegistry[handle];
                metadata.FilePath = path;
                metadata.Type = assetType;

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
            // Serialize default asset first with their handles
            // This is important since we dont want duplicate mesh sources when creating same meshes unless client explicitly creates one
            out << YAML::Key << "AssetDefaults" << YAML::Value;

            out << YAML::BeginSeq;
            for (u32 defaultAsset = 0; defaultAsset < DefaultAsset_Count; defaultAsset++)
            {
                static std::array<const char*, DefaultAsset_Count> s_StringifiedDefaultAsset =
                {
                    "Cube",
                    "Sphere"
                };
                out << YAML::BeginMap;
                out << YAML::Key << "Asset" << YAML::Value << m_DefaultAssetRegistry[defaultAsset]->Handle;
                out << YAML::Key << "Name" << YAML::Value << s_StringifiedDefaultAsset[defaultAsset];
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;

            // Serialize main asset registry
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

    /**
     * Only creates default assets so later in Deserialize function we can add them into loaded asset with deserialized handle ID
     */
    void EditorAssetRegistry::LoadDefaultAssets()
    {
        // TODO: Embed default assets?
        static std::array<const char*, DefaultAsset_Count> s_DefaultAsset = {
            "Meshes/Default/Cube.fbx",
            "Meshes/Default/Sphere.fbx"
        };

        for (u32 i = 0; i < DefaultAsset_Count; i++)
        { 
            AssetMetadata metadata;
            metadata.FilePath = s_DefaultAsset[i];
            metadata.Type = AssetType_MeshSource;
            Ref<Asset> asset = Asset::TryLoad(metadata);
            if (!asset)
            {
                TBO_ENGINE_ERROR("Could not load {}!", s_DefaultAsset[i]);
                continue;
            }
            asset->SetFlags(AssetFlag_Default);
            asset->SetFlags(AssetFlag_Loaded);
            metadata.IsLoaded = true;
            m_DefaultAssetRegistry[i] = asset;
        }
    }

    Ref<Asset> EditorAssetRegistry::CreateFromDefaultAsset(std::string_view assetName, DefaultAsset defaultAsset)
    {
        if (defaultAsset >= DefaultAsset_Count)
        {
            TBO_ENGINE_ERROR("Invalid default asset index!");
            return nullptr;
        }

        Ref<Asset> primaryAsset = m_DefaultAssetRegistry[defaultAsset];

        // Create secondary asset and load it with the primary asset 
        AssetMetadata secondaryMetadata;
        secondaryMetadata.Type = Utils::GetAssetTypeFromSource(primaryAsset->GetAssetType());
        secondaryMetadata.FilePath = Utils::GetPathFromAssetType(assetName, secondaryMetadata.Type);

        // Create secondary asset and load it
        auto secondaryAsset = Asset::Create(secondaryMetadata, primaryAsset);
        if (!secondaryAsset)
        {
            TBO_ENGINE_ERROR("Could not create asset!");
            return nullptr;
        }

        // And serialize it
        if (!Asset::Serialize(secondaryMetadata, secondaryAsset))
        {
            TBO_ENGINE_ERROR("Could not serialize asset!");
            return nullptr;
        }

        secondaryMetadata.IsLoaded = true;
        m_LoadedAssets[secondaryAsset->Handle] = secondaryAsset;
        m_AssetRegistry[secondaryAsset->Handle] = secondaryMetadata;

        return secondaryAsset;
    }

}
