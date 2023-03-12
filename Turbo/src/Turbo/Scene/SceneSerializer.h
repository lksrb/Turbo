#pragma once

#include "Turbo/Core/Common.h"

#include "Turbo/Scene/Scene.h"

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
    private:
        Ref<Scene> m_Scene;
    };
}
