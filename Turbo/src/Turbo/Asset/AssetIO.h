#pragma once

#include "Asset.h"

#include <filesystem>

#include "Turbo/Scene/Prefab.h"

namespace Turbo {
    
    class EditorAssetRegistry;

    class AssetIO
    {
    public:
        template<typename T>
        static Ref<Asset> Import(const std::filesystem::path& filepath);

        template<typename T, typename... Args>
        static Ref<Asset> Create(Args&&... args);
    };

}
