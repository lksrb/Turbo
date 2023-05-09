#pragma once

#include <glm/glm.hpp>

namespace Turbo
{
    class Entity;
    class Scene;

    class AssetManager
    {
    public:

        // Temporary solution to prefabs
        static bool SerializeToPrefab(const std::filesystem::path& filepath, Entity entity);
        static Entity DeserializePrefab(const std::filesystem::path& filepath, Scene* scene, glm::vec3 translation = glm::vec3(0.0f));
    private:
    };
}

