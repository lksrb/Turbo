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
        delete m_Config.DefaultScene;
    }

    Project* Project::CreateDefault(const Filepath& rootDirectory, const FString64& projectName)
    {
        Scene::Config sceneConfig = {};
        sceneConfig.RelativePath = "Assets\\Scenes\\BlankScene";
        sceneConfig.Name = "BlankScene";
        Scene* defaultScene = new Scene(sceneConfig);

        defaultScene->CreateEntity("Entity");
        defaultScene->CreateEntity("Entity2");

        Project::Config projectConfig = {};
        projectConfig.Name = projectName;
        projectConfig.RootDirectory = rootDirectory;
        projectConfig.DefaultScene = defaultScene;
        projectConfig.Scenes.push_back(projectConfig.DefaultScene);

        Project* project = new Project(projectConfig);

        // Serialize new project
        ProjectSerializer projectSerializer(project);
        TBO_ASSERT(projectSerializer.Serialize(rootDirectory));

        return project;
    }

    Project* Project::Deserialize(const Filepath& projectFilepath)
    {
        Project* project = new Project;
        ProjectSerializer serializer(project);
        {
            Benchmark::ScopeTimer timer("Project Deserialization");
            TBO_ASSERT(serializer.Deserialize(projectFilepath));
        }

        return project;
    }

}
