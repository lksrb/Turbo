#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Scene/Scene.h"

namespace YAML
{
    class Node;
    class Emitter;
}

namespace Turbo
{
    class SceneSerializer
    {
    public:
        SceneSerializer(Ref<Scene> scene);
        SceneSerializer(const SceneSerializer&) = delete;

        ~SceneSerializer();

        bool Deserialize(const std::filesystem::path& filepath);
        bool Serialize(const std::filesystem::path& filepath);
    public:
        static void SerializeEntity(YAML::Emitter& out, Entity entity);
        static void DeserializeEntity(YAML::Node& entity, Entity deserializedEntity, bool overwriteTranslation = true);
    private:
        Ref<Scene> m_Scene;
    };
}
