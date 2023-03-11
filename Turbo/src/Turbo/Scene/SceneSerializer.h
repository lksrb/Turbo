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

        bool Deserialize(const std::string& filepath);
        bool Serialize(const std::string& filepath);
    private:
        Ref<Scene> m_Scene;
    };
}
