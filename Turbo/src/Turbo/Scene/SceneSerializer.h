#pragma once

#include "Turbo/Scene/Components.h"
#include "Turbo/Scene/Entity.h"

#include "Turbo/Script/Lua/LuaEngine.h"

#include <fstream>

//std::ofstream& operator<<(std::ofstream& stream, const glm::vec3& vector3)
//{
//    stream << "{ " << vector3.x << ", " << vector3.y << ", " << vector3.z << " }";
//    return stream;
//}

namespace Turbo
{
	struct Parameter
	{
		FString64 ParameterName;
		void* Data;
	};

	struct ComponentBuild
	{
		FString64 ComponentName;
		std::unordered_map<FString64, Parameter> Parameters;
	};

    class SceneSerializer
    {
    public:
        SceneSerializer(Scene* scene);
        ~SceneSerializer();

        bool Serialize(const Filepath& filepath);
        bool Deserialize(const Filepath& filepath);
    private:
        void SerializeEntity(std::ofstream& stream, Entity entity);
		bool ComponentExists(const std::string& component);

		std::string m_StackComponent;
		Entity m_StackEntity;
		StackLevel m_StackLevel;

		bool m_ExecutionOk;
		Scene* m_Scene;

        LuaEngine m_LuaEngine;
    };
}
