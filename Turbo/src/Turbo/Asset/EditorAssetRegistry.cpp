#include "tbopch.h"
#include "EditorAssetRegistry.h"

#include "AssetImporter.h"

#include "Turbo/Core/FileSystem.h"
#include "Turbo/Solution/Project.h"

#include "Turbo/Renderer/Texture2D.h"

#include <yaml-cpp/yaml.h>

namespace Turbo
{
    namespace Utils
    {
        static AssetType GetAssetTypeFromExtension(const std::filesystem::path& extension)
        {
            if (extension == ".png") return AssetType_Texture2D;
            if (extension == ".fbx") return AssetType_StaticMesh;

            TBO_ENGINE_ASSERT(false, "Unsupported extension!");

            return AssetType_Count;
        }

        static std::filesystem::path ReplaceExtension(std::filesystem::path filepath)
        {
            auto extension = filepath.extension();

            // Those are special cases where we want to have separate file to refence an asset
            if (extension == ".png") return FileSystem::ReplaceExtension(filepath, ".ttex");

            return filepath;
        }

        static std::filesystem::path GetMetadataPath(const std::filesystem::path& filepath)
        {
            auto assetsPath = Project::GetAssetsPath();
            auto metadataPath = std::filesystem::relative(filepath, assetsPath);
            return ReplaceExtension(metadataPath);
        }
    }

    EditorAssetRegistry::EditorAssetRegistry()
    {
        Deserialize();
    }

    EditorAssetRegistry::~EditorAssetRegistry()
    {
        Serialize();
        //m_LoadedAssets.clear();
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
            // Load asset?
            return nullptr;
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
        // Generates ID
        AssetHandle assetHandle;

        // Create metadata specific to the asset type
        AssetMetadata metadata;
        metadata.Type = Utils::GetAssetTypeFromExtension(filepath.extension());
        metadata.FilePath = Utils::GetMetadataPath(filepath);

        // Serialize data
        TBO_ENGINE_ASSERT(AssetImporter::Serialize(metadata));

        // Load data
        Ref<Asset> asset = AssetImporter::TryLoad(metadata);

        if (asset)
        {
            asset->Handle = assetHandle;
            m_LoadedAssets[assetHandle] = asset;
            m_AssetRegistry[assetHandle] = metadata;
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
                m_LoadedAssets[handle] = AssetImporter::TryLoad(metadata);

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
