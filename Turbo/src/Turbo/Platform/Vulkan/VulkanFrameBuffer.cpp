#include "tbopch.h"
#include "VulkanFrameBuffer.h"

#include "Turbo/Renderer/Renderer.h"

#include "VulkanImage2D.h"
#include "VulkanRenderPass.h"
#include "VulkanContext.h"

namespace Turbo
{
    VulkanFrameBuffer::VulkanFrameBuffer(const FrameBuffer::Config& config)
        : FrameBuffer(config)
    {
        m_ClearValues.reserve(config.Attachments.size());
    }

    VulkanFrameBuffer::~VulkanFrameBuffer()
    {
        Renderer::SubmitResourceFree([framebuffers = m_Framebuffers]()
        {
            VkDevice device = VulkanContext::Get()->GetDevice();

            for (u32 i = 0; i < framebuffers.Size(); ++i)
                vkDestroyFramebuffer(device, framebuffers[i], nullptr);
        });
    }

    VkFramebuffer VulkanFrameBuffer::GetHandle() const
    {
        u32 currentFrame = Renderer::GetCurrentFrame();
        return m_Framebuffers[currentFrame];
    }

    void VulkanFrameBuffer::Invalidate(u32 width, u32 height)
    {
        m_Config.Width = width;
        m_Config.Height = height;

        VkDevice device = VulkanContext::Get()->GetDevice();

        // Add it to deletion queue
        if (!m_Framebuffers.Empty())
        {
            for (u32 i = 0; i < m_Framebuffers.Size(); ++i)
            {
                for (const auto& resourceByType : m_AttachmentResources)
                {
                    for (const auto& resouceByIndex : resourceByType)
                    {
                        auto image = resouceByIndex[i].As<VulkanImage2D>();
                        image->Invalidate(width, height);
                    }
                }
            }

            Renderer::SubmitRuntimeResourceFree([device, framebuffers = m_Framebuffers, attachmentResources = m_AttachmentResources]()
            {
                for (u32 i = 0; i < framebuffers.Size(); ++i)
                {
                    vkDestroyFramebuffer(device, framebuffers[i], nullptr);
                }
            });
        }

        for (const auto& [type, count] : m_Config.Attachments)
        {
            m_AttachmentResources[type].clear();
            m_AttachmentResources[type].reserve(count);
            if (type == FrameBuffer::AttachmentType_Color)
            {
                Image2D::Config config = {};
                config.Format = ImageFormat_BGRA_Unorm;
                config.Aspect = ImageAspect_Color;
                config.MemoryStorage = MemoryStorage_DeviceLocal;
                config.Usage = ImageUsage_ColorAttachment | ImageUsage_Sampled;
                config.Tiling = ImageTiling_Optimal;
                config.DebugName = "FrameBuffer-ColorAttachment";

                auto& fifResource = m_AttachmentResources[type].emplace_back();
                for (u32 i = 0; i < fifResource.Size(); ++i)
                {
                    fifResource[i] = Image2D::Create(config);
                    fifResource[i]->Invalidate(width, height);
                }

                auto& clearValue = m_ClearValues.emplace_back();
                clearValue.color = { m_Config.ClearColor.r, m_Config.ClearColor.g, m_Config.ClearColor.b, m_Config.ClearColor.a };
            } 
            else if (type == FrameBuffer::AttachmentType_SelectionBuffer)
            {
                Image2D::Config config = {};
                config.Format = ImageFormat_R_SInt;
                config.Aspect = ImageAspect_Color;
                config.MemoryStorage = MemoryStorage_DeviceLocal;
                config.Usage = ImageUsage_ColorAttachment | ImageUsage_Sampled | ImageUsage_Transfer_Source;
                config.Tiling = ImageTiling_Optimal;
                config.DebugName = "FrameBuffer-SelectionBufferAttachment";

                auto& fifResource = m_AttachmentResources[type].emplace_back();
                for (u32 i = 0; i < fifResource.Size(); ++i)
                {
                    fifResource[i] = Image2D::Create(config);
                    fifResource[i]->Invalidate(width, height);
                }
                auto& clearValue = m_ClearValues.emplace_back();
                clearValue.color.int32[0] = -1;
                clearValue.color.int32[1] = -1;
                clearValue.color.int32[2] = -1;
                clearValue.color.int32[3] = -1;
            }
            else if (type == FrameBuffer::AttachmentType_Depth)
            {
                Image2D::Config config = {};
                config.Format = ImageFormat_D32_SFloat_S8_UInt;
                config.Aspect = ImageAspect_Depth;
                config.MemoryStorage = MemoryStorage_DeviceLocal;
                config.Usage = ImageUsage_DepthStencilSttachment;
                config.Tiling = ImageTiling_Optimal;
                config.DebugName = "FrameBuffer-DepthAttachment";

                auto& fifResource = m_AttachmentResources[type].emplace_back();
                for (u32 i = 0; i < fifResource.Size(); ++i)
                {
                    fifResource[i] = Image2D::Create(config);
                    fifResource[i]->Invalidate(width, height);
                }
                auto& clearValue = m_ClearValues.emplace_back();
                clearValue.depthStencil = { 1.0f, 0 };
            }
        }
        // TODO: Multiple attachments issue

        for (u32 i = 0; i < m_Framebuffers.Size(); ++i)
        {
            std::vector<VkImageView> attachments;
            attachments.reserve(m_Config.Attachments.size());

            for (const auto& resourceByType : m_AttachmentResources)
            {
                for (const auto& resouceByIndex : resourceByType)
                {
                    auto image = resouceByIndex[i].As<VulkanImage2D>();
                    attachments.emplace_back(image->GetImageView());
                }
            }

            VkFramebufferCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.renderPass = m_RenderPass.As<VulkanRenderPass>()->GetHandle();
            createInfo.width = width;
            createInfo.height = height;
            createInfo.layers = 1;
            createInfo.attachmentCount = (u32)m_Config.Attachments.size();
            createInfo.pAttachments = attachments.data();

            TBO_VK_ASSERT(vkCreateFramebuffer(device, &createInfo, nullptr, &m_Framebuffers[i]));
        }

#if OLD
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
                depthBufferConfig.Format = ImageFormat_D32_SFloat_S8_UInt;
                depthBufferConfig.Aspect = ImageAspect_Depth;
                depthBufferConfig.MemoryStorage = MemoryStorage_DeviceLocal;
                depthBufferConfig.Usage = ImageUsage_DepthStencilSttachment;
                depthBufferConfig.Tiling = ImageTiling_Optimal;
                m_DepthAttachments[i] = Image2D::Create(depthBufferConfig);
                m_DepthAttachments[i]->Invalidate(width, height);

                attachments[1] = m_DepthAttachments[i].As<VulkanImage2D>()->GetImageView();
                attachmentCount++;
            }

            VkFramebufferCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.renderPass = m_RenderPass.As<VulkanRenderPass>()->GetHandle();
            createInfo.width = width;
            createInfo.height = height;
            createInfo.layers = 1;
            createInfo.attachmentCount = attachmentCount;
            createInfo.pAttachments = attachments;

            TBO_VK_ASSERT(vkCreateFramebuffer(device, &createInfo, nullptr, &m_Framebuffers[i]));
        }
#endif
    }

    Ref<Image2D> VulkanFrameBuffer::GetAttachment(AttachmentType type, u32 index) const
    {
        TBO_ENGINE_ASSERT(type < FrameBuffer::AttachmentType_Count);
        TBO_ENGINE_ASSERT(index < m_AttachmentResources[type].size());

        u32 currentFrame = Renderer::GetCurrentFrame();
        return m_AttachmentResources[type][index][currentFrame];
    }

}
