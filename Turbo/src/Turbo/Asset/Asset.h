#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Core/UUID.h"

namespace Turbo {

    using AssetHandle = UUID;

    // !order dependent
    enum AssetType : u32 {
        AssetType_Texture2D = 0,
        AssetType_MeshSource = 1,
        AssetType_StaticMesh = 2,

        // Also serves purpose as invalid value
        AssetType_Count = 3
    };

    struct AssetMetadata {
        std::filesystem::path FilePath;
        AssetType Type = AssetType_Count;
        bool IsLoaded = false;
    };

    class Asset : public RefCounted {
    public:
        using AssetFlags = u32;
        virtual ~Asset() = default;
        virtual AssetType GetAssetType() const = 0;

        AssetHandle Handle; // Generates unique ID
    public:
        static const char* StringifyAssetType(AssetType type);
        static AssetType StringToAssetType(std::string_view type);

        static bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset = nullptr);
        static Ref<Asset> TryLoad(const AssetMetadata& metadata);
    };
}
