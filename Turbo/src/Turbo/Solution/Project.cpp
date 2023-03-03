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

    Ref<Project> Project::CreateDefault(const Filepath& root_dir, const String64& project_name)
    {
        Scene::Config sceneConfig = {};
        sceneConfig.RelativePath = "Assets\\Scenes\\BlankScene";
        sceneConfig.Name = "BlankScene";
        Ref<Scene>& defaultScene = Ref<Scene>::Create(sceneConfig);

        Project::Config project_config = {};
        project_config.Name = project_name;
        project_config.RootDirectory = root_dir;
        project_config.StartupScene = defaultScene;
        project_config.Scenes.push_back(project_config.StartupScene);

        Ref<Project> project = Ref<Project>::Create(project_config);

        // Serialize new project
        ProjectSerializer project_serializer(project);
        TBO_ASSERT(project_serializer.Serialize(root_dir));

        return project;
    }

    Ref<Project> Project::Deserialize(const Filepath& project_filepath)
    {
        Ref<Project> project = Ref<Project>::Create();
        ProjectSerializer serializer(project);
        {
            Benchmark::ScopeTimer timer("Project Deserialization");
            TBO_ASSERT(serializer.Deserialize(project_filepath));
        }

        return project;
    }

	bool Project::SerializeScene(Ref<Scene> scene, const Filepath& scene_filepath)
	{
        SceneSerializer serializer(scene);
        return serializer.Serialize(scene_filepath);
	}

}
