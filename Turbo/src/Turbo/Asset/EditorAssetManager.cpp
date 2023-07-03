#include "tbopch.h"
#include "EditorAssetManager.h"

namespace Turbo
{
    EditorAssetManager::EditorAssetManager()
    {
    }

    EditorAssetManager::~EditorAssetManager()
    {
    }

    bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle)
    {
        return m_AssetMap.find(handle) != m_AssetMap.end();
    }

    Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle)
    {
        TBO_ENGINE_ASSERT(IsAssetHandleValid(handle));

        if (!IsAssetHandleValid(handle))
            return nullptr;

        return m_AssetMap.at(handle);
    }
}
