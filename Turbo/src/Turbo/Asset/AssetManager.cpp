#include "tbopch.h"
#include "AssetManager.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Scene/SceneSerializer.h"
#include "Turbo/Scene/Entity.h"
#include "Turbo/Debug/ScopeTimer.h"
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
    struct CachedNode
    {
        YAML::Node Node;
        size_t LastWriteTime = 0;
        Entity CachedPrefabEntity;
    };

    static std::unordered_map<std::filesystem::path, CachedNode> s_CachedNodes(10);
    static Ref<Scene> s_CacheScene = Ref<Scene>::Create();

    namespace Utils
    {
        static void SerializeRecursively(YAML::Emitter& out, Scene* scene, Entity entity)
        {
            SceneSerializer::SerializeEntity(out, entity);

            // Children
            const auto& children = entity.GetChildren();
            for (auto childUUID : children)
            {
                Entity child = scene->FindEntityByUUID(childUUID);
                if (child)
                {
                    SerializeRecursively(out, scene, child);
                }
            }
        }

        static CachedNode LoadOrGetNode(const std::filesystem::path& filepath)
        {
            size_t lastTimeWrite = std::filesystem::last_write_time(filepath).time_since_epoch().count();

            auto& cachedNode = s_CachedNodes[filepath];

            try
            {
                // If the file was modified, then
                if (lastTimeWrite != cachedNode.LastWriteTime)
                {
                    // Load file 
                    YAML::Node node = YAML::LoadFile(filepath);

                    auto entities = node["Entities"];

                    // Deserialize parent entity
                    auto& it = entities.begin();
                    Entity cacheParentEntity = s_CacheScene->CreateEntity();
                    SceneSerializer::DeserializeEntity(*it, cacheParentEntity);

                    while (++it != entities.end())
                    {
                        auto& it = entities.begin();
                        Entity child = s_CacheScene->CreateEntity();
                        SceneSerializer::DeserializeEntity(*it, child);
                    }

                    cachedNode = { node, lastTimeWrite, cacheParentEntity };
                }
            }
            catch (YAML::Exception e)
            {
                s_CachedNodes.erase(filepath);
                TBO_ENGINE_ERROR(e.what());
                return {};
            }

            return cachedNode;
        }

        static bool PrefabErrorCheck(const YAML::Node& node, const std::filesystem::path& filepath)
        {
            if (!node)
                return false;

            if (!node["Prefab"])
            {
                s_CachedNodes.erase(filepath);
                TBO_ENGINE_ERROR("Prefab keyword is missing in prefab file!");
                return false;
            }

            auto entities = node["Entities"];

            if (!entities)
            {
                s_CachedNodes.erase(filepath);
                TBO_ENGINE_ERROR("Entity keyword is missing in prefab file!");
                return false;
            }

            return true;
        }
    }

    bool AssetManager::SerializeToPrefab(const std::filesystem::path& filepath, Entity entity)
    {
        std::string filename = entity.GetName();
        filename.append(".tprefab");
        std::ofstream fout(filepath / filename);

        if (!fout)
        {
            TBO_ENGINE_ERROR("Could not serialize prefab! ({})", filepath);
            return false;
        }

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Prefab" << YAML::Value << "0"; // TODO: Prefab UUID
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        Utils::SerializeRecursively(out, entity.m_Scene, entity);

        out << YAML::EndSeq;
        out << YAML::EndMap;

        fout << out.c_str();

        return true;
    }

    Entity AssetManager::DeserializePrefab(const std::filesystem::path& filepath, Scene* scene, glm::vec3 translation)
    {
        //Debug::ScopeTimer timer("Prefab Deserialization");

#if 0
        const auto& data = Utils::LoadOrGetNode(filepath).Node;

        if (!Utils::PrefabErrorCheck(data, filepath))
            return {};

        auto entities = data["Entities"];
        auto& it = entities.begin();

        // Top entity
        Entity parent = scene->CreateEntity();
        parent.Transform().Translation = translation;
        SceneSerializer::DeserializeEntity(*it, parent, false);

        //  Children
        while (++it != entities.end())
        {
            u64 uuid = (*it)["Entity"].as<u64>();
            Entity child = scene->CreateEntityWithUUID(uuid);
            child.Transform().Translation = translation; // Temporary
            SceneSerializer::DeserializeEntity(*it, child, false);
        }
        return parent;
#else
        auto& data = Utils::LoadOrGetNode(filepath);

        if (!Utils::PrefabErrorCheck(data.Node, filepath))
            return {};

        Entity cachedPrefabEntity = data.CachedPrefabEntity;
        cachedPrefabEntity.Transform().Translation = translation;

        // Create new entity
        Entity parent = scene->CreateEntity(cachedPrefabEntity.GetName());

        // Creates and copies new field instances
        if (cachedPrefabEntity.HasComponent<ScriptComponent>())
            Script::CopyScriptClassFields(cachedPrefabEntity, parent);

        scene->CopyEntity(cachedPrefabEntity, parent);

        // Copy children
        const auto& cachedChildren = cachedPrefabEntity.GetChildren();
        for (UUID childUUID : cachedChildren)
        {
            Entity cachedChild = s_CacheScene->FindEntityByUUID(childUUID);
            cachedChild.Transform().Translation = translation;

            Entity child = scene->CreateEntity();

            if (cachedPrefabEntity.HasComponent<ScriptComponent>())
                Script::CopyScriptClassFields(cachedChild, child);

            scene->CopyEntity(cachedChild, child);
        }

        return parent;
#endif
    }

}
