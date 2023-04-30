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
            std::filesystem::path AssetsDirectory;
            std::filesystem::path ScriptModulePath;
            std::filesystem::path StartScenePath;
            bool OpenSolutionOnStart = true;

            // Not serialized
            std::filesystem::path ProjectDirectory;
        };

        Project(const Project::Config& config = {});
        ~Project();

        static void SetOpenSolutionOnStart(bool openSolutionOnStart)
        {
            TBO_ENGINE_ASSERT(s_ActiveProject);
            s_ActiveProject->m_Config.OpenSolutionOnStart = openSolutionOnStart;
        }

        const Project::Config& GetConfig() const
        {
            TBO_ENGINE_ASSERT(s_ActiveProject);
            return s_ActiveProject->m_Config;
        }

        static const std::string& GetProjectName() 
        { 
            TBO_ENGINE_ASSERT(s_ActiveProject);
            return s_ActiveProject->m_Config.Name;
        }
        static std::filesystem::path GetProjectDirectory() 
        {
            TBO_ENGINE_ASSERT(s_ActiveProject);
            return s_ActiveProject->m_Config.ProjectDirectory; 
        }

        static const std::filesystem::path GetAssetsPath()
        {
            TBO_ENGINE_ASSERT(s_ActiveProject);
            return GetProjectDirectory() / s_ActiveProject->m_Config.AssetsDirectory;
        }

        static std::filesystem::path GetProjectConfigPath()
        {
            TBO_ENGINE_ASSERT(s_ActiveProject);
            std::filesystem::path& configFile = Project::GetProjectDirectory() / s_ActiveProject->GetProjectName();
            configFile.concat(".tproject");
            return configFile;
        }

        static Ref<Project> GetActive() { return s_ActiveProject; }
        static void SetActive(Ref<Project> project) { s_ActiveProject = project; }

        static void Build();
    private:
        Project::Config m_Config;

        static inline Ref<Project> s_ActiveProject;

        friend class ProjectSerializer;
    };

}
