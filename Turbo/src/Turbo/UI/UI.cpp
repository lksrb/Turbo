#include "tbopch.h"
#include "UI.h"

#include "TurboImGui.h"

#include "Turbo/Renderer/RendererContext.h"
#include "Turbo/Platform/Vulkan/VulkanImage2D.h"

namespace Turbo
{
    void UI::Image(Ref<Image2D> image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
    {
        Ref<VulkanImage2D> vulkanImage = image.As<VulkanImage2D>();

        if (vulkanImage->GetImageView())
        {
            const auto textureID = ImGui::RegisterTexture(vulkanImage->GetSampler(), vulkanImage->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            ImGui::Image(textureID, size, uv0, uv1, tint_col, border_col);
        }
    }

    bool UI::ImageButton(Ref<Texture2D> texture, const ImVec2& size, const ImVec2& uv0 /*= ImVec2(0, 0)*/, const ImVec2& uv1 /*= ImVec2(1, 1)*/, i32 frame_padding /*= -1*/, const ImVec4& bg_col /*= ImVec4(0, 0, 0, 0)*/, const ImVec4& tint_col /*= ImVec4(1, 1, 1, 1)*/)
    {
        Ref<VulkanImage2D> vulkanImage = texture->GetImage().As<VulkanImage2D>();

        const auto textureID = ImGui::RegisterTexture(vulkanImage->GetSampler(), vulkanImage->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        return ImGui::ImageButton(textureID, size, uv0, uv1, frame_padding, bg_col, tint_col);
    }
}
