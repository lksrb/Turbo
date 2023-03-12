#include "tbopch.h"
#include "ProjectSerializer.h"

#include "Turbo/Core/Platform.h"

#include "Turbo/Solution/Project.h"
#include "Turbo/Scene/SceneSerializer.h"

#include <yaml-cpp/yaml.h>

#include <fstream>

namespace Turbo
{
    inline YAML::Emitter& operator<<(YAML::Emitter& emitter, const std::filesystem::path& v)
    {
        return emitter.Write(v.string());
    }

	ProjectSerializer::ProjectSerializer(Ref<Project> project)
		: m_Project(project)
	{
	}

	ProjectSerializer::~ProjectSerializer()
	{
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
        TBO_ENGINE_TRACE("Deserializing project '{0}'", filepath.string());

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

        // Project name
        m_Project->m_Config.Name = data["Project"].as<std::string>();
        m_Project->m_Config.AssetsDirectory = data["AssetsDirectory"].as<std::string>();
        m_Project->m_Config.StartScenePath = data["StartScene"].as<std::string>();

        // Not serialized
        m_Project->m_Config.ProjectDirectory = filepath.parent_path();

		return true;
	}

	bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
	{
        const auto& config = m_Project->GetConfig();

        // Serialize project
        {
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "Project" << YAML::Value << config.Name;
            out << YAML::Key << "AssetsDirectory" << YAML::Value << config.AssetsDirectory;
            out << YAML::Key << "StartScene" << YAML::Value << config.StartScenePath;
            out << YAML::EndMap;
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
