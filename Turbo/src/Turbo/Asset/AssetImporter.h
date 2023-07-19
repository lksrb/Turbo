#pragma once

#include "Asset.h"
#include "AssetRegistryBase.h"
#include "AssetSerializer.h"

#include "Turbo/Core/Scopes.h"

namespace Turbo
{
    // Importing asset means:
    //    - Asset file can be imported to engine via ContentBrowser (regular file types like .png, .jpg, etc...)
    //    - When you import an asset, it will be registered in AssetRegistry 

    class AssetImporter
    {
    public:
        static bool Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset = nullptr);
        static Ref<Asset> TryLoad(const AssetMetadata& metadata);
    private:
        AssetImporter();

        Scope<AssetSerializer> m_Serializers[AssetType_Count];

        static AssetImporter s_AssetImporter;
    };

}
