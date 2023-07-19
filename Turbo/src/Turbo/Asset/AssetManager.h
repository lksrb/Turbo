#pragma once

#include <glm/glm.hpp>
#include <filesystem>

#include "Turbo/Solution/Project.h"

#include "Asset.h"

namespace Turbo
{
    class Entity;
    class Scene;

    class AssetManager
    {
    public:
        template<typename T = Asset>
        static Ref<T> GetAsset(AssetHandle handle)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Class must be derived from \"Asset\" base class!");

            auto asset = Project::GetActive()->m_AssetRegistry->GetAsset(handle);
            if (!asset)
                return nullptr;

            return asset.As<T>();
        }

        static const AssetMetadata& GetAssetMetadata(AssetHandle handle);

        static void ImportAsset(const std::filesystem::path& filepath);

        static bool IsAssetHandleValid(AssetHandle handle);
        static bool IsAssetLoaded(AssetHandle handle);

        // Temporary solution to prefabs
        // TODO: Remove
        static bool SerializeToPrefab(const std::filesystem::path& filepath, Entity entity);
        static Entity DeserializePrefab(const std::filesystem::path& filepath, Scene* scene, glm::vec3 translation = glm::vec3(0.0f));
    private:
    };
}

