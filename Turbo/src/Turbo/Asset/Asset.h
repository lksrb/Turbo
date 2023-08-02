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
        AssetType_MeshSource = 1,
        AssetType_StaticMesh = 2,

        // Also serves purpose as invalid value
        AssetType_Count = 3
    };

    enum DefaultAsset : u32 {
        DefaultAsset_Cube = 0,
        DefaultAsset_Sphere = 1,

        DefaultAsset_Count = 2
    };

    enum AssetFlag : u32
    {
        AssetFlag_None = 0,
        AssetFlag_Loaded = TBO_BIT(0), // Is asset loaded
        AssetFlag_Default = TBO_BIT(1) // Is asset a default asset
    };

    struct AssetMetadata
    {
        std::filesystem::path FilePath;
        AssetType Type = AssetType_Count;
        bool IsLoaded = false;
    };

    class Asset
    {
    public:
        using AssetFlags = u32;
        virtual ~Asset() = default;
        virtual AssetType GetAssetType() const = 0;
        void SetFlags(AssetFlag flags, bool enable = true);

        AssetFlags Flags;
        AssetHandle Handle; // Generates ID
    public:
        static const char* StringifyAssetType(AssetType type);
        static AssetType StringToAssetType(std::string_view type);

        static bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset = nullptr);
        static Ref<Asset> TryLoad(const AssetMetadata& metadata);
        static Ref<Asset> Create(const AssetMetadata& metadata, const Ref<Asset>& sourceAsset);
    };
}
