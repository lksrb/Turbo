#pragma once

#include "Turbo/Asset/Asset.h"

namespace Turbo::UI::Widgets
{
    AssetHandle AssetSearchPopup(const char* popupName, AssetType filterType);
    bool YesNoPopup(const char* popupName, const char* text);
}
