#include "tbopch.h"
#include "Project.h"

#include "Turbo/Scene/Entity.h"
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

    Ref<Project> Project::CreateDefault(const Filepath& rootDirectory, const String64& projectName)
    {
        Scene::Config sceneConfig = {};
        sceneConfig.RelativePath = "Assets\\Scenes\\BlankScene";
        sceneConfig.Name = "BlankScene";
        Ref<Scene>& defaultScene = Ref<Scene>::Create(sceneConfig);

        defaultScene->CreateEntity("Entity");
        defaultScene->CreateEntity("Entity2");

        Project::Config projectConfig = {};
        projectConfig.Name = projectName;
        projectConfig.RootDirectory = rootDirectory;
        projectConfig.StartupScene = defaultScene;
        projectConfig.Scenes.push_back(projectConfig.StartupScene);

        Ref<Project> project = Ref<Project>::Create(projectConfig);

        // Serialize new project
        ProjectSerializer projectSerializer(project);
        TBO_ASSERT(projectSerializer.Serialize(rootDirectory));

        return project;
    }

    Ref<Project> Project::Deserialize(const Filepath& projectFilepath)
    {
        Ref<Project> project = Ref<Project>::Create();
        ProjectSerializer serializer(project);
        {
            Benchmark::ScopeTimer timer("Project Deserialization");
            TBO_ASSERT(serializer.Deserialize(projectFilepath));
        }

        return project;
    }

}
