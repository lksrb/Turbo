#pragma once

#include "Asset.h"

namespace Turbo
{
    class AssetSerializer
    {
    public:
        virtual Ref<Asset> Create(const AssetMetadata& metadata, const Ref<Asset>& primaryAsset) const { return nullptr; };
        virtual bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const = 0;
        virtual Ref<Asset> TryLoad(const AssetMetadata& metadata) = 0;
    };

    class Texture2DSerializer : public AssetSerializer
    {
    public:
        bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
        Ref<Asset> TryLoad(const AssetMetadata& metadata) override;
    };

    class MeshSourceSerializer : public AssetSerializer
    {
    public:
        bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
        Ref<Asset> TryLoad(const AssetMetadata& metadata) override;
    };

    class StaticMeshSerializer : public AssetSerializer
    {
    public:
        Ref<Asset> Create(const AssetMetadata& metadata, const Ref<Asset>& primaryAsset) const override;
        bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
        Ref<Asset> TryLoad(const AssetMetadata& metadata) override;
    };
}
