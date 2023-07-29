#pragma once

#include "Turbo/Renderer/Texture.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanTextureCube : public TextureCube
    {
    public:
        VulkanTextureCube(const TextureCube::Config& config);
        ~VulkanTextureCube();
        
        VkImageView GetImageView() const { return m_ImageView; }
        VkSampler GetSampler() const { return m_Sampler; }
    private:
        void Load();
        VkImage m_Image = VK_NULL_HANDLE;
        VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        VkSampler m_Sampler = VK_NULL_HANDLE;
    };
}
