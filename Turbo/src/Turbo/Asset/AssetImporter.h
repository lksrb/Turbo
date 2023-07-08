#pragma once

#include "Asset.h"
#include "AssetRegistryBase.h"
#include "AssetSerializer.h"

namespace Turbo
{
    // Importing asset means:
    //    - Asset file can be imported to engine via ContentBrowser (regular file types like .png, .jpg, etc...)
    //    - When you import an asset, it will be registered in AssetRegistry 

    class AssetImporter
    {
    public:
        static bool Serialize(AssetHandle handle, const AssetMetadata& metadata);
        static Ref<Asset> TryLoad(const AssetMetadata& metadata);
    private:
        AssetImporter();

        Ref<AssetSerializer> m_Serializers[AssetType_Count];

        static AssetImporter s_AssetImporter;
    };

}
