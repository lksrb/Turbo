#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Scene/Scene.h"

#include <vector>

namespace Turbo
{
    class Project
    {
    public:
        struct Config
        {
            String64 Name;
            std::vector<Ref<Scene>> Scenes;
            Ref<Scene> StartupScene;
            Filepath RootDirectory; // Will point to the root directory of the project
        };

        Project(const Project::Config& config = {});
        ~Project();

        static Ref<Project> CreateDefault(const Filepath& rootDirectory, const String64& projectName);
        static Ref<Project> Deserialize(const Filepath& projectFilepath);

        const Filepath& GetRootDirectory() const { return m_Config.RootDirectory; }

        const String64& GetName() const { return m_Config.Name; }
        Ref<Scene> GetStartupScene() const { return m_Config.StartupScene; }

    private:
        Project::Config m_Config;

        friend class ProjectSerializer;
    };

}
