#pragma once

#include "Asset.h"

#include "Turbo/Solution/Project.h"

#include <glm/glm.hpp>
#include <filesystem>

namespace Turbo {

    class Entity;
    class Scene;

    class AssetManager
    {
    public:
        template<typename T>
        static Ref<T> GetAsset(AssetHandle handle)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Class must be derived from \"Asset\" base class!");

            auto asset = Project::GetActive()->m_AssetRegistry->GetAsset(handle);
            if (!asset)
                return nullptr;

            return asset.As<T>();
        }

        template<typename T>
        static Ref<T> GetAsset(const std::filesystem::path& filepath)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Class must be derived from \"Asset\" base class!");

            auto asset = Project::GetActive()->m_AssetRegistry->GetAsset(filepath);
            if (!asset)
                return nullptr;

            return asset.As<T>();
        }

        static const AssetMetadata& GetAssetMetadata(AssetHandle handle);

        static AssetHandle ImportAsset(const std::filesystem::path& filepath);

        static bool IsAssetHandleValid(AssetHandle handle);
        static bool IsAssetLoaded(AssetHandle handle);
    private:
    };

}
