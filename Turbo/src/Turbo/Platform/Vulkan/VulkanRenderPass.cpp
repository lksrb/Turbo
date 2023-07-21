#include "tbopch.h"
#include "VulkanRenderPass.h"

#include "Turbo/Renderer/RendererContext.h"

namespace Turbo
{
    VulkanRenderPass::VulkanRenderPass(const RenderPass::Config& config)
        : RenderPass(config)
    {
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
    }

    void VulkanRenderPass::Invalidate()
    {
        VkDevice device = RendererContext::GetDevice();

        const auto& frameBufferConfig = m_Config.TargetFrameBuffer->GetConfig();

        std::vector<VkAttachmentDescription> attachments;

        // Color attachment
        {
            auto& colorAttachment = attachments.emplace_back();
            colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM; // VK_FORMAT_B8G8R8A8_SRGB;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            // For beginning of the render pass (vkCmdBeginRenderPass)
            colorAttachment.loadOp = m_Config.ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            // For the end of the render pass (vkCmdEndRenderPass)
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = m_Config.ClearOnLoad ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        if (frameBufferConfig.EnableDepthTesting)
        {
            // Depth buffer
            auto& depthAttachment = attachments.emplace_back();
            depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = m_Config.ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            depthAttachment.storeOp = m_Config.ClearOnLoad ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = m_Config.ClearOnLoad ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        // Subpasses and attachment references
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Depth buffer attachment reference
        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            
        std::vector<VkSubpassDescription> subpassDescriptions;
        std::vector<VkSubpassDependency> subpassDependencies;
        subpassDescriptions.resize(m_Config.SubPassCount);
        subpassDependencies.resize(m_Config.SubPassCount);

        for (u32 i = 0; i < subpassDescriptions.size(); ++i)
        {
            auto& subpassDescription = subpassDescriptions[i];
            subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription.colorAttachmentCount = 1;
            subpassDescription.pColorAttachments = &colorAttachmentRef;
            subpassDescription.inputAttachmentCount = 0;
            subpassDescription.pInputAttachments = VK_NULL_HANDLE;
            subpassDescription.preserveAttachmentCount = 0;
            subpassDescription.pPreserveAttachments = VK_NULL_HANDLE;
            subpassDescription.pResolveAttachments = VK_NULL_HANDLE;
            subpassDescription.pDepthStencilAttachment = frameBufferConfig.EnableDepthTesting ? &depthAttachmentRef : VK_NULL_HANDLE;
        }

        // Self-dependency
        auto& dependency = subpassDependencies[0];
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = 0;

        if (frameBufferConfig.EnableDepthTesting)
        {
            dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }

        // Describe dependencies between first and second subpass
        for (u32 i = 1; i < subpassDependencies.size(); ++i)
        {
            // Dependencies
            auto& dependency = subpassDependencies[i];
            dependency.srcSubpass = i - 1;
            dependency.dstSubpass = i;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            dependency.dependencyFlags = 0;

            if (frameBufferConfig.EnableDepthTesting)
            {
                dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                dependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            }
        }

        // Render pass 
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.pNext = nullptr;

        // Attachments
        renderPassInfo.attachmentCount = (u32)attachments.size();
        renderPassInfo.pAttachments = attachments.data();

        // Subpasses
        renderPassInfo.subpassCount = (u32)subpassDescriptions.size();
        renderPassInfo.pSubpasses = subpassDescriptions.data();

        // Dependencies
        renderPassInfo.dependencyCount = (u32)subpassDependencies.size();
        renderPassInfo.pDependencies = subpassDependencies.data();

        TBO_VK_ASSERT(vkCreateRenderPass(device, &renderPassInfo, VK_NULL_HANDLE, &m_RenderPass));

        // Add it to deletion queue 
        RendererContext::SubmitResourceFree([device, m_RenderPass = m_RenderPass]()
        {
            vkDestroyRenderPass(device, m_RenderPass, nullptr);
        });
    }

}
