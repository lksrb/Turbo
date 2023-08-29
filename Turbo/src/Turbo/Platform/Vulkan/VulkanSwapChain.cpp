#include "tbopch.h"

#include "VulkanSwapChain.h"

#include "VulkanContext.h"
#include "VulkanImage2D.h"
#include "VulkanUtils.h"

namespace Turbo
{
    VulkanSwapChain::VulkanSwapChain()
    {
        Initialize();
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        Shutdown();
    }

    void VulkanSwapChain::Initialize()
    {
        VulkanDevice& device = VulkanContext::Get()->GetDevice();

        // Set swapchain format
        m_SwapChainFormat = VulkanContext::Get()->GetSurfaceFormat();

        // Create swapchain render pass
        CreateRenderpass();

        // Create semaphores and fences
        CreateSyncObjects();

        // Create dedicated command buffers
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = device.GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = m_RenderCommandBuffers.Size();

        TBO_VK_ASSERT(vkAllocateCommandBuffers(device,
            &allocInfo, m_RenderCommandBuffers.Data()));

        // Swapchain, renderpass, etc. are created on the first resize

        // Determine if graphics and present are the same queue
        auto& indices = device.GetPhysicalDevice().GetQueueFamilyIndices();
        if (indices.Graphics != indices.Present)
        {
            m_SwapchainQueues = { indices.Graphics.value(), indices.Present.value() };
            m_QueueShareMode = VK_SHARING_MODE_CONCURRENT; 
        }
        else
        {
            m_QueueShareMode = VK_SHARING_MODE_EXCLUSIVE;
        }
    }

    void VulkanSwapChain::Shutdown()
    {
        VkDevice device = VulkanContext::Get()->GetDevice();

        vkDestroyRenderPass(device, m_Renderpass, nullptr);

        for (u32 i = 0; i < RendererSettings::FramesInFlight; ++i)
        {
            vkDestroySemaphore(device, m_PresentSemaphores[i], nullptr);
            vkDestroySemaphore(device, m_RenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, m_InFlightFences[i], nullptr);
        }

        Cleanup();
    }

    void VulkanSwapChain::Cleanup()
    {
        VkDevice device = VulkanContext::Get()->GetDevice();

        for (auto framebuffer : m_Framebuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);

        for (auto imageView : m_Imageviews)
            vkDestroyImageView(device, imageView, nullptr);

        vkDestroySwapchainKHR(device, m_Swapchain, nullptr);

        m_Swapchain = VK_NULL_HANDLE;
    }

    void VulkanSwapChain::Resize(u32 width, u32 height)
    {
        VulkanDevice& device = VulkanContext::Get()->GetDevice();

        // Wait for GPU
        device.WaitIdle();

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = VulkanContext::Get()->GetSurface();
        createInfo.minImageCount = RendererSettings::FramesInFlight;
        createInfo.imageFormat = m_SwapChainFormat.format;
        createInfo.imageColorSpace = m_SwapChainFormat.colorSpace;
        createInfo.imageExtent = { width, height };
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = VulkanContext::Get()->GetSurfaceCapabilities().currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // V-Sync
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = m_Swapchain; // Using old swapchain
        createInfo.imageSharingMode = m_QueueShareMode;
        if (m_QueueShareMode == VK_SHARING_MODE_CONCURRENT)
        {
            createInfo.queueFamilyIndexCount = static_cast<u32>(m_SwapchainQueues.size());
            createInfo.pQueueFamilyIndices = m_SwapchainQueues.data();
        }

        // Create new swapchain
        VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
        TBO_VK_ASSERT(vkCreateSwapchainKHR(device, &createInfo, nullptr, &newSwapchain));
        
        // Get swapchain images
        u32 imageCount;
        TBO_VK_ASSERT(vkGetSwapchainImagesKHR(device, newSwapchain, &imageCount, nullptr));
        TBO_ENGINE_ASSERT(imageCount <= RendererSettings::FramesInFlight);
        TBO_VK_ASSERT(vkGetSwapchainImagesKHR(device, newSwapchain, &imageCount, m_Images.Data()));

        // Destroy old stuff if exists
        if (m_Swapchain)
            Cleanup();

        // Assign new swapchain
        m_Swapchain = newSwapchain;

        // Create swapchain imageviews
        CreateImageviews();

        // Create swapchain framebuffers
        CreateFramebuffers(width, height);

        TBO_ENGINE_WARN("Swapchain resized! {0}, {1}", width, height);
    }

