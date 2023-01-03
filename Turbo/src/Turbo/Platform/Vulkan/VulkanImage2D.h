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

        void Invalidate(u32 width, u32 height) override;
    private:
        VkDeviceMemory m_Memory;
        VkImageView m_ImageView;
        VkImage m_Image;
    };
}
