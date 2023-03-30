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
        VkDeviceMemory GetMemory() const { return m_Memory; }
        VkSampler GetSampler() const { return m_Sampler; }

        void Invalidate(u32 width, u32 height) override;
    private:
        VkDeviceMemory m_Memory = nullptr;

        VkSampler m_Sampler = nullptr;
        VkImageView m_ImageView = nullptr;
        VkImage m_Image = nullptr;
    };
}
