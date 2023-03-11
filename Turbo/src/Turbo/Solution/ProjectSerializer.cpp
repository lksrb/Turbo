#include "tbopch.h"
#include "ProjectSerializer.h"

#include "Turbo/Core/Platform.h"

#include "Turbo/Solution/Project.h"
#include "Turbo/Scene/SceneSerializer.h"

#include <yaml-cpp/yaml.h>

#include <fstream>

namespace Turbo
{
	ProjectSerializer::ProjectSerializer(Ref<Project> project)
		: m_Project(project)
	{
	}

	ProjectSerializer::~ProjectSerializer()
	{
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
        YAML::Node data;

        try
        {
            data = YAML::LoadFile(filepath.string());
        }
        catch (YAML::Exception e)
        {
            TBO_ENGINE_ERROR("An error has occured during project deserialization!");
            return false;
        }

        if (!data["Project"])
        {
            TBO_ENGINE_ERROR("Invalid project file!");
            return false;
        }

        // Root directory
        m_Project->m_Config.RootDirectory = std::filesystem::path(filepath).parent_path();
        std::string project_name = data["Project"].as<std::string>();
        m_Project->m_Config.Name = project_name;

        TBO_ENGINE_TRACE("Deserializing project '{0}'", project_name);

        std::string startup_scene_name = data["StartupScene"].as<std::string>();
        
        TBO_ENGINE_ASSERT(!startup_scene_name.empty());

        auto scenes = data["Scenes"];

        if (scenes)
        {
            for (auto scene : scenes)
            {
                std::filesystem::path p = scene["Path"].as<std::string>();
                m_Project->m_Config.ScenesFullPaths.push_back(p.string());
                
                if (p.stem() == startup_scene_name)
                {
                    Scene::Config config;
                    config.Name = startup_scene_name;
                    config.FullPath = m_Project->m_Config.RootDirectory / p.string().c_str();

                    // Deserialize startup scene
                    Ref<Scene> startup_scene = Ref<Scene>::Create(config);
                    SceneSerializer serializer(startup_scene);
                    TBO_ENGINE_ASSERT(serializer.Deserialize(config.FullPath.string()));

                    m_Project->m_Config.ActiveScene = m_Project->m_Config.StartupScene = startup_scene;
                    return true;
                }
            }
        }

		return false;
	}

	bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
	{
        // Serialize project
        {
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "Project" << YAML::Value << m_Project->GetName();
            out << YAML::Key << "StartupScene" << YAML::Value << m_Project->GetStartupScene()->GetName();

            out << YAML::Key << "Scenes" << YAML::Value << YAML::BeginSeq;

            for (auto& scene_path : m_Project->m_Config.ScenesFullPaths)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Path" << YAML::Value << scene_path.string();
                out << YAML::EndMap;
            }

            out << YAML::EndSeq << YAML::EndMap;

            std::ofstream fout(filepath);
            
            if (fout)
            {
                fout << out.c_str();
                return true;
            }
        }

		return false;
	}

}
