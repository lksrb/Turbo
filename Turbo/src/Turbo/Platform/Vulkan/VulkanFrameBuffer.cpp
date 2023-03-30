#include "tbopch.h"
#include "VulkanFrameBuffer.h"

#include "VulkanImage2D.h"
#include "VulkanRenderPass.h"

#include "Turbo/Renderer/Renderer.h"
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

    VkFramebuffer VulkanFrameBuffer::GetFrameBuffer() const
    {
        u32 currentFrame = Renderer::GetCurrentFrame();
        return m_Framebuffers[currentFrame];
    }

    void VulkanFrameBuffer::Invalidate(u32 width, u32 height)
    {
        m_Config.Width = width;
        m_Config.Height = height;

        VkDevice device = RendererContext::GetDevice();
        u32 framesInFlight = RendererContext::FramesInFlight();

        m_ColorAttachments.resize(framesInFlight);
        m_Framebuffers.resize(framesInFlight);
        for (u32 i = 0; i < m_Framebuffers.size(); ++i)
        {
            // Images
            Image2D::Config imageConfig = {};
            imageConfig.ImageFormat = Image2D::Format_BGRA8_Unorm;
            imageConfig.Aspect = Image2D::AspectFlags_Color;
            imageConfig.Storage = Image2D::MemoryPropertyFlags_DeviceLocal;
            imageConfig.Usage = Image2D::ImageUsageFlags_ColorAttachment | Image2D::ImageUsageFlags_Sampled;
            imageConfig.ImageTiling = Image2D::ImageTiling_Optimal;

            m_ColorAttachments[i] = Image2D::Create(imageConfig);
            m_ColorAttachments[i]->Invalidate(width, height);

            // Attachments
            u32 attachmentCount = 1;

            VkImageView attachments[2] = {};
            attachments[0] = m_ColorAttachments[i].As<VulkanImage2D>()->GetImageView();

            if (m_Config.DepthBuffer)
            {
                attachments[1] = m_Config.DepthBuffer.As<VulkanImage2D>()->GetImageView();
                attachmentCount++;
            }

            VkFramebufferCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            create_info.pNext = nullptr;
            create_info.renderPass = m_Config.Renderpass.As<VulkanRenderPass>()->GetRenderPass();
            create_info.width = width;
            create_info.height = height;
            create_info.layers = 1;
            create_info.attachmentCount = attachmentCount;
            create_info.pAttachments = attachments;

            TBO_VK_ASSERT(vkCreateFramebuffer(device, &create_info, nullptr, &m_Framebuffers[i]));
        }

        // Add it to deletion queue 
        auto& resourceFreeQueue = RendererContext::GetResourceQueue();

        resourceFreeQueue.Submit(FRAMEBUFFER, [device, framebuffers = m_Framebuffers]()
        {
            for (u32 i = 0; i < framebuffers.size(); ++i)
                vkDestroyFramebuffer(device, framebuffers[i], nullptr);
        });
    }

    Ref<Image2D> VulkanFrameBuffer::GetColorAttachment() const
    {
        u32 current_frame = Renderer::GetCurrentFrame();
        return m_ColorAttachments[current_frame];
    }

}
