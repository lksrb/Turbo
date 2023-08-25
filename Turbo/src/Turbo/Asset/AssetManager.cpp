#include "tbopch.h"
#include "AssetManager.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Scene/SceneSerializer.h"
#include "Turbo/Scene/Entity.h"
#include "Turbo/Debug/ScopeTimer.h"
#include "Turbo/Script/Script.h"

#include <yaml-cpp/yaml.h>

namespace YAML {

    Node LoadFile(const std::filesystem::path& filename)
    {
        std::ifstream fin(filename);
        if (!fin)
        {
            throw BadFile(filename.string());
        }
        return Load(fin);
    }

    template<>
    struct convert<glm::vec3> {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<Turbo::f32>();
            rhs.y = node[1].as<Turbo::f32>();
            rhs.z = node[2].as<Turbo::f32>();
            return true;
        }
    };
}

namespace Turbo {

    const AssetMetadata& AssetManager::GetAssetMetadata(AssetHandle handle)
    {
        return Project::GetActive()->m_AssetRegistry->GetAssetMetadata(handle);
    }

    AssetHandle AssetManager::ImportAsset(const std::filesystem::path& filepath)
    {
        return Project::GetActive()->m_AssetRegistry->ImportAsset(filepath);
    }

    bool AssetManager::IsAssetHandleValid(AssetHandle handle)
    {
        return Project::GetActive()->m_AssetRegistry->IsAssetHandleValid(handle);
    }

    bool AssetManager::IsAssetLoaded(AssetHandle handle)
    {
        return Project::GetActive()->m_AssetRegistry->IsAssetLoaded(handle);
    }

}
