#include "tbopch.h"
#include "SceneSerializer.h"

#define TBO_NODE_BEGIN(node, arg) stream << node" \"" << arg << "\"\n"
#define TBO_NODE_END(name) stream << "\n"
#define TBO_NODE_VEC3(name, vect3)  stream << name << " { " << vect3.x << ", " << vect3.y << ", " << vect3.z << " }\n"

namespace Turbo
{
    SceneSerializer::SceneSerializer(Scene* scene)
        : m_Scene(scene), m_StackLevel(StackLevel::None), m_ExecutionOk(false)
    {
        m_LuaEngine.SetUserdata(this);

        m_LuaEngine.AddCommand("Scene", [](void* userData, const char** pArgs, size_t nArgs)
        {
            TBO_ENGINE_ASSERT(nArgs == 1);
            SceneSerializer* serializer = reinterpret_cast<SceneSerializer*>(userData);
            TBO_ENGINE_ASSERT(serializer->m_StackLevel == StackLevel::None);
            serializer->m_StackLevel = StackLevel::SceneLevel;

            TBO_ENGINE_ASSERT(serializer->m_Scene->GetName() == pArgs[0]); // Filename and scene name should match
        });

        m_LuaEngine.AddCommand("Entity", [](void* userData, const char** pArgs, size_t nArgs)
        {
            TBO_ENGINE_ASSERT(nArgs == 1);
            SceneSerializer* serializer = reinterpret_cast<SceneSerializer*>(userData);
            TBO_ENGINE_ASSERT(serializer->m_StackLevel >= StackLevel::SceneLevel);
            serializer->m_StackLevel = StackLevel::EntityLevel;

            //---
            u64 uuid = std::stoull(pArgs[0]);

            // Sets current entity
            serializer->m_StackEntity = serializer->m_Scene->CreateEntityWithUUID(uuid);
        });

        m_LuaEngine.AddCommand("Component", [](void* userData, const char** pArgs, size_t nArgs)
        {
            std::string s = pArgs[0]; // IDK why honestly

            TBO_ENGINE_ASSERT(nArgs == 1);
            SceneSerializer* serializer = reinterpret_cast<SceneSerializer*>(userData);
            TBO_ENGINE_ASSERT(serializer->m_StackLevel >= StackLevel::EntityLevel);
            serializer->m_StackLevel = StackLevel::ComponentLevel;

            TBO_ENGINE_ASSERT(serializer->ComponentExists(s.c_str()), "Unknown component!");

            serializer->m_StackComponent = pArgs[0];
        });

        // -- Parameters --

        // Tag
        m_LuaEngine.AddCommand("Tag", [](void* userData, const char** pArgs, size_t nArgs)
        {
            TBO_ENGINE_ASSERT(nArgs == 1);
            SceneSerializer* serializer = reinterpret_cast<SceneSerializer*>(userData);
            TBO_ENGINE_ASSERT(serializer->m_StackLevel >= StackLevel::ComponentLevel);
            serializer->m_StackLevel = StackLevel::ParameterLevel;
            TBO_ENGINE_ASSERT(serializer->m_StackComponent == "Tag");

            Entity e = serializer->m_StackEntity;
            e.GetComponent<TagComponent>().Tag = pArgs[0];
        });

        // Transform
        m_LuaEngine.AddCommand("Translation", [](void* userData, const char** pArgs, size_t nArgs)
        {
            TBO_ENGINE_ASSERT(nArgs == 3);
            SceneSerializer* serializer = reinterpret_cast<SceneSerializer*>(userData);
            TBO_ENGINE_ASSERT(serializer->m_StackLevel >= StackLevel::ComponentLevel);
            serializer->m_StackLevel = StackLevel::ParameterLevel;
            TBO_ENGINE_ASSERT(serializer->m_StackComponent == "Transform");

            Entity e = serializer->m_StackEntity;
            auto& transform = e.GetComponent<TransformComponent>();
            transform.Translation.x = std::stof(pArgs[0]);
            transform.Translation.y = std::stof(pArgs[1]);
            transform.Translation.z = std::stof(pArgs[2]);
        });

        m_LuaEngine.AddCommand("Rotation", [](void* userData, const char** pArgs, size_t nArgs)
        {
            TBO_ENGINE_ASSERT(nArgs == 3);
            SceneSerializer* serializer = reinterpret_cast<SceneSerializer*>(userData);
            TBO_ENGINE_ASSERT(serializer->m_StackLevel >= StackLevel::ComponentLevel);
            serializer->m_StackLevel = StackLevel::ParameterLevel;
            TBO_ENGINE_ASSERT(serializer->m_StackComponent == "Transform");

            Entity e = serializer->m_StackEntity;
            auto& transform = e.GetComponent<TransformComponent>();
            transform.Rotation.x = std::stof(pArgs[0]);
            transform.Rotation.y = std::stof(pArgs[1]);
            transform.Rotation.z = std::stof(pArgs[2]);
        });

        m_LuaEngine.AddCommand("Scale", [](void* userData, const char** pArgs, size_t nArgs)
        {
            TBO_ENGINE_ASSERT(nArgs == 3);
            SceneSerializer* serializer = reinterpret_cast<SceneSerializer*>(userData);
            TBO_ENGINE_ASSERT(serializer->m_StackLevel >= StackLevel::ComponentLevel);
            serializer->m_StackLevel = StackLevel::ParameterLevel;
            TBO_ENGINE_ASSERT(serializer->m_StackComponent == "Transform");

            Entity e = serializer->m_StackEntity;
            auto& transform = e.GetComponent<TransformComponent>();
            transform.Scale.x = std::stof(pArgs[0]);
            transform.Scale.y = std::stof(pArgs[1]);
            transform.Scale.z = std::stof(pArgs[2]);
        });
    }

