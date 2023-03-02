#pragma once

#include "Turbo/Renderer/FrameBuffer.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanFrameBuffer : public FrameBuffer
    {
    public:
        VulkanFrameBuffer(const FrameBuffer::Config& config);
        ~VulkanFrameBuffer();

        VkFramebuffer GetFrameBuffer() const { return m_Framebuffer; }
        void Invalidate(u32 width, u32 height) override;
    private:
        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
    };
}
