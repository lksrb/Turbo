#include "tbopch.h"
#include "AssetHandler.h"

#include "AssetManager.h"

#include "Turbo/Core/FileSystem.h"
#include "Turbo/Renderer/Texture.h"
#include "Turbo/Solution/Project.h"
#include "Turbo/Renderer/Mesh.h"

#include "Turbo/Scene/SceneSerializer.h"
#include "Turbo/Scene/Prefab.h"

#include <yaml-cpp/yaml.h>

namespace Turbo {

    namespace Utils {

        static const char* StringifyImageFilter(ImageFilter filter)
        {
            static const char* s_FilterTypeStrings[] = { "Nearest", "Linear" };

            return s_FilterTypeStrings[filter];
        }

        static const char* StringifyImageFormat(ImageFormat format)
        {
            static std::map<ImageFormat, const char*> s_FormatTypeStrings
            {
                { ImageFormat_RGBA_Unorm, "RGBA_Unorm" },
                { ImageFormat_RGBA_SRGB, "RGBA_SRGB" }
            };

            return s_FormatTypeStrings.at(format);
        }

        static ImageFilter GetImageFilterFromString(const std::string& filter)
        {
            static std::map<std::string_view, ImageFilter> s_FilterStringTypes
            {
                { "Nearest", ImageFilter_Nearest },
                { "Linear", ImageFilter_Linear }
            };

            return s_FilterStringTypes.at(filter.c_str());
        }

        static ImageFormat GetImageFormatFromString(const std::string& format)
        {
            static std::map<std::string_view, ImageFormat> s_FormatStringTypes
            {
                { "RGBA_SRGB", ImageFormat_RGBA_SRGB },
                { "RGBA_Unorm", ImageFormat_RGBA_Unorm }
            };

            return s_FormatStringTypes.at(format.c_str());
        }
    }

    bool Texture2DHandler::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
    {
        return true;
        auto path = Project::GetAssetsPath() / metadata.FilePath;

        // Default values
        ImageFilter filter = ImageFilter_Linear;
        ImageFormat format = ImageFormat_RGBA_SRGB;

        if (asset)
        {
            auto& config = asset.As<Texture2D>()->GetConfig();
            filter = config.Filter;
            format = config.Format;
        }

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "TexturePath" << YAML::Value << FileSystem::ReplaceExtension(metadata.FilePath, ".png").generic_string();
        out << YAML::Key << "TextureFilter" << YAML::Value << Utils::StringifyImageFilter(filter);
        out << YAML::Key << "TextureFormat" << YAML::Value << Utils::StringifyImageFormat(format);
        out << YAML::EndMap;

        std::ofstream stream(path);
        if (!stream)
            return false;

        stream << out.c_str();

        return true;
    }
    Ref<Asset> Texture2DHandler::TryLoad(const AssetMetadata& metadata)
    {
        auto assetsPath = Project::GetAssetsPath();
        auto path = assetsPath / metadata.FilePath;

        Ref<Texture2D> result = Texture2D::Create(path.string());
        if (!result->IsLoaded())
            return nullptr;

        return result;

#if 0
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string());
        }
        catch (YAML::Exception e)
        {
            TBO_ENGINE_ERROR(e.what());
            return nullptr;
        }

        auto texturePath = assetsPath / data["TexturePath"].as<std::string>();

        Texture2D::Config config;
        config.Path = texturePath.generic_string();
        config.Filter = Utils::GetImageFilterFromString(data["TextureFilter"].as<std::string>());
        config.Format = Utils::GetImageFormatFromString(data["TextureFormat"].as<std::string>());
        Ref<Texture2D> result = Texture2D::Create(config);
        if (!result->IsLoaded())
            return nullptr;
