#pragma once

#include "Turbo/Core/UUID.h"
#include "Turbo/Core/Ref.h"

#include <filesystem>

namespace Turbo {

    using AssetHandle = UUID;

    enum AssetType : u32
    {
        AssetType_Texture2D = 0,
        AssetType_MeshSource = 1,
        AssetType_StaticMesh = 2,
        AssetType_Prefab = 3,
        AssetType_MaterialAsset = 4,

        // Also serves purpose as invalid value
        AssetType_Count = 5
    };

    struct AssetMetadata
    {
        std::filesystem::path FilePath;
        AssetType Type = AssetType_Count;
        bool IsLoaded = false;

        AssetMetadata() = default;

        explicit AssetMetadata(const std::filesystem::path& filepath, AssetType type, bool isLoaded)
            : FilePath(filepath), Type(type), IsLoaded(isLoaded)
        {
        }
    };

    class Asset : public RefCounted
    {
    public:
        virtual ~Asset() = default;
        virtual AssetType GetAssetType() const = 0;

        AssetHandle Handle; // Generates unique ID
    public:
        static const char* StringifyAssetType(AssetType type);
        static AssetType DestringifyAssetType(std::string_view type);

        static bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset = nullptr);
        static Ref<Asset> TryLoad(const AssetMetadata& metadata);
    };
}
