#include "tbopch.h"
#include "AssetImporter.h"

#include "AssetSerializer.h"

namespace Turbo
{
    AssetImporter AssetImporter::s_AssetImporter;

    AssetImporter::AssetImporter()
    {
        m_Serializers[AssetType_Texture2D] = Ref<Texture2DSerializer>::Create();
    }

    bool AssetImporter::Serialize(AssetHandle handle, const AssetMetadata& metadata)
    {
        return s_AssetImporter.m_Serializers[metadata.Type]->Serialize(handle, metadata);
    }

    Ref<Asset> AssetImporter::TryLoad(const AssetMetadata& metadata)
    {
        return s_AssetImporter.m_Serializers[metadata.Type]->TryLoad(metadata);
    }

}
