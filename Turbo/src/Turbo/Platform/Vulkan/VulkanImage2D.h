#pragma once

#include "Turbo/Renderer/Image2D.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanImage2D : public Image2D
    {
    public:
        VulkanImage2D(const Image2D::Config& config);
        ~VulkanImage2D();

        VkImageView GetImageView() const { return m_ImageView; }
        VkImage GetImage() const { return m_Image; }
        VkSampler GetSampler() const { return m_Sampler; }

        void Invalidate(u32 width, u32 height) override;
    private:
        VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
        VkSampler m_Sampler = VK_NULL_HANDLE;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        VkImage m_Image = VK_NULL_HANDLE;
    };
}
