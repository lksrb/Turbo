#include "tbopch.h"

#include "VulkanSwapChain.h"

#include "Turbo/Core/Engine.h"
#include "Turbo/Core/Window.h"

#include "Turbo/Renderer/GPUDevice.h"

#include "Turbo/Platform/Vulkan/VulkanImage2D.h"
#include "Turbo/Platform/Vulkan/VulkanUtils.h"

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
        u32 framesInFlight = RendererContext::FramesInFlight();

        m_RenderFinishedSemaphores.resize(framesInFlight);
        m_PresentSemaphores.resize(framesInFlight);
        m_InFlightFences.resize(framesInFlight);
        m_Imageviews.resize(framesInFlight);
        m_Images.resize(framesInFlight);
        m_Framebuffers.resize(framesInFlight);
        m_RenderCommandBuffers.resize(framesInFlight);

        m_SwapchainFormat = Vulkan::SelectSurfaceFormat().format; // VK_FORMAT_B8G8R8A8_UNORM

        CreateRenderpass();
        CreateSyncObjects();

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = RendererContext::GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<u32>(framesInFlight);

        TBO_VK_ASSERT(vkAllocateCommandBuffers(RendererContext::GetDevice(),
            &allocInfo, m_RenderCommandBuffers.data()));

        // Swapchain, renderpass, etc. are created on the first resize
    }

    void VulkanSwapChain::Shutdown()
    {
        VkDevice device = RendererContext::GetDevice();

        vkDestroyRenderPass(device, m_Renderpass, nullptr);

        Cleanup();

        for (u32 i = 0; i < RendererContext::FramesInFlight(); ++i)
        {
            vkDestroySemaphore(device, m_PresentSemaphores[i], nullptr);
            vkDestroySemaphore(device, m_RenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, m_InFlightFences[i], nullptr);
        }
    }

    void VulkanSwapChain::Cleanup()
    {
        VkDevice device = RendererContext::GetDevice();

        for (auto framebuffer : m_Framebuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);

        for (auto imageView : m_Imageviews)
            vkDestroyImageView(device, imageView, nullptr);

        vkDestroySwapchainKHR(device, m_Swapchain, nullptr);

        m_Swapchain = VK_NULL_HANDLE;
    }

    void VulkanSwapChain::Resize(u32 width, u32 height)
    {
        VkDevice device = RendererContext::GetDevice();

        VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;

        const SwapchainSupportDetails& deviceDetails = RendererContext::GetSwapchainSupportDetails();
        const QueueFamilyIndices& indices = RendererContext::GetQueueFamilyIndices();

        // Wait for GPU
        vkDeviceWaitIdle(device);

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = RendererContext::GetSurface();
        createInfo.minImageCount = RendererContext::FramesInFlight();
        createInfo.imageFormat = m_SwapchainFormat;
        createInfo.imageColorSpace = deviceDetails.SurfaceFormat.colorSpace;
        createInfo.imageExtent = { width, height };
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = deviceDetails.Capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // V-Sync
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = m_Swapchain; // Using old swapchain

        if (indices.GraphicsFamily != indices.PresentFamily)
        {
            u32 queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        TBO_VK_ASSERT(vkCreateSwapchainKHR(device, &createInfo, nullptr, &newSwapchain));

        u32 imageCount;
        TBO_VK_ASSERT(vkGetSwapchainImagesKHR(device, newSwapchain, &imageCount, nullptr));
        TBO_ENGINE_ASSERT(imageCount <= RendererContext::FramesInFlight());
        TBO_VK_ASSERT(vkGetSwapchainImagesKHR(device, newSwapchain, &imageCount, m_Images.data()));

        // Destroy old stuff if exists
        if (m_Swapchain)
            Cleanup();

        // Assign new swapchain
        m_Swapchain = newSwapchain;

        CreateImageviews();
        CreateFramebuffers(width, height);

        TBO_ENGINE_WARN("Swapchain resized! {0}, {1}", width, height);
    }

    void VulkanSwapChain::NewFrame()
    {
        VkSemaphore currentSemaphore = m_PresentSemaphores[m_CurrentFrame];
        TBO_VK_ASSERT(vkAcquireNextImageKHR(RendererContext::GetDevice(), m_Swapchain, UINT64_MAX, currentSemaphore, VK_NULL_HANDLE, &m_ImageIndex));
    }

    void VulkanSwapChain::SwapFrame()
    {
        SubmitCommandBuffers();
        PresentFrame();
    }

    void VulkanSwapChain::SubmitCommandBuffers()
    {
        VkDevice device = RendererContext::GetDevice();

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
        TBO_VK_ASSERT(vkQueueSubmit(RendererContext::GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]));
    }

    void VulkanSwapChain::PresentFrame()
    {
        VkDevice device = RendererContext::GetDevice();

        VkQueue presentQueue = RendererContext::GetPresentQueue();

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame];  // Wait for the frame to render
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_Swapchain;
        presentInfo.pImageIndices = &m_ImageIndex;

        VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
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
        m_CurrentFrame = (m_CurrentFrame + 1) % RendererContext::FramesInFlight();
    }

    void VulkanSwapChain::CreateRenderpass()
    {
        VkDevice device = RendererContext::GetDevice();

        // Attachment description
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = m_SwapchainFormat;
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
        VkDevice device = RendererContext::GetDevice();

        // Present and RenderFinished 
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.flags = 0;

        // Fences
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (u32 i = 0; i < RendererContext::FramesInFlight(); ++i)
        {
            TBO_VK_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_PresentSemaphores[i]));
            TBO_VK_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));
            TBO_VK_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFences[i]));
        }
    }

    void VulkanSwapChain::CreateImageviews()
    {
        VkDevice device = RendererContext::GetDevice();

        for (u32 i = 0; i < RendererContext::FramesInFlight(); ++i)
        {
            VkImageViewCreateInfo imageViewInfo = {};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.image = m_Images[i];
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.format = m_SwapchainFormat;
            imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            imageViewInfo.subresourceRange = image_range;
            TBO_VK_ASSERT(vkCreateImageView(device, &imageViewInfo, nullptr, &m_Imageviews[i]));
        }
    }

    void VulkanSwapChain::CreateFramebuffers(u32 width, u32 height)
    {
        VkDevice device = RendererContext::GetDevice();

        for (u32 i = 0; i < RendererContext::FramesInFlight(); ++i)
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
