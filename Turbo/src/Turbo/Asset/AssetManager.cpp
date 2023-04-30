#include "tbopch.h"
#include "AssetManager.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Scene/SceneSerializer.h"
#include "Turbo/Scene/Entity.h"
#include "Turbo/Script/Script.h"

#include <yaml-cpp/yaml.h>

namespace YAML
{
    Node LoadFile(const std::filesystem::path& filename)
    {
        std::ifstream fin(filename);
        if (!fin)
        {
            throw BadFile(filename.string());
        }
        return Load(fin);
    }
}

namespace Turbo
{
	bool AssetManager::SerializeToPrefab(const std::filesystem::path& filepath, Entity entity)
	{
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Prefab" << YAML::Value << "0"; // TODO: Prefab UUID
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        SceneSerializer::SerializeEntity(out, entity);
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::string filename = entity.GetName();
        filename.append(".tprefab");
        std::ofstream fout(filepath / filename);

        if (fout)
        {
            fout << out.c_str();
            return true;
        }

        TBO_ENGINE_ERROR("Could not serialize prefab! ({})", filepath);

        return false;
	}

	bool AssetManager::DeserializePrefab(const std::filesystem::path& filepath, Entity deserializedEntity)
	{
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(filepath);
        }
        catch (YAML::Exception e)
        {
            TBO_ENGINE_ERROR(e.what());
            return {};
        }

        auto prefab = data["Prefab"];

        if (!prefab)
        {
            TBO_ENGINE_ERROR("Prefab keyword is missing in prefab file!");
            return false;
        }

        auto entities = data["Entities"];

        if (!entities)
        {
            TBO_ENGINE_ERROR("Entity keyword is missing in prefab file!");
            return false;
        }

        // TODO: Clean up

        auto entity = *entities.begin();

        std::string name;
        auto tagComponent = entity["TagComponent"];
        if (tagComponent)
            name = tagComponent["Tag"].as<std::string>();

        deserializedEntity.GetComponent<TagComponent>().Tag = name;

        SceneSerializer::DeserializeEntity(entity, deserializedEntity, false);

        return true;
	}

}
