#pragma once

#include "Turbo/Renderer/FrameBuffer.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanFrameBuffer : public FrameBuffer
    {
    public:
        using AttachmentResources = std::array<std::vector<std::vector<Ref<Image2D>>>, AttachmentType_Count>;

        VulkanFrameBuffer(const FrameBuffer::Config& config);
        ~VulkanFrameBuffer();

        VkFramebuffer GetHandle() const;
        void Invalidate(u32 width, u32 height) override;

        Ref<Image2D> GetAttachment(AttachmentType type, u32 index = 0) const override;
    private:
        AttachmentResources m_AttachmentResources;
        std::vector<VkFramebuffer> m_Framebuffers;
    };
}
