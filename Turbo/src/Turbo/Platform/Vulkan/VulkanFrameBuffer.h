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

        VkFramebuffer GetHandle() const;
        void Invalidate(u32 width, u32 height) override;

        Ref<Image2D> GetColorAttachment() const override;
    private:
        std::vector<Ref<Image2D>> m_ColorAttachments;
        std::vector<Ref<Image2D>> m_DepthBuffers;
        std::vector<VkFramebuffer> m_Framebuffers;
    };
}
