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
        ProjectSerializer(Project* project);
        ~ProjectSerializer();

        bool Deserialize(const Filepath& filepath);

        bool Serialize(const Filepath& filepath);
    private:
        bool m_ExecutionOk;
        LuaEngine m_LuaEngine;
        StackLevel m_StackLevel;

        struct ProjectBuild
        {
            // Deserialization
            FString64 DefaultSceneName;
            std::vector<FString64> SceneRelativePaths;

            FString64 ProjectName; // TODO: Stack allocator
            Filepath RootFilePath; // Until I ran into stack overflow
        };

        Project* m_Project;
        ProjectBuild m_ProjectBuild;
    };

}
