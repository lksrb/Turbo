#include "tbopch.h"
#include "AssetSerializer.h"

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

	bool StaticMeshSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
        // TODO: Figure out what should be serialized
        return true;
	}

	Ref<Asset> StaticMeshSerializer::TryLoad(const AssetMetadata& metadata)
	{
        auto path = Project::GetAssetsPath() / metadata.FilePath;
        Ref<StaticMesh> result = StaticMesh::Create(path.string());
        if (!result->IsLoaded())
            return nullptr;

        return result;

	}

}