    void VulkanSwapChain::NewFrame()
    {
        TBO_PROFILE_FUNC();

        VkSemaphore currentSemaphore = m_PresentSemaphores[m_CurrentFrame];
        TBO_VK_ASSERT(vkAcquireNextImageKHR(VulkanContext::Get()->GetDevice(), m_Swapchain, UINT64_MAX, currentSemaphore, VK_NULL_HANDLE, &m_ImageIndex));
    }

    void VulkanSwapChain::SwapFrame()
    {
        TBO_PROFILE_FUNC();

        SubmitCommandBuffers();
        PresentFrame();
    }

    void VulkanSwapChain::SubmitCommandBuffers()
    {
        VulkanDevice& device = VulkanContext::Get()->GetDevice();

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_RenderCommandBuffers[m_CurrentFrame];
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_PresentSemaphores[m_CurrentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame];
        submitInfo.pWaitDstStageMask = &waitStage;

        TBO_VK_ASSERT(vkResetFences(device, 1, &m_InFlightFences[m_CurrentFrame]));
        TBO_VK_ASSERT(vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]));
    }

    void VulkanSwapChain::PresentFrame()
    {
        VulkanDevice& device = VulkanContext::Get()->GetDevice();

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame];  // Wait for the frame to render
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_Swapchain;
        presentInfo.pImageIndices = &m_ImageIndex;

        VkResult result = vkQueuePresentKHR(device.GetPresentQueue(), &presentInfo);
        if (result != VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
        {
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                //Resize();
                return;
            }

            // Something went wrong
            TBO_ENGINE_ASSERT(false);
        }

        // Wait for the previous frame to finish - blocks cpu until signaled
        TBO_VK_ASSERT(vkWaitForFences(device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX));

        // Cycle frames in flights
        m_CurrentFrame = (m_CurrentFrame + 1) % RendererSettings::FramesInFlight;
    }

    void VulkanSwapChain::CreateRenderpass()
    {
        VkDevice device = VulkanContext::Get()->GetDevice();

        // Attachment description
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = m_SwapChainFormat.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        // Subpasses and attachment references
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttachmentRef;
        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;
        subpassDescription.pResolveAttachments = nullptr;

        // Render pass 
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;

        // Self-dependency
        VkSubpassDependency subpassDependency = {};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &subpassDependency;

        TBO_VK_ASSERT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_Renderpass));
    }

    void VulkanSwapChain::CreateSyncObjects()
    {
        VkDevice device = VulkanContext::Get()->GetDevice();

        // Present and RenderFinished 
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.flags = 0;

        // Fences
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (u32 i = 0; i < RendererSettings::FramesInFlight; ++i)
        {
            TBO_VK_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_PresentSemaphores[i]));
            TBO_VK_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));
            TBO_VK_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFences[i]));
        }
    }

    void VulkanSwapChain::CreateImageviews()
    {
        VkDevice device = VulkanContext::Get()->GetDevice();

        for (u32 i = 0; i < RendererSettings::FramesInFlight; ++i)
        {
            VkImageViewCreateInfo imageViewInfo = {};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.image = m_Images[i];
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.format = m_SwapChainFormat.format;
            imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            TBO_VK_ASSERT(vkCreateImageView(device, &imageViewInfo, nullptr, &m_Imageviews[i]));
        }
    }

    void VulkanSwapChain::CreateFramebuffers(u32 width, u32 height)
    {
        VkDevice device = VulkanContext::Get()->GetDevice();

        for (u32 i = 0; i < RendererSettings::FramesInFlight; ++i)
        {
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_Renderpass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &m_Imageviews[i]; // Color attachment
            framebufferInfo.width = width;
            framebufferInfo.height = height;
            framebufferInfo.layers = 1;

            TBO_VK_ASSERT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_Framebuffers[i]));
        }
    }

}
