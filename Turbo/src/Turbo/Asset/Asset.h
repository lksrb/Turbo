#pragma once

#include "Turbo/Core/UUID.h"

namespace Turbo
{
    using AssetHandle = UUID;

    enum class AssetType : u32
    {
        Texture = 0,
        Audio,
        StaticMesh
    };

    class Asset
    {
    public:
        virtual AssetType GetAssetType() const = 0;

        AssetHandle Handle; // Generates ID
    };
}