    SceneSerializer::~SceneSerializer()
    {
    }

    bool SceneSerializer::Serialize(const Filepath& filepath)
    {
        std::ofstream stream(filepath.c_str(), std::ios::trunc);
        if (!stream.is_open())
        {
            stream.close();

            TBO_ERROR("Could not serialize project! ({0})", filepath.c_str());
            return false;
        }

        TBO_NODE_BEGIN("Scene", m_Scene->m_Config.RelativePath.Filename().c_str());
        stream << "\n";

        bool success = true;

        m_Scene->m_Registry.each([&](entt::entity e)
        {
            Entity entity = { e, m_Scene };
        if (!entity)
        {
            success = false;
            return;
        }

        SerializeEntity(stream, entity);
        });

        stream.close();

        return success;
    }

    bool SceneSerializer::Deserialize(const Filepath& filepath)
    {
        return m_LuaEngine.Execute(filepath) /*&& m_ExecutionOk*/;
    }

    void SceneSerializer::SerializeEntity(std::ofstream& stream, Entity entity)
    {
        TBO_NODE_BEGIN("Entity", entity.GetUUID());

        // Tag
        TBO_NODE_BEGIN("\tComponent", "Tag");
        TBO_NODE_BEGIN("\t\tTag", entity.GetName().c_str());

        // Transform
        auto& transform = entity.Transform();
        TBO_NODE_BEGIN("\tComponent", "Transform");
        TBO_NODE_VEC3("\t\tTranslation", transform.Translation);
        TBO_NODE_VEC3("\t\tRotation", transform.Rotation);
        TBO_NODE_VEC3("\t\tScale", transform.Scale);

        TBO_NODE_END("Entity");
    }

    bool SceneSerializer::ComponentExists(const std::string& component)
    {
        // TODO: Maybe automize this process?
        static std::vector<std::string> s_ComponentRegistry =
        {
            "Tag",
            "Transform",
            "BoxCollider2D"
        };

        return std::find(s_ComponentRegistry.begin(), s_ComponentRegistry.end(), component) != s_ComponentRegistry.end();
    }
}
