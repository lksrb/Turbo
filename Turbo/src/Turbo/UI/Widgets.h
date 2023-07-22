#pragma once

#include "Turbo/Asset/Asset.h"

namespace Turbo::UI::Widgets
{
    // popupName - must be unique
    AssetHandle AssetSearchPopup(const char* popupName, AssetType filterType);

    bool YesNoPopup(const char* popupName, const char* text);
}
