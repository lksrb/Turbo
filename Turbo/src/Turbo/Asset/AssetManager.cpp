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
    namespace Utils
    {
        template<typename... Component>
        static void CopyComponentIfExists(Entity dst, Entity src)
        {
            ([&]()
            {
                if (src.HasComponent<Component>())
                    dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
            }(), ...);
        }

        template<typename... Component>
        static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
        {
            CopyComponentIfExists<Component...>(dst, src);
        }
    }

    static Ref<Scene> s_CacheScene = Ref<Scene>::Create();

    struct CachedNode
    {
        YAML::Node Node;
        size_t LastWriteTime = 0;
        Entity CachedPrefab;
    };

    static std::unordered_map<std::filesystem::path, CachedNode> s_CachedNodes(10);

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
        SceneSerializer::SerializeEntity(out, entity);
        out << YAML::EndSeq;
        out << YAML::EndMap;

        fout << out.c_str();

        return true;
    }

    Entity AssetManager::DeserializePrefab(const std::filesystem::path& filepath, Scene* scene, glm::vec3 translation)
    {
        Debug::ScopeTimer timer("Prefab");

        size_t lastTimeWrite = std::filesystem::last_write_time(filepath).time_since_epoch().count();
        auto& cachedNode = s_CachedNodes[filepath];

        Entity deserializedEntity = scene->CreateEntity();
        deserializedEntity.Transform().Translation = translation;
#if 1
        try
        {
            // If the file was modified, then
            if (lastTimeWrite != cachedNode.LastWriteTime)
            {
                // Load file 
                cachedNode = { YAML::LoadFile(filepath), lastTimeWrite };
            }
        }
        catch (YAML::Exception e)
        {
            s_CachedNodes.erase(filepath);
            TBO_ENGINE_ERROR(e.what());
            return {};
        }

        // Actually load the entity
        const auto& data = cachedNode.Node;

        if (!data["Prefab"])
        {
            s_CachedNodes.erase(filepath);
            TBO_ENGINE_ERROR("Prefab keyword is missing in prefab file!");
            return {};
        }

        auto entities = data["Entities"];

        if (!entities)
        {
            s_CachedNodes.erase(filepath);
            TBO_ENGINE_ERROR("Entity keyword is missing in prefab file!");
            return {};
        }

        // Deserialize
        // TODO: Maybe some more error checking?
        SceneSerializer::DeserializeEntity(*entities.begin(), deserializedEntity, false);
#else
        try
        {
            // If the file was modified, then
            if (lastTimeWrite != cachedNode.LastWriteTime)
            {
                // Load file 
                cachedNode = { YAML::LoadFile(filepath), lastTimeWrite };

                // Create cache entity for cache scene
                cachedNode.CachedPrefab = s_CacheScene->CreateEntityWithUUID(deserializedEntity.GetUUID());

                // Actually load the entity
                const auto& data = cachedNode.Node;

                if (!data["Prefab"])
                {
                    s_CachedNodes.erase(filepath);
                    scene->DestroyEntity(deserializedEntity);
                    TBO_ENGINE_ERROR("Prefab keyword is missing in prefab file!");
                    return {};
                }

                auto entities = data["Entities"];

                if (!entities)
                {
                    s_CachedNodes.erase(filepath);
                    scene->DestroyEntity(deserializedEntity);
                    TBO_ENGINE_ERROR("Entity keyword is missing in prefab file!");
                    return {};
                }

                // Deserialize
                // TODO: Maybe some more error checking?
                SceneSerializer::DeserializeEntity(*entities.begin(), cachedNode.CachedPrefab, false);
            }
        }
        catch (YAML::Exception e)
        {
            s_CachedNodes.erase(filepath);
            scene->DestroyEntity(deserializedEntity);
            TBO_ENGINE_ERROR(e.what());
            return {};
        }

        // Set name
        deserializedEntity.GetComponent<TagComponent>().Tag = cachedNode.CachedPrefab.GetName();

        // Set translation
        cachedNode.CachedPrefab.Transform().Translation = translation;

        // Copy all components except IDComponent and TagComponent
        scene->CopyEntity(cachedNode.CachedPrefab, deserializedEntity);
#endif
        
        return deserializedEntity;
    }

}
