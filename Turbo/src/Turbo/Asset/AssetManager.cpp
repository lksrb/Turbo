#include "tbopch.h"
#include "AssetManager.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Scene/SceneSerializer.h"
#include "Turbo/Scene/Entity.h"
#include "Turbo/Debug/ScopeTimer.h"
#include "Turbo/Script/ScriptEngine.h"

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
