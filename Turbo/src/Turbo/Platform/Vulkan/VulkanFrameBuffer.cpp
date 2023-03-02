#include "tbopch.h"
#include "VulkanFrameBuffer.h"

#include "VulkanImage2D.h"
#include "VulkanRenderPass.h"

#include "Turbo/Renderer/RendererContext.h"

namespace Turbo
{
    VulkanFrameBuffer::VulkanFrameBuffer(const FrameBuffer::Config& config)
        : FrameBuffer(config)
    {
    }

    VulkanFrameBuffer::~VulkanFrameBuffer()
    {
    }

    void VulkanFrameBuffer::Invalidate(u32 width, u32 height)
    {
        VkDevice device = RendererContext::GetDevice();
        u32 attachment_count = 1;

        VkImageView attachments[2] = {};
        attachments[0] = m_Config.ColorAttachment.Image.As<VulkanImage2D>()->GetImageView();

        if (m_Config.DepthBuffer)
        {
            attachments[1] = m_Config.DepthBuffer.As<VulkanImage2D>()->GetImageView();
            attachment_count++;
        }

        VkFramebufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.renderPass = m_Config.Renderpass.As<VulkanRenderPass>()->GetRenderPass();
        create_info.width = width;
        create_info.height = height;
        create_info.layers = 1;
        create_info.attachmentCount = attachment_count;
        create_info.pAttachments = attachments;

        TBO_VK_ASSERT(vkCreateFramebuffer(device, &create_info, nullptr, &m_Framebuffer));

        // Add it to deletion queue 
        auto& resource_free_queue = RendererContext::GetResourceQueue();

        resource_free_queue.Submit(FRAMEBUFFER, [device, m_Framebuffer = m_Framebuffer]()
        {
            vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
        });
    }

}
