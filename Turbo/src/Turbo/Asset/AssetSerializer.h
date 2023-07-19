#pragma once

#include "Asset.h"

namespace Turbo
{
    class AssetSerializer
    {
    public:
        virtual bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const = 0;
        virtual Ref<Asset> TryLoad(const AssetMetadata& metadata) = 0;
    };

    class Texture2DSerializer : public AssetSerializer
    {
    public:
        bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
        Ref<Asset> TryLoad(const AssetMetadata& metadata) override;
    };
}
