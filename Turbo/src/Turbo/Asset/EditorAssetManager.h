#pragma once

#include "AssetManagerBase.h"

#include <map>

namespace Turbo
{
    class EditorAssetManager : public AssetManagerBase
    {
        using AssetMap = std::map<AssetHandle, Ref<Asset>>;

    public:
        EditorAssetManager();
        virtual ~EditorAssetManager();

        bool IsAssetHandleValid(AssetHandle handle) const override;
        Ref<Asset> GetAsset(AssetHandle handle) const override;
    private:
        AssetMap m_AssetMap;
    };
}
