#include "tbopch.h"
#include "AssetSerializer.h"

#include "Turbo/Renderer/Texture2D.h"
#include "Turbo/Solution/Project.h"

#include <yaml-cpp/yaml.h>

namespace Turbo
{
    bool Texture2DSerializer::Serialize(AssetHandle handle, const AssetMetadata& metadata) const
    {
        return true;
    }

    Ref<Asset> Texture2DSerializer::TryLoad(const AssetMetadata& metadata)
    {
        auto assetPath = Project::GetAssetsPath();
        auto path = (assetPath / metadata.FilePath).generic_string();
        Ref<Texture2D> result = Texture2D::Create(path);
        if (!result->IsLoaded())
            return nullptr;

        return result;
    }

}
