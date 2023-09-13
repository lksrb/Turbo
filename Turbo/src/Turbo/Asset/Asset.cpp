#include "tbopch.h"
#include "Asset.h"

#include "AssetRegistryBase.h"
#include "AssetHandler.h"

#include "Turbo/Core/Owned.h"

namespace Turbo {

    struct AssetHandlers
    {
        Owned<AssetHandler> Handlers[AssetType_Count];

        AssetHandlers()
        {
            Handlers[AssetType_Texture2D] = Owned<Texture2DHandler>::Create();
            Handlers[AssetType_MeshSource] = Owned<MeshSourceHandler>::Create();
            Handlers[AssetType_StaticMesh] = Owned<StaticMeshHandler>::Create();
            Handlers[AssetType_Prefab] = Owned<PrefabHandler>::Create();
            Handlers[AssetType_MaterialAsset] = Owned<PrefabHandler>::Create();
        }
    };

    static AssetHandlers s_AssetHandlers;

    const char* Asset::StringifyAssetType(AssetType type)
    {
        TBO_ENGINE_ASSERT(type < AssetType_Count, "Unknown asset type!");

        static constexpr const char* s_StringifiedAssetTypeMap[AssetType_Count] =
        {
            "Texture2D",
            "MeshSource",
            "StaticMesh",
            "Prefab",
            "MaterialAsset"
        };

        return s_StringifiedAssetTypeMap[type];
    }

    AssetType Asset::DestringifyAssetType(std::string_view type)
    {
        static std::unordered_map<std::string_view, AssetType> s_AssetTypeMap =
        {
            { "Texture2D", AssetType_Texture2D },
            { "MeshSource", AssetType_MeshSource },
            { "StaticMesh", AssetType_StaticMesh },
            { "Prefab", AssetType_Prefab },
            { "MaterialAsset", AssetType_MaterialAsset },
        };

        return s_AssetTypeMap.at(type);
    }

    bool Asset::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset)
    {
        return s_AssetHandlers.Handlers[metadata.Type]->Serialize(metadata, asset);
    }

    Ref<Asset> Asset::TryLoad(const AssetMetadata& metadata)
    {
        return s_AssetHandlers.Handlers[metadata.Type]->TryLoad(metadata);
    }
}
