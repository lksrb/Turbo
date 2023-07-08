#pragma once

#include "Turbo/Core/UUID.h"

namespace Turbo
{
    using AssetHandle = UUID;

    // !order dependent
    enum AssetType : u32
    {
        AssetType_Texture2D = 0,
        //AssetType_StaticMesh,
        //Audio,

        AssetType_Count //  Also serves purpose as invalid value
    };

    class Asset
    {
    public:
        virtual AssetType GetAssetType() const = 0;

        static const char* StringifyAssetType(AssetType type);
        static AssetType StringToAssetType(std::string_view type);

        AssetHandle Handle; // Generates ID
    };

    struct AssetMetadata
    {
        std::filesystem::path FilePath;
        AssetType Type = AssetType_Count;
    };
}
