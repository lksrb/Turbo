#pragma once

#include "Turbo/Asset/AssetRegistryBase.h"
#include "Turbo/Asset/EditorAssetRegistry.h"

#include <filesystem>

namespace Turbo {

    class Project : public RefCounted
    {
    public:
        struct Config
        {
            std::string Name;
            std::filesystem::path AssetsDirectory;
            std::filesystem::path ScriptModulePath;
            std::filesystem::path StartScenePath;
            bool OpenSolutionOnStart;

            // Not serialized
            std::filesystem::path ProjectDirectory;
        };

        Project();
        ~Project();

        Project::Config& GetConfig()
        {
            TBO_ENGINE_ASSERT(s_ActiveProject);
            return s_ActiveProject->m_Config;
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

        /**
         * @return Full path to asset folder
         */
        static const std::filesystem::path GetAssetsPath()
        {
            TBO_ENGINE_ASSERT(s_ActiveProject);
            return GetProjectDirectory() / s_ActiveProject->m_Config.AssetsDirectory;
        }

        static std::filesystem::path GetProjectConfigPath()
        {
            TBO_ENGINE_ASSERT(s_ActiveProject);
            std::filesystem::path configFile = Project::GetProjectDirectory() / s_ActiveProject->GetProjectName();
            configFile.concat(".tproject");
            return configFile;
        }

        static Ref<Project> GetActive() { return s_ActiveProject; }

        static void SetActive(Ref<Project> project);

        Ref<EditorAssetRegistry> GetEditorAssetRegistry() const { return m_AssetRegistry.As<EditorAssetRegistry>(); }

        static std::filesystem::path GetAssetRegistryPath()
        {
            TBO_ENGINE_ASSERT(s_ActiveProject);

            return s_ActiveProject->GetAssetsPath() / "AssetRegistry.treg";
        }
    private:
        Project::Config m_Config;

        Ref<AssetRegistryBase> m_AssetRegistry;

        static inline Ref<Project> s_ActiveProject;

        friend class AssetManager;
        friend class ProjectSerializer;
    };

}
