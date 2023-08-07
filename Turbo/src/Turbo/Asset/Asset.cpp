#include "tbopch.h"
#include "Asset.h"

#include "AssetRegistryBase.h"
#include "AssetHandler.h"

#include "Turbo/Core/Scopes.h"

namespace Turbo {

    struct AssetHandlers {
        Scope<AssetHandler> Serializers[AssetType_Count];

        AssetHandlers()
        {
            Serializers[AssetType_Texture2D] = CreateScope<Texture2DHandler>();
            Serializers[AssetType_MeshSource] = CreateScope<MeshSourceHandler>();
            Serializers[AssetType_StaticMesh] = CreateScope<StaticMeshHandler>();
        }
    };

    static AssetHandlers s_AssetHandlers;

    const char* Asset::StringifyAssetType(AssetType type)
    {
        TBO_ENGINE_ASSERT(type < AssetType_Count, "Unknown asset type!");

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
        return s_AssetHandlers.Serializers[metadata.Type]->Serialize(metadata, asset);
    }

    Ref<Asset> Asset::TryLoad(const AssetMetadata& metadata)
    {
        return s_AssetHandlers.Serializers[metadata.Type]->TryLoad(metadata);
    }
}
