#pragma once

#include "Turbo/Renderer/Image2D.h"
#include "Turbo/Renderer/Texture2D.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include <climits>

namespace Turbo::UI
{
    struct ScopedStyleColor
    {
        ScopedStyleColor(ImGuiCol idx, const ImVec4& color, bool predicate = true)
            : Set(predicate)
        {
            if (Set)
                ImGui::PushStyleColor(idx, color);
        }

        ScopedStyleColor(ImGuiCol idx, ImU32 color, bool predicate = true)
            : Set(predicate)
        {
            if (Set)
                ImGui::PushStyleColor(idx, color);
        }

        ~ScopedStyleColor()
        {
            if (Set)
                ImGui::PopStyleColor();
        }
    private:
        bool Set = false;
    };

    // For displaying rendererd images
    void Image(Ref<Texture2D> texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
    void Image(Ref<Image2D> image, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));

    // Display icon buttons
    bool ImageButton(Ref<Texture2D> texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), i32 frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));

    // Extends imgui standard DragScalars
    bool DragByte(const char* label, char* v, float v_speed = 1.0f, char v_min = 0, char v_max = 0, const char* format = NULL, ImGuiSliderFlags flags = 0);
    bool DragUByte(const char* label, unsigned char* v, float v_speed = 1.0f, unsigned char v_min = 0, unsigned char v_max = UCHAR_MAX, const char* format = NULL, ImGuiSliderFlags flags = 0);
    bool DragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = NULL, ImGuiSliderFlags flags = 0);
    bool DragUInt(const char* label, unsigned int* v, float v_speed = 1.0f, unsigned int v_min = 0, unsigned int v_max = UINT_MAX, const char* format = NULL, ImGuiSliderFlags flags = 0);
    bool DragLong(const char* label, long long* v, float v_speed = 1.0f, long long v_min = 0, long long v_max = 0, const char* format = NULL, ImGuiSliderFlags flags = 0);
    bool DragULong(const char* label, unsigned long long* v, float v_speed = 1.0f, unsigned long long v_min = 0, unsigned long long v_max = ULONG_MAX, const char* format = NULL, ImGuiSliderFlags flags = 0);
}
