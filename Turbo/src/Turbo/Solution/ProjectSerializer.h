#pragma once

#include "Turbo/Script/Lua/LuaEngine.h"

#include <unordered_map>
#include <vector>

namespace Turbo 
{
    class Project;
    class Scene;

    class ProjectSerializer 
    {
    public:
        ProjectSerializer(Ref<Project> project);
        ~ProjectSerializer();

        bool Deserialize(const Filepath& filepath);

        bool Serialize(const Filepath& filepath);
    private:
        bool m_ExecutionOk = false;
        StackLevel m_StackLevel = StackLevel::None;
        struct ProjectBuild
        {
            // Deserialization
            String64 DefaultSceneName;
            std::vector<String64> SceneRelativePaths;

            String64 ProjectName; // TODO: Stack allocator
            Filepath RootFilePath; // Until I ran into stack overflow
        };

        LuaEngine m_LuaEngine;
        Ref<Project> m_Project;

        ProjectBuild m_ProjectBuild;
    };

}
