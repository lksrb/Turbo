#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Core/UUID.h"

namespace Turbo
{
    using AssetHandle = UUID;

    // !order dependent
    enum AssetType : u32
    {
        AssetType_Texture2D = 0,
        AssetType_MeshSource,
        AssetType_StaticMesh,

        // Also serves purpose as invalid value
        AssetType_Count
    };

    enum AssetClassification : u32
    {
        AssetClassification_None = 0,
        AssetClassification_Primary,
        AssetClassification_Secondary
    };

    enum AssetFlags_ : u32
    {
        AssetFlags_None = 0,
        AssetFlags_Loaded = TBO_BIT(0)
    };

    using AssetFlags = u32;

    struct AssetMetadata
    {
        std::filesystem::path FilePath;
        AssetType Type = AssetType_Count;
        bool IsLoaded = false;
        AssetClassification Classification = AssetClassification_None;
    };

    class Asset
    {
    public:
        virtual ~Asset() = default;
        virtual AssetType GetAssetType() const = 0;
        AssetHandle Handle; // Generates ID
    public:
        static const char* StringifyAssetType(AssetType type);
        static AssetType StringToAssetType(std::string_view type);

        static bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset = nullptr);
        static Ref<Asset> TryLoad(const AssetMetadata& metadata);
        static Ref<Asset> Create(const AssetMetadata& metadata, const Ref<Asset>& primaryAsset);
    };
}
