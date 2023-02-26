#pragma once

#include "Turbo/Renderer/Image2D.h"

#include <imgui.h>

namespace Turbo::UI
{
    // For displaying image
    void Image(Ref<Image2D> image, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
}
