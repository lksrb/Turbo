#pragma once

#include "Asset.h"

#include <filesystem>
#include <map>

namespace Turbo {

    class TBO_NOVTABLE AssetRegistryBase : public RefCounted {
    public:
        using AssetMap = std::map<AssetHandle, Ref<Asset>>;
        using AssetRegistry = std::map<AssetHandle, AssetMetadata>;

        virtual ~AssetRegistryBase() = default;

        virtual AssetHandle ImportAsset(const std::filesystem::path& filepath) = 0;
        virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;

        // Tries to load asset from asset registy
        virtual bool IsAssetLoaded(AssetHandle handle) const = 0;
        virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;
        virtual const AssetMetadata& GetAssetMetadata(AssetHandle handle) const = 0;
    };

}
