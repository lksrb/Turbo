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
        template<typename T>
        static Ref<T> GetAsset(AssetHandle handle)
        {
            return Project::GetActive()->m_AssetManager->GetAsset(handle).As<T>();
        }

        static 
        static void ExportAsset(Ref<Asset> asset);

        // Temporary solution to prefabs
        static bool SerializeToPrefab(const std::filesystem::path& filepath, Entity entity);
        static Entity DeserializePrefab(const std::filesystem::path& filepath, Scene* scene, glm::vec3 translation = glm::vec3(0.0f));
    private:
    };
}

