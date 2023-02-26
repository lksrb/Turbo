#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Core/Filepath.h"

#include "Turbo/Scene/Scene.h"

namespace Turbo
{
    class SceneSerializer
    {
    public:
        SceneSerializer(Ref<Scene> scene);
        ~SceneSerializer();

        bool Deserialize(const Filepath& filepath);
        bool Serialize(const Filepath& filepath);
    private:
        Ref<Scene> m_Scene;
    };
}
