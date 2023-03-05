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

    Ref<Project> Project::CreateDefault(const Filepath& root_dir, const String& project_name)
    {
        Scene::Config scene_config = {};
        scene_config.RelativePath = "Assets\\Scenes\\BlankScene.tscene"; // TOOD: More control over where the scene will be stored
        scene_config.Name = "BlankScene";
        Ref<Scene> default_scene = Ref<Scene>::Create(scene_config);
        {
            Entity sprite = default_scene->CreateEntity("Sprite");
            sprite.AddComponent<SpriteRendererComponent>();

            Entity camera = default_scene->CreateEntity("Camera");
            camera.AddComponent<CameraComponent>();
        }
        Project::Config project_config = {};
        project_config.Name = project_name;
        project_config.RootDirectory = root_dir;
        project_config.StartupScene = project_config.ActiveScene  = default_scene;
        project_config.ScenesRelPaths.push_back(scene_config.RelativePath);

        Ref<Project> project = Ref<Project>::Create(project_config);

        // Serialize new project
        ProjectSerializer project_serializer(project);
        TBO_ENGINE_ASSERT(project_serializer.Serialize(root_dir));
        
        // Serialize active scene
        SceneSerializer serializer(default_scene);
        TBO_ENGINE_ASSERT(serializer.Serialize(root_dir / scene_config.RelativePath));

        return project;
    }

    Ref<Project> Project::Deserialize(const Filepath& project_filepath)
    {
        Ref<Project> project = Ref<Project>::Create();
        ProjectSerializer serializer(project);
        {
            Benchmark::ScopeTimer timer("Project & Startup Scene - deserialization");
            TBO_ENGINE_ASSERT(serializer.Deserialize(project_filepath));
        }

        return project;
    }

	bool Project::SerializeScene(Ref<Scene> scene, const Filepath& scene_filepath)
	{
        SceneSerializer serializer(scene);
        return serializer.Serialize(scene_filepath);
	}

    bool Project::DeserializeScene(Ref<Scene> scene, const Filepath& scene_filepath)
    {
        SceneSerializer serializer(scene);
        return serializer.Deserialize(scene_filepath);
    }

}
