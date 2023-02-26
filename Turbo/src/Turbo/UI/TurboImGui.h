#pragma once

#include <imgui.h>

#pragma warning(push, 0)
    #include <backends/imgui_impl_vulkan.h>
#pragma warning(pop)

namespace ImGui
{
    VkDescriptorSet RegisterTexture(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout);
}
