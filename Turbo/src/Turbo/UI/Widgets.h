#pragma once

#include "Turbo/Asset/Asset.h"

namespace Turbo::UI::Widgets
{
    using CreateMeshPopupFunc = std::function<void(const std::string&, const Ref<Asset>&)>;

    AssetHandle AssetSearchPopup(const char* popupName, AssetType filterType);

    bool YesNoPopup(const char* popupName, const char* text);

    void CreateMeshPopup(const char* popupName, DefaultAsset defaultAsset, const CreateMeshPopupFunc& func);
}
