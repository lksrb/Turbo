#include "tbopch.h"
#include "Asset.h"

namespace Turbo
{
    const char* Asset::StringifyAssetType(AssetType type)
    {
        static const char* s_StringifiedAssetTypeMap[AssetType_Count] =
        {
            "Texture2D"
        };

        return s_StringifiedAssetTypeMap[type];
    }

    AssetType Asset::StringToAssetType(std::string_view type)
    {
        static std::unordered_map<std::string_view, AssetType> s_AssetTypeMap =
        {
            { "Texture2D", AssetType_Texture2D }
        };

        return s_AssetTypeMap.at(type);
    }

}
