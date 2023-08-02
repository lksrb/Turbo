#include "tbopch.h"
#include "AssetSerializer.h"

#include "AssetManager.h"

#include "Turbo/Core/FileSystem.h"
#include "Turbo/Renderer/Texture.h"
#include "Turbo/Solution/Project.h"
#include "Turbo/Renderer/Mesh.h"

#include <yaml-cpp/yaml.h>

namespace Turbo
{
    namespace Utils
    {
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

    bool Texture2DSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
    {
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
    Ref<Asset> Texture2DSerializer::TryLoad(const AssetMetadata& metadata)
    {
        auto assetsPath = Project::GetAssetsPath();
        auto path = assetsPath / metadata.FilePath;

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

        auto texturePath = assetsPath / data["TexturePath"].as<std::string>();

        Texture2D::Config config;
        config.Path = texturePath.generic_string();
        config.Filter = Utils::GetImageFilterFromString(data["TextureFilter"].as<std::string>());
        config.Format = Utils::GetImageFormatFromString(data["TextureFormat"].as<std::string>());
        Ref<Texture2D> result = Texture2D::Create(config);
        if (!result->IsLoaded())
            return nullptr;

        return result;
    }

    bool MeshSourceSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
    {
        // TODO: Figure out what should be serialized
        return true;
    }

    Ref<Asset> MeshSourceSerializer::TryLoad(const AssetMetadata& metadata)
    {
        auto path = Project::GetAssetsPath() / metadata.FilePath;
        Ref<MeshSource> meshSource = Ref<MeshSource>::Create(path.string());
        if (!meshSource->IsLoaded())
            return nullptr;

        return meshSource;
    }

    Ref<Asset> StaticMeshSerializer::Create(const AssetMetadata& metadata, const Ref<Asset>& sourceAsset) const
    {
        auto path = Project::GetAssetsPath() / metadata.FilePath;
        Ref<MeshSource> meshSource = sourceAsset.As<MeshSource>();
        return Ref<StaticMesh>::Create(meshSource);
    }

    bool StaticMeshSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
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

    Ref<Asset> StaticMeshSerializer::TryLoad(const AssetMetadata& metadata)
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
            return false;
        }

        if (!data["Mesh"])
        {
            TBO_ENGINE_ERROR("File does not contain mesh data!");
            return false;
        }

        auto meshData = data["Mesh"];
        AssetHandle meshSourceHandle = meshData["MeshSource"].as<u64>();

        // NOTE: This is a bit problematic since we deserialize assets in sequence.
        // Could be solved by loading primary assets first and then load secondary assets
        Ref<MeshSource> meshSource = AssetManager::GetAsset<MeshSource>(meshSourceHandle);

        if (!meshSource)
        {
            TBO_ENGINE_ERROR("MeshSource is missing!");
            return nullptr;
        }

        auto submeshIndices = meshData["SubmeshIndices"];
        std::vector<u32> indices;
        indices.reserve(submeshIndices.size());
        for (auto& index : submeshIndices)
        {
            indices.emplace_back(index.as<u32>());
        }

        TBO_ENGINE_ASSERT(indices.size() != 0);

        return Ref<StaticMesh>::Create(meshSource, indices);
    }

}
