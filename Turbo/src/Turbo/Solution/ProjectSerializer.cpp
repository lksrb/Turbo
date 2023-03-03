#include "tbopch.h"
#include "ProjectSerializer.h"

#include "Turbo/Core/Platform.h"

#include "Turbo/Solution/Project.h"
#include "Turbo/Scene/SceneSerializer.h"

#include <fstream>

namespace Turbo
{
	ProjectSerializer::ProjectSerializer(Ref<Project> project)
		: m_Project(project)
	{
		// Push userdata so this class can be accessed from lua
		m_LuaEngine.SetUserdata(this);

		m_LuaEngine.AddCommand("Project", [](void* userData, const char** pArgs, size_t nArgs)
		{
			TBO_ENGINE_ASSERT(nArgs == 1);
			ProjectSerializer* serializer = reinterpret_cast<ProjectSerializer*>(userData);
			TBO_ENGINE_ASSERT(serializer->m_StackLevel == StackLevel::None);
			serializer->m_StackLevel = StackLevel::ProjectLevel;

			serializer->m_ProjectBuild.ProjectName = pArgs[0];
		});

		m_LuaEngine.AddCommand("Scenes", [](void* userData, const char** pArgs, size_t nArgs)
		{
			ProjectSerializer* serializer = reinterpret_cast<ProjectSerializer*>(userData);
			TBO_ENGINE_ASSERT(serializer->m_StackLevel == StackLevel::ProjectLevel);
			serializer->m_StackLevel = StackLevel::SceneLevel;

			for (size_t i = 0; i < nArgs; ++i)
			{
				String64 sceneRelPath = pArgs[i];

				// Converts forward slash to backslash  
				Filepath::ConvertToBackslash(sceneRelPath.Data());

				sceneRelPath.Append(".tscene");

				serializer->m_ProjectBuild.SceneRelativePaths.push_back(sceneRelPath);
			}
		});

		m_LuaEngine.AddCommand("DefaultScene", [](void* userData, const char** pArgs, size_t nArgs)
		{
			TBO_ENGINE_ASSERT(nArgs == 1);
			ProjectSerializer* serializer = reinterpret_cast<ProjectSerializer*>(userData);
			TBO_ENGINE_ASSERT(serializer->m_StackLevel == StackLevel::ProjectLevel);

			serializer->m_ProjectBuild.DefaultSceneName = pArgs[0];
		});
	}

	ProjectSerializer::~ProjectSerializer()
	{
	}

	bool ProjectSerializer::Deserialize(const Filepath& filepath)
	{
		m_ProjectBuild.RootFilePath = filepath.Directory();

		// Gathers information about project
		if (m_LuaEngine.Execute(filepath)/* && m_ExecutionOk*/)
		{
			// Set root directory
			m_Project->m_Config.RootDirectory = m_ProjectBuild.RootFilePath;

			// Set path to config file(.tproject) TODO: Maybe unnecessary
			m_Project->m_Config.Name = m_ProjectBuild.ProjectName;

			// --- Selecting default scene ---

			Filepath defaultRelPath;
			for (auto& path : m_ProjectBuild.SceneRelativePaths)
			{
				Filepath sceneRelPath = path.CStr();

				if (sceneRelPath.Filename() == m_ProjectBuild.DefaultSceneName.CStr())
				{
					// Scene specified in 'DefaultScene' was found in 'Scenes' list
					defaultRelPath = sceneRelPath;
					break;
				}
			}

			if (defaultRelPath.Empty())
			{
				// Scene specified in 'DefaultScene' was NOT found

				if (m_ProjectBuild.SceneRelativePaths.empty())
				{
					TBO_ENGINE_WARN("Project has no scenes, No default scene selected.");
					return true;
				}

				// Select first one
				defaultRelPath = m_ProjectBuild.SceneRelativePaths[0].CStr();

				TBO_ENGINE_WARN("No default scene, selecting first scene from the list. ({0})", defaultRelPath.CStr());
			}

			Filepath defaultAbsPath = m_Project->m_Config.RootDirectory;
			defaultAbsPath /= defaultRelPath;

			// SCENE SERIALIZATION

			// Allocates memory for scene
			Scene::Config sceneConfig = {};
			sceneConfig.Name = defaultRelPath.Filename();
			sceneConfig.RelativePath = defaultRelPath;

            Ref<Scene> defaultScene = Ref<Scene>::Create(sceneConfig);

			SceneSerializer serializer(defaultScene);
			bool success = serializer.Deserialize(defaultAbsPath);

			if (success)
			{
				m_Project->m_Config.StartupScene = defaultScene;
				return true;
			}

            defaultScene.Reset();
		}

		TBO_ENGINE_ERROR("Could not deserialize project!({0})", filepath.CStr());

		return false;
	}

	bool ProjectSerializer::Serialize(const Filepath& filepath)
	{
		Filepath root = filepath;
		Filepath rootAssetsSceneAbs = root / "Assets" / "Scenes";

		// Create folders
		Platform::Result result = Platform::CreateDirectory(root);

		if (result == Platform::Result::AlreadyExists)
		{
			// Project already exists...
			TBO_ENGINE_ASSERT(false);
			return true;
		}

		// Create asset folder
		Platform::CreateDirectory(root / "Assets");
		Platform::CreateDirectory(rootAssetsSceneAbs);

		// Create root config file
		root /= root.Filename();
		root.Append(".tproject");

		std::ofstream stream(root.CStr(), std::ios::trunc);
		if (!stream.is_open())
		{
			stream.close();

			TBO_ENGINE_ERROR("Could not serialize project! ({0})", filepath.CStr());
			return false;
		}

		stream << "Project \"" << root.Filename().CStr() << "\"\n";
		stream << "\n\tDefaultScene \"" << "BlankScene" << "\"\n";
		stream << "\n\tScenes {\n";
		stream << "\t\t\"Assets/Scenes/BlankScene\"\n";
		stream << "\t}\n";
		stream.close();

		// Create Default Scene
		SceneSerializer serializer(m_Project->m_Config.StartupScene);
		TBO_ENGINE_ASSERT(serializer.Serialize(rootAssetsSceneAbs / "BlankScene.tscene"), "Failed to serialize scene!");

		return true;
	}

}
