#pragma once

#include "Asset.h"

#include <filesystem>

namespace Turbo
{
    class AssetManagerBase
    {
    public:
        virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
        virtual Ref<Asset> GetAsset(AssetHandle handle) const = 0;
    };
}