#endif

        return result;
    }

    Ref<Asset> MeshSourceHandler::TryLoad(const AssetMetadata& metadata)
    {
        auto path = Project::GetAssetsPath() / metadata.FilePath;
        Ref<MeshSource> meshSource = Ref<MeshSource>::Create(path.string());
        if (!meshSource->IsLoaded())
            return nullptr;

        return meshSource;
    }

    bool StaticMeshHandler::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
    {
        auto path = Project::GetAssetsPath() / metadata.FilePath;

        auto staticMesh = asset.As<StaticMesh>();

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Mesh" << YAML::Value;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "MeshSource" << YAML::Value << staticMesh->GetMeshSource()->Handle;
            out << YAML::Key << "SubmeshIndices" << YAML::Value << YAML::Flow << staticMesh->GetSubmeshIndices();
            out << YAML::EndMap;
        }
        out << YAML::EndMap;

        std::ofstream stream(path);
        if (!stream)
            return false;

        stream << out.c_str();

        return true;
    }

    Ref<Asset> StaticMeshHandler::TryLoad(const AssetMetadata& metadata)
    {
        auto path = Project::GetAssetsPath() / metadata.FilePath;

        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string());
        }
        catch (YAML::Exception e)
        {
            TBO_ENGINE_ERROR(e.what());
            return nullptr;
        }

        if (!data["Mesh"])
        {
            TBO_ENGINE_ERROR("File does not contain mesh data!");
            return nullptr;
        }

        auto meshData = data["Mesh"];
        AssetHandle meshSourceHandle = meshData["MeshSource"].as<u64>();

        // Get asset
        Ref<MeshSource> meshSource = AssetManager::GetAsset<MeshSource>(meshSourceHandle);

        if (!meshSource)
        {
            TBO_ENGINE_ERROR("MeshSource is missing!");
            return nullptr;
        }

        return Ref<StaticMesh>::Create(meshSource, meshData["SubmeshIndices"].as<std::vector<u32>>());
    }

    bool PrefabHandler::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
    {
        Ref<Prefab> prefab = asset.As<Prefab>();

        std::ofstream fout(Project::GetAssetsPath() / metadata.FilePath);

        if (!fout)
        {
            TBO_ENGINE_ERROR("Could not serialize prefab! ({})", metadata.FilePath);
            return false;
        }

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Prefab" << YAML::Value << prefab->Handle;
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        auto entities = prefab->m_Scene->m_Registry.view<IDComponent>();
        for (auto it = entities.rbegin(); it != entities.rend(); ++it)
        {
            Entity entity(*it, prefab->m_Scene.Get());
            SceneSerializer::SerializeEntity(out, entity);
        }

        out << YAML::EndSeq;
        out << YAML::EndMap;

        fout << out.c_str();

        return true;
    }

    Ref<Asset> PrefabHandler::TryLoad(const AssetMetadata& metadata)
    {
        auto path = Project::GetAssetsPath() / metadata.FilePath;

        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string());
        }
        catch (YAML::Exception e)
        {
            TBO_ENGINE_ERROR(e.what());
            return nullptr;
        }

        if (!data["Prefab"])
        {
            TBO_ENGINE_ERROR("File does not contain prefab data!");
            return nullptr;
        }

        AssetHandle prefabHandle = data["Prefab"].as<u64>();
        TBO_ENGINE_TRACE("Deserializing prefab '{0}'", prefabHandle);

        auto entities = data["Entities"];

        Ref<Prefab> prefab;
        if (entities)
        {
            prefab = Ref<Prefab>::Create(entities.size());
            prefab->Handle = prefabHandle;

            // Deserialize prefab entity
            auto entityNode = entities.begin();
            u64 uuid = (*entityNode)["Entity"].as<u64>();
            prefab->m_Entity = prefab->m_Scene->CreateEntityWithUUID(uuid);
            SceneSerializer::DeserializeEntity(*entityNode, prefab->m_Entity);

            // Deserialize the rest
            while (++entityNode != entities.end())
            {
                u64 uuid = (*entityNode)["Entity"].as<u64>();
                Entity deserializedEntity = prefab->m_Scene->CreateEntityWithUUID(uuid);
                SceneSerializer::DeserializeEntity(*entityNode, deserializedEntity);
            }
        }

        return prefab;
    }

}
