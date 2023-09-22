#include "tbopch.h"
#include "AssetIO.h"

#include "AssetManager.h"

#include "Turbo/Core/FileSystem.h"

#include "Turbo/Renderer/Mesh.h"
#include "Turbo/Renderer/Texture.h"
#include "Turbo/Scene/Prefab.h"

#include <yaml-cpp/yaml.h>

namespace Turbo {

    /**
     * Import Asset - This means that we want to import a new asset into into the registry and serialize it
     */
    template<typename T>
    Ref<Asset> AssetIO::Import(const std::filesystem::path& filepath)
    {
        TBO_ENGINE_ASSERT(false);
        return nullptr;
    }

    template<>
    Ref<Asset> AssetIO::Import<Texture2D>(const std::filesystem::path& filepath)
    {
        Ref<Texture2D> texture = Texture2D::Create(filepath.string());
        if (!texture->IsLoaded())
            return nullptr;

        return texture;
    }

    template<>
    Ref<Asset> AssetIO::Import<MeshSource>(const std::filesystem::path& filepath)
    {
        Ref<MeshSource> meshSource = Ref<MeshSource>::Create(filepath.string());
        if (!meshSource->IsLoaded())
            return nullptr;

        // Feels a bit hacky but good for now
        {
            std::filesystem::path staticMeshPath = FileSystem::ReplaceExtension(filepath, ".tmesh");

            AssetMetadata metadata;
            metadata.FilePath = std::filesystem::relative(staticMeshPath, Project::GetAssetsPath());
            metadata.Type = AssetType_StaticMesh;
            metadata.IsLoaded = true;

            Ref<StaticMesh> staticMesh = Ref<StaticMesh>::Create(meshSource);
            Project::GetActive()->GetEditorAssetRegistry()->RegisterAsset(staticMesh, metadata);

            Asset::Serialize(metadata, staticMesh);
        }

        return meshSource;
    }

}
