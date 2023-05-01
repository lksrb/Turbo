#include "tbopch.h"
#include "TurboImGui.h"

#include <unordered_map>

static std::unordered_map<VkImageView, VkDescriptorSet> s_DescriptorSetCache;

VkDescriptorSet ImGui::RegisterTexture(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout)
{
    if (s_DescriptorSetCache.find(image_view) != s_DescriptorSetCache.end())
    {
        return s_DescriptorSetCache.at(image_view);
    }

    //TBO_ENGINE_INFO("Created descriptor set!");

    // Register a texture
    VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(sampler, image_view, image_layout);

    // Cache its descriptor
    s_DescriptorSetCache[image_view] = descriptorSet;
    
    return descriptorSet;
}
