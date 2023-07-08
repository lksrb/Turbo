#pragma once

#include <glm/glm.hpp>
#include <filesystem>

#include "Turbo/Solution/Project.h"

#include "Asset.h"

namespace Turbo
{
    class Entity;
    class Scene;

    class AssetRegistry
    {
    public:
        template<typename T>
        static Ref<T> GetAsset(AssetHandle handle)
        {
            return Project::GetActive()->m_AssetRegistry->GetAsset(handle).As<T>();
        }

        static void ImportAsset(const std::filesystem::path& filepath);

        static bool IsAssetHandleValid(AssetHandle handle);
        static bool IsAssetLoaded(AssetHandle handle);

        // Temporary solution to prefabs
        // TODO: RemoveS
        static bool SerializeToPrefab(const std::filesystem::path& filepath, Entity entity);
        static Entity DeserializePrefab(const std::filesystem::path& filepath, Scene* scene, glm::vec3 translation = glm::vec3(0.0f));
    private:
    };
}

