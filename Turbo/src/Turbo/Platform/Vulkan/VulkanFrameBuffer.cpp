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
            imageConfig.Format = ImageFormat_BGRA_Unorm;
            imageConfig.Aspect = ImageAspect_Color;
            imageConfig.MemoryStorage = MemoryStorage_DeviceLocal;
            imageConfig.Usage = ImageUsage_ColorAttachment | ImageUsage_Sampled;
            imageConfig.Tiling = ImageTiling_Optimal;

            m_ColorAttachments[i] = Image2D::Create(imageConfig);
            m_ColorAttachments[i]->Invalidate(width, height);

            // Attachments
            u32 attachmentCount = 1;

            VkImageView attachments[2] = {};
            attachments[0] = m_ColorAttachments[i].As<VulkanImage2D>()->GetImageView();

            if (m_Config.EnableDepthTesting)
            {
                Image2D::Config depthBufferConfig = {};
                depthBufferConfig.Format = ImageFormat_D32_SFloat_S8_Uint;
                depthBufferConfig.Aspect = ImageAspect_Depth;
                depthBufferConfig.MemoryStorage = MemoryStorage_DeviceLocal;
                depthBufferConfig.Usage = ImageUsage_DepthStencilSttachment;
                depthBufferConfig.Tiling = ImageTiling_Optimal;
                m_DepthBuffer = Image2D::Create(depthBufferConfig);
                m_DepthBuffer->Invalidate(width, height);

                attachments[1] = m_DepthBuffer.As<VulkanImage2D>()->GetImageView();
                attachmentCount++;
            }

            VkFramebufferCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.renderPass = m_Config.Renderpass.As<VulkanRenderPass>()->GetRenderPass();
            createInfo.width = width;
            createInfo.height = height;
            createInfo.layers = 1;
            createInfo.attachmentCount = attachmentCount;
            createInfo.pAttachments = attachments;

            TBO_VK_ASSERT(vkCreateFramebuffer(device, &createInfo, nullptr, &m_Framebuffers[i]));
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
        u32 currentFrame = Renderer::GetCurrentFrame();
        return m_ColorAttachments[currentFrame];
    }

}
