#pragma once

#include "Turbo/Renderer/FrameBuffer.h"

#include "Turbo/Renderer/Fly.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanFrameBuffer : public FrameBuffer
    {
    public:
        using AttachmentResources = std::array<std::vector<Fly<Ref<Image2D>>>, AttachmentType_Count>;

        VulkanFrameBuffer(const FrameBuffer::Config& config);
        ~VulkanFrameBuffer();

        VkFramebuffer GetHandle() const;
        void Invalidate(u32 width, u32 height) override;

        const std::vector<VkClearValue>& GetClearValues() const { return m_ClearValues; }

        Ref<Image2D> GetAttachment(AttachmentType type, u32 index = 0) const override;
    private:
        std::vector<VkClearValue> m_ClearValues;
        AttachmentResources m_AttachmentResources;
        Fly<VkFramebuffer> m_Framebuffers;
    };
}
