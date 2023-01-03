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
            FString64 Name;
            std::vector<Scene*> Scenes;
            Scene* DefaultScene;
            Filepath RootDirectory; // Will point to the root directory of the project
        };

        Project(const Project::Config& config = {});
        ~Project();

        static Project* CreateDefault(const Filepath& rootDirectory, const FString64& projectName);
        static Project* Deserialize(const Filepath& projectFilepath);

        const Filepath& GetRootDirectory() const { return m_Config.RootDirectory; }

        const FString64& GetName() const { return m_Config.Name; }
        Scene* GetDefaultScene() const { return m_Config.DefaultScene; }

    private:
        Project::Config m_Config;

        friend class ProjectSerializer;
    };

}
