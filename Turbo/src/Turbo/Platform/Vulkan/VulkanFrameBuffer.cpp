#include "tbopch.h"
#include "VulkanFrameBuffer.h"

#include "VulkanImage2D.h"
#include "VulkanRenderPass.h"

#include "Turbo/Renderer/RendererContext.h"

namespace Turbo
{
    VulkanFrameBuffer::VulkanFrameBuffer(const FrameBuffer::Config& config)
        : FrameBuffer(config), m_Framebuffer(VK_NULL_HANDLE)
    {
    }

    VulkanFrameBuffer::~VulkanFrameBuffer()
    {
    }

    void VulkanFrameBuffer::Invalidate(u32 width, u32 height)
    {
        TBO_ENGINE_ASSERT(m_Config.AttachmentsCount < 10);
        
        VkDevice device = RendererContext::GetDevice();

        VkFramebufferCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.renderPass = m_Config.Renderpass.As<VulkanRenderPass>()->GetRenderPass();
        createInfo.width = width;
        createInfo.height = height;
        createInfo.layers = 1;
        createInfo.attachmentCount = m_Config.AttachmentsCount;

        VkImageView attachments[10] = {};

        for (u32 i = 0; i < m_Config.AttachmentsCount; ++i)
        {
            attachments[i] = m_Config.Attachments[i].Image.As<VulkanImage2D>()->GetImageView();
        }


        if (m_Config.DepthBuffer)
        {
            attachments[m_Config.AttachmentsCount] = m_Config.DepthBuffer.As<VulkanImage2D>()->GetImageView();
            createInfo.attachmentCount = m_Config.AttachmentsCount + 1;
        }

        createInfo.pAttachments = attachments;

        TBO_VK_ASSERT(vkCreateFramebuffer(device, &createInfo, nullptr, &m_Framebuffer));
        // Add it to deletion queue 
        auto& resourceFreeQueue = RendererContext::GetResourceQueue();

        resourceFreeQueue.Submit(FRAMEBUFFER, [device, m_Framebuffer = m_Framebuffer]()
        {
            vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
        });
    }

}
