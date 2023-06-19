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

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = RendererContext::GetCommandPool();
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = static_cast<uint32_t>(RendererContext::FramesInFlight());

        TBO_VK_ASSERT(vkAllocateCommandBuffers(RendererContext::GetDevice(),
            &alloc_info, m_RenderCommandBuffers.data()));

        // Swapchain, renderpass, etc. are created/invalided on the first resize
    }

    void VulkanSwapChain::Shutdown()
    {
        VkDevice device = RendererContext::GetDevice();

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
        createInfo.imageColorSpace = deviceDetails.surfaceFormat.colorSpace;
        createInfo.imageExtent = { width, height };
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = deviceDetails.capabilities.currentTransform;
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

        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_RenderCommandBuffers[m_CurrentFrame];
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &m_PresentSemaphores[m_CurrentFrame];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame];
        submit_info.pWaitDstStageMask = &wait_stage;

        TBO_VK_ASSERT(vkResetFences(device, 1, &m_InFlightFences[m_CurrentFrame]));
        TBO_VK_ASSERT(vkQueueSubmit(RendererContext::GetGraphicsQueue(), 1, &submit_info, m_InFlightFences[m_CurrentFrame]));
    }

    void VulkanSwapChain::PresentFrame()
    {
        VkDevice device = RendererContext::GetDevice();

        VkQueue present_queue = RendererContext::GetPresentQueue();

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame];  // Wait for the frame to render
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &m_Swapchain;
        present_info.pImageIndices = &m_ImageIndex;

        VkResult result = vkQueuePresentKHR(present_queue, &present_info);
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
        VkAttachmentDescription color_attachment = {};
        color_attachment.format = m_SwapchainFormat;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        // Subpasses and attachment references
        VkAttachmentReference color_attachment_ref = {};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass_desc = {};
        subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc.colorAttachmentCount = 1;
        subpass_desc.pColorAttachments = &color_attachment_ref;
        subpass_desc.inputAttachmentCount = 0;
        subpass_desc.pInputAttachments = nullptr;
        subpass_desc.preserveAttachmentCount = 0;
        subpass_desc.pPreserveAttachments = nullptr;
        subpass_desc.pResolveAttachments = nullptr;

        // Render pass 
        VkRenderPassCreateInfo renderpass_info = {};
        renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_info.attachmentCount = 1;
        renderpass_info.pAttachments = &color_attachment;
        renderpass_info.subpassCount = 1;
        renderpass_info.pSubpasses = &subpass_desc;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        renderpass_info.dependencyCount = 1;
        renderpass_info.pDependencies = &dependency;
        renderpass_info.pSubpasses;

        TBO_VK_ASSERT(vkCreateRenderPass(device, &renderpass_info, nullptr, &m_Renderpass));

        // Add it to deletion queue 
        auto& resource_free_queue = RendererContext::GetResourceQueue();
        resource_free_queue.Submit(RENDERPASS, [device, render_pass = m_Renderpass]()
        {
            vkDestroyRenderPass(device, render_pass, nullptr);
        });
    }

    void VulkanSwapChain::CreateSyncObjects()
    {
        VkDevice device = RendererContext::GetDevice();

        // Present and RenderFinished 
        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphore_info.flags = 0;

        // Fences
        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (u32 i = 0; i < RendererContext::FramesInFlight(); ++i)
        {
            TBO_VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, nullptr, &m_PresentSemaphores[i]));
            TBO_VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, nullptr, &m_RenderFinishedSemaphores[i]));
            TBO_VK_ASSERT(vkCreateFence(device, &fence_info, nullptr, &m_InFlightFences[i]));
        }
    }

    void VulkanSwapChain::CreateImageviews()
    {
        VkDevice device = RendererContext::GetDevice();

        for (u32 i = 0; i < RendererContext::FramesInFlight(); ++i)
        {
            VkImageViewCreateInfo imageview_info{};
            imageview_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageview_info.image = m_Images[i];
            imageview_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageview_info.format = m_SwapchainFormat;
            imageview_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageview_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageview_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageview_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            imageview_info.subresourceRange = image_range;
            TBO_VK_ASSERT(vkCreateImageView(device, &imageview_info, nullptr, &m_Imageviews[i]));
        }
    }

    void VulkanSwapChain::CreateFramebuffers(u32 width, u32 height)
    {
        VkDevice device = RendererContext::GetDevice();

        for (u32 i = 0; i < RendererContext::FramesInFlight(); ++i)
        {
            VkFramebufferCreateInfo framebuffer_info{};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = m_Renderpass;
            framebuffer_info.attachmentCount = 1;
            framebuffer_info.pAttachments = &m_Imageviews[i]; // Color attachment
            framebuffer_info.width = width;
            framebuffer_info.height = height;
            framebuffer_info.layers = 1;

            TBO_VK_ASSERT(vkCreateFramebuffer(device, &framebuffer_info, nullptr, &m_Framebuffers[i]));
        }
    }

}
