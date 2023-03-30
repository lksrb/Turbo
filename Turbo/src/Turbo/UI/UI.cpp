#include "tbopch.h"
#include "UI.h"

#include "TurboImGui.h"

#include "Turbo/Renderer/RendererContext.h"
#include "Turbo/Platform/Vulkan/VulkanImage2D.h"

namespace Turbo
{
    void UI::Image(Ref<Texture2D> texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
    {
        UI::Image(texture->GetImage(), size, uv0, uv1, tint_col, border_col);
    }

    void UI::Image(Ref<Image2D> image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
    {
        Ref<VulkanImage2D> vulkanImage = image.As<VulkanImage2D>();

        if (vulkanImage->GetImageView())
        {
            const auto textureID = ImGui::RegisterTexture(vulkanImage->GetSampler(), vulkanImage->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            ImGui::Image(textureID, size, uv0, uv1, tint_col, border_col);
        }
    }

    bool UI::ImageButton(Ref<Texture2D> texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, i32 frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
    {
        Ref<VulkanImage2D> vulkanImage = texture->GetImage().As<VulkanImage2D>();

        const auto textureID = ImGui::RegisterTexture(vulkanImage->GetSampler(), vulkanImage->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        return ImGui::ImageButton(textureID, size, uv0, uv1, frame_padding, bg_col, tint_col);
    }

    bool UI::DragByte(const char* label, char* v, float v_speed , char v_min , char v_max , const char* format, ImGuiSliderFlags flags )
    {
        return ImGui::DragScalar(label, ImGuiDataType_S8, v, v_speed, &v_min, &v_max, format, flags);
    }

    bool UI::DragUByte(const char* label, unsigned char* v, float v_speed , unsigned char v_min , unsigned char v_max , const char* format, ImGuiSliderFlags flags)
    {
        return ImGui::DragScalar(label, ImGuiDataType_U8, v, v_speed, &v_min, &v_max, format, flags);
    }

    bool UI::DragInt(const char* label, int* v, float v_speed , int v_min , int v_max , const char* format, ImGuiSliderFlags flags )
    {
        return ImGui::DragScalar(label, ImGuiDataType_S32, v, v_speed, &v_min, &v_max, format, flags);
    }

    bool UI::DragUInt(const char* label, unsigned int* v, float v_speed, unsigned int v_min, unsigned int v_max, const char* format, ImGuiSliderFlags flags)
    {
        return ImGui::DragScalar(label, ImGuiDataType_U32, v, v_speed, &v_min, &v_max, format, flags);
    }

    bool UI::DragLong(const char* label, long long* v, float v_speed , long long v_min , long long v_max , const char* format, ImGuiSliderFlags flags )
    {
        return ImGui::DragScalar(label, ImGuiDataType_S64, v, v_speed, &v_min, &v_max, format, flags);
    }

    bool UI::DragULong(const char* label, unsigned long long* v, float v_speed , unsigned long long v_min , unsigned long long v_max , const char* format, ImGuiSliderFlags flags )
    {
        return ImGui::DragScalar(label, ImGuiDataType_U64, v, v_speed, &v_min, &v_max, format, flags);
    }

}
