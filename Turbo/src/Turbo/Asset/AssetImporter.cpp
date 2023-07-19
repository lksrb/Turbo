#include "tbopch.h"
#include "AssetImporter.h"

#include "Turbo/Solution/Project.h"

#include "AssetSerializer.h"

#include <yaml-cpp/yaml.h>

namespace Turbo
{
    AssetImporter AssetImporter::s_AssetImporter;

    AssetImporter::AssetImporter()
    {
        m_Serializers[AssetType_Texture2D] = CreateScope<Texture2DSerializer>();
    }

    bool AssetImporter::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset)
    {
        return s_AssetImporter.m_Serializers[metadata.Type]->Serialize(metadata, asset);
    }

    Ref<Asset> AssetImporter::TryLoad(const AssetMetadata& metadata)
    {
        return s_AssetImporter.m_Serializers[metadata.Type]->TryLoad(metadata);
    }

}
