#pragma once

#include "Turbo/Renderer/Image2D.h"
#include "Turbo/Renderer/Texture2D.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Turbo::UI
{
    // For displaying image
    void Image(Ref<Image2D> image, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));

    // Display icon buttons
    bool ImageButton(Ref<Texture2D> texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), i32 frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));
}
