#pragma once

namespace Turbo
{
    class Entity;
    class Scene;

    class AssetManager
    {
    public:

        // Temporary solution to prefabs
        static bool SerializeToPrefab(const std::filesystem::path& filepath, Entity entity);
        static bool DeserializePrefab(const std::filesystem::path& filepath, Entity entity);
    private:
    };
}

