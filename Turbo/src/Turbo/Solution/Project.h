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
            std::string Name;
            std::vector<std::filesystem::path> ScenesFullPaths;
            Ref<Scene> StartupScene, ActiveScene;
            std::filesystem::path RootDirectory; // Will point to the root directory of the project
        };

        Project(const Project::Config& config = {});
        ~Project();

        static bool Create(const std::filesystem::path& root_dir);
        static bool Open(const std::filesystem::path& project_path);

        const std::filesystem::path& GetRootDirectory() const { return m_Config.RootDirectory; }

        const std::string& GetName() const { return m_Config.Name; }
        Ref<Scene> GetStartupScene() const { return m_Config.StartupScene; }

        static Ref<Project> GetActive() { return s_ActiveProject; }
    private:
        static inline Ref<Project> s_ActiveProject;

        Project::Config m_Config;

        friend class ProjectSerializer;
    };

}
