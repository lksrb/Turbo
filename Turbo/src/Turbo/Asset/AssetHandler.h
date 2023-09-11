#pragma once

#include "Asset.h"

namespace Turbo {

    class TBO_NOVTABLE AssetHandler
    {
    public:
        virtual bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const = 0;
        virtual Ref<Asset> TryLoad(const AssetMetadata& metadata) = 0;
    };

    class Texture2DHandler : public AssetHandler
    {
    public:
        bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
        Ref<Asset> TryLoad(const AssetMetadata& metadata) override;
    };

    class MeshSourceHandler : public AssetHandler
    {
    public:
        bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override { return true; }
        Ref<Asset> TryLoad(const AssetMetadata& metadata) override;
    };

    class StaticMeshHandler : public AssetHandler
    {
    public:
        bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
        Ref<Asset> TryLoad(const AssetMetadata& metadata) override;
    };

    class PrefabHandler : public AssetHandler
    {
    public:
        bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
        Ref<Asset> TryLoad(const AssetMetadata& metadata) override;
    };

    class MaterialAssetHandler : public AssetHandler
    {
    public:
        bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
        Ref<Asset> TryLoad(const AssetMetadata& metadata) override;
    };
}
