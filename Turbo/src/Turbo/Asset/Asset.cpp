#include "tbopch.h"
#include "Asset.h"

#include "AssetRegistryBase.h"
#include "AssetSerializer.h"

#include "Turbo/Core/Scopes.h"

namespace Turbo
{
    struct AssetHandler
    {
        Scope<AssetSerializer> Serializers[AssetType_Count];

        AssetHandler()
        {
            Serializers[AssetType_Texture2D] = CreateScope<Texture2DSerializer>();
            Serializers[AssetType_MeshSource] = CreateScope<MeshSourceSerializer>();
            Serializers[AssetType_StaticMesh] = CreateScope<StaticMeshSerializer>();
        }
    };

    static AssetHandler s_AssetHandler;

    void Asset::SetFlags(AssetFlag flags, bool enable)
    {
        if (enable)
        {
            Flags |= flags;
        }
        else
        {
            Flags &= ~flags;
        }
    }

    const char* Asset::StringifyAssetType(AssetType type)
    {
        static const char* s_StringifiedAssetTypeMap[AssetType_Count] =
        {
            "Texture2D",
            "MeshSource",
            "StaticMesh"
        };

        return s_StringifiedAssetTypeMap[type];
    }

    AssetType Asset::StringToAssetType(std::string_view type)
    {
        static std::unordered_map<std::string_view, AssetType> s_AssetTypeMap =
        {
            { "Texture2D", AssetType_Texture2D },
            { "MeshSource", AssetType_MeshSource },
            { "StaticMesh", AssetType_StaticMesh }
        };

        return s_AssetTypeMap.at(type);
    }

    bool Asset::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset)
    {
        return s_AssetHandler.Serializers[metadata.Type]->Serialize(metadata, asset);
    }

    Ref<Asset> Asset::TryLoad(const AssetMetadata& metadata)
    {
        return s_AssetHandler.Serializers[metadata.Type]->TryLoad(metadata);
    }

    Ref<Asset> Asset::Create(const AssetMetadata& metadata, const Ref<Asset>& sourceAsset)
    {
        return s_AssetHandler.Serializers[metadata.Type]->Create(metadata, sourceAsset);
    }

}
