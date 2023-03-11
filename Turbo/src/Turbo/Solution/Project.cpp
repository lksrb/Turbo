#include "tbopch.h"
#include "Project.h"

#include "Turbo/Scene/Entity.h"
#include "Turbo/Scene/SceneSerializer.h"

#include "Turbo/Solution/ProjectSerializer.h"
#include "Turbo/Benchmark/ScopeTimer.h"

namespace Turbo
{
    Project::Project(const Project::Config& config)
        : m_Config(config)
    {
    }

    Project::~Project()
    {
    }

    bool Project::Create(const std::filesystem::path& root_dir_path, const std::string& project_name)
    {
        // Active project should reset
        s_ActiveProject.Reset();

        Scene::Config scene_config = {};
        scene_config.FullPath = root_dir_path / "Assets\\Scenes\\BlankScene.tscene"; // TOOD: More control over where the scene will be stored
        scene_config.Name = "BlankScene";
        Ref<Scene> default_scene = Ref<Scene>::Create(scene_config);

        Project::Config project_config = {};
        project_config.Name = project_name;
        project_config.RootDirectory = root_dir_path;
        project_config.StartupScene = project_config.ActiveScene  = default_scene;
        project_config.ScenesFullPaths.push_back(scene_config.FullPath);

        s_ActiveProject = Ref<Project>::Create(project_config);

        // Serialize new project
        ProjectSerializer project_serializer(s_ActiveProject);

        if (!project_serializer.Serialize(root_dir_path.string()))
            return false;
        
        // Serialize active scene
        SceneSerializer serializer(default_scene);
        TBO_ENGINE_ASSERT(serializer.Serialize(scene_config.FullPath.string()));

        return true;
    }

    bool Project::Open(const std::filesystem::path& project_path)
    {
        if (s_ActiveProject)
        {
            std::filesystem::path config_file = s_ActiveProject->GetRootDirectory();
            config_file /= s_ActiveProject->GetName();
            config_file.concat(".tproject");

            if (config_file == project_path)
            {
                TBO_WARN("Project is already loaded!");

                return false;
            }

            // Unload project
            s_ActiveProject.Reset();
        }

        if (project_path.extension() == ".tproject")
        {
            s_ActiveProject = Project::Deserialize(project_path);

            return true;
        }

        return false;
    }

    Ref<Project> Project::Deserialize(const std::filesystem::path& project_path)
    {
        Ref<Project> project = Ref<Project>::Create();
        ProjectSerializer serializer(project);
        {
            Benchmark::ScopeTimer timer("Project & Startup Scene - deserialization");
            TBO_ENGINE_ASSERT(serializer.Deserialize(project_path.string()));
        }

        return project;
    }
}
