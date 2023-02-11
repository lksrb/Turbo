#include "tbopch.h"

#include "VulkanSwapChain.h"

#include "Turbo/Core/Engine.h"
#include "Turbo/Core/Window.h"

#include "Turbo/Renderer/GPUDevice.h"

#include "Turbo/Platform/Vulkan/VulkanImage2D.h"

namespace Turbo
{
    VulkanSwapChain::VulkanSwapChain()
        : m_Swapchain(VK_NULL_HANDLE), m_Renderpass(VK_NULL_HANDLE),
        m_RenderFinishedSemaphores{ VK_NULL_HANDLE }, m_PresentSemaphores{ VK_NULL_HANDLE },
        m_InFlightFences{ VK_NULL_HANDLE },
        m_Imageviews{ VK_NULL_HANDLE }, m_Images{ VK_NULL_HANDLE },
        m_Framebuffers{ VK_NULL_HANDLE }, m_RenderCommandBuffers{ VK_NULL_HANDLE },
        m_CurrentFrame(0), m_ImageIndex(0)
    {
        Initialize();
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        Shutdown();
    }

    void VulkanSwapChain::Initialize()
    {
        m_SwapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;

        // TODO: Remove depth buffer 
        Image2D::Config imageConfig = {};
        imageConfig.ImageFormat = Image2D::Format_D32_SFloat_S8_Uint;
        imageConfig.Aspect = Image2D::AspectFlags_Depth;
        imageConfig.Storage = Image2D::MemoryPropertyFlags_DeviceLocal;
        imageConfig.Usage = Image2D::ImageUsageFlags_DepthStencilSttachment;
        imageConfig.ImageTiling = Image2D::ImageTiling_Optimal;
        m_DepthBuffer = Image2D::Create(imageConfig);
        m_DepthBuffer->Invalidate(1600, 900);

        m_Renderpass = RenderPass::Create({ m_DepthBuffer }); 

        CreateRenderpass();
        CreateSyncObjects();

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = RendererContext::GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(RendererContext::FramesInFlight());

        TBO_VK_ASSERT(vkAllocateCommandBuffers(RendererContext::GetDevice(),
            &allocInfo, m_RenderCommandBuffers));

        /* // Depth buffer
        {
            m_DepthBuffer->Invalidate(width, height);
        }*/

        // Swapchain, renderpass, ... are created on the first resize
    }

    void VulkanSwapChain::Shutdown()
    {
        VkDevice device = RendererContext::GetDevice();

        Cleanup();

        //m_DepthBuffer.Reset();
        //delete m_Renderpass;
        //vkDestroySurfaceKHR(RendererContext::GetInstance(), I->Surface, nullptr);
        //vkDestroyRenderPass(device, m_Renderpass, nullptr);

        for (size_t i = 0; i < RendererContext::FramesInFlight(); i++)
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
        {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
        for (auto imageView : m_Imageviews)
        {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, m_Swapchain, nullptr);

        m_Swapchain = VK_NULL_HANDLE;
    }

    void VulkanSwapChain::Resize(u32 width, u32 height)
    {
        VkDevice device = RendererContext::GetDevice();

        VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;

        auto& physicalDeviceSupp = RendererContext::GetSwapchainSupportDetails();
        auto& indices = RendererContext::GetQueueFamilyIndices();

        vkDeviceWaitIdle(device);

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = RendererContext::GetSurface();
        createInfo.minImageCount = physicalDeviceSupp.nMinImageCount;
        createInfo.imageFormat = m_SwapchainFormat;
        createInfo.imageColorSpace = physicalDeviceSupp.surfaceFormat.colorSpace;
        createInfo.imageExtent = { width, height };
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = physicalDeviceSupp.capabilities.currentTransform;
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

        uint32_t imageCount;
        TBO_VK_ASSERT(vkGetSwapchainImagesKHR(device, newSwapchain, &imageCount, nullptr));
        TBO_ENGINE_ASSERT(imageCount <= 3);
        TBO_VK_ASSERT(vkGetSwapchainImagesKHR(device, newSwapchain, &imageCount, m_Images));

        // Destroy old stuff if exists
        if (m_Swapchain)
            Cleanup();

        // Assign new swapchain
        m_Swapchain = newSwapchain;

        m_DepthBuffer->Invalidate(width, height);

        CreateImageviews();
        CreateFramebuffers(width, height);

        TBO_ENGINE_WARN("Swapchain resized! {0}, {1}", width, height);
    }

    void VulkanSwapChain::SubmitSecondary(VkCommandBuffer bufferToSubmit)
    {
        m_SecondaryCommandBuffers.push_back(bufferToSubmit);
    }

    VkCommandBuffer VulkanSwapChain::GetCurrentRenderCommandBuffer() const
    {
        return m_RenderCommandBuffers[m_CurrentFrame];
    }

    VkFramebuffer VulkanSwapChain::GetCurrentFramebuffer() const
    {
        return m_Framebuffers[m_ImageIndex];
    }

    Ref<RenderPass> VulkanSwapChain::GetRenderPass() const
    {
        return m_Renderpass;
    }

    u32 VulkanSwapChain::GetCurrentFrame() const
    {
        return m_CurrentFrame;
    }

    void VulkanSwapChain::NewFrame()
    {
        u32 currentFrame = m_CurrentFrame;
        VkSemaphore currentSemaphore = m_PresentSemaphores[currentFrame];
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

        // Get frame info from renderer
        u32 currentFrame = m_CurrentFrame;
        const Window* viewportWindow = Engine::Get().GetViewportWindow();
        u32 width = viewportWindow->GetWidth();
        u32 height = viewportWindow->GetHeight();

        // Record render command buffer and execute all secondary command buffers from renderers
        {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            beginInfo.pInheritanceInfo = nullptr;
            VkCommandBuffer currentBuffer = GetCurrentRenderCommandBuffer();
            TBO_VK_ASSERT(vkBeginCommandBuffer(currentBuffer, &beginInfo));
            {
                VkClearValue clearValues[2]{};
                clearValues[0].color = { {0.0f, 0.0f,0.0f, 1.0f} };
                clearValues[1].depthStencil = { 1.0f, 0 };

                VkRenderPassBeginInfo renderPassBeginInfo = {};
                renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassBeginInfo.renderPass = m_Renderpass.As<VulkanRenderPass>()->GetRenderPass();
                renderPassBeginInfo.renderArea.offset.x = 0;
                renderPassBeginInfo.renderArea.offset.y = 0;
                renderPassBeginInfo.renderArea.extent = { width, height };
                renderPassBeginInfo.clearValueCount = 2; // Color
                renderPassBeginInfo.pClearValues = clearValues;
                renderPassBeginInfo.framebuffer = m_Framebuffers[m_ImageIndex];

                vkCmdBeginRenderPass(currentBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

                // Execute all submitted secondary command buffers
                {
                    TBO_ENGINE_ASSERT(m_SecondaryCommandBuffers.size(), "Cannot submit 0 command buffers!");

                    vkCmdExecuteCommands(currentBuffer, static_cast<uint32_t>(m_SecondaryCommandBuffers.size()), m_SecondaryCommandBuffers.data());
                    m_SecondaryCommandBuffers.clear();

                }
                vkCmdEndRenderPass(currentBuffer);
            }

            TBO_VK_ASSERT(vkEndCommandBuffer(currentBuffer));
        }


        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_RenderCommandBuffers[currentFrame];
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_PresentSemaphores[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphores[currentFrame];
        submitInfo.pWaitDstStageMask = &waitStage;

        TBO_VK_ASSERT(vkResetFences(device, 1, &m_InFlightFences[currentFrame]));
        TBO_VK_ASSERT(vkQueueSubmit(RendererContext::GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[currentFrame]));
    }

    void VulkanSwapChain::PresentFrame()
    {
        VkDevice device = RendererContext::GetDevice();

        VkQueue presentQueue = RendererContext::GetPresentQueue();
        u32 currentFrame = m_CurrentFrame;

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[currentFrame];  // Wait for the frame to render
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
        TBO_VK_ASSERT(vkWaitForFences(device, 1, &m_InFlightFences[currentFrame], VK_TRUE, UINT64_MAX)); // TODO: Fix resizing

        // Cycle frames in flights
        m_CurrentFrame = (m_CurrentFrame + 1) % RendererContext::FramesInFlight();
    }

    void VulkanSwapChain::CreateRenderpass()
    {
        /* // Attachment description
         VkAttachmentDescription colorAttachment{};
         colorAttachment.format = m_SwapchainFormat;
         colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
         colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
         colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
         colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
         colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
         colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
         colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
         // Subpasses and attachment references
         VkAttachmentReference colorAttachmentRef{};
         colorAttachmentRef.attachment = 0;
         colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

         VkSubpassDescription subpassDescription{};
         subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
         subpassDescription.colorAttachmentCount = 1;
         subpassDescription.pColorAttachments = &colorAttachmentRef;
         subpassDescription.inputAttachmentCount = 0;
         subpassDescription.pInputAttachments = nullptr;
         subpassDescription.preserveAttachmentCount = 0;
         subpassDescription.pPreserveAttachments = nullptr;
         subpassDescription.pResolveAttachments = nullptr;

         // Render pass
         VkRenderPassCreateInfo renderPassInfo{};
         renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
         renderPassInfo.attachmentCount = 1;
         renderPassInfo.pAttachments = &colorAttachment;
         renderPassInfo.subpassCount = 1;
         renderPassInfo.pSubpasses = &subpassDescription;

         VkSubpassDependency dependency{};
         dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
         dependency.dstSubpass = 0;
         dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
         dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
         dependency.srcAccessMask = 0;
         dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

         renderPassInfo.dependencyCount = 1;
         renderPassInfo.pDependencies = &dependency;
         renderPassInfo.pSubpasses;

         TBO_VK_ASSERT(vkCreateRenderPass(RendererContext::GetDevice(), &renderPassInfo, nullptr, &m_Renderpass));*/

        VkDevice device = RendererContext::GetDevice();

        m_Renderpass->Invalidate();

        /*  std::vector<VkAttachmentDescription> attachments;

          // Color attachment
          {
              auto& colorAttachment = attachments.emplace_back();
              colorAttachment.format = m_SwapchainFormat;
              colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
              colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
              colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
              colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
              colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
              colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
              colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
          }

          // Depth buffer
          {
              auto& depthAttachment = attachments.emplace_back();
              depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
              depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
              depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
              depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
              depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
              depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
              depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
              depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
          }


          // Subpasses and attachment references
          VkAttachmentReference colorAttachmentRef{};
          colorAttachmentRef.attachment = 0;
          colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

          // Depth buffer attachment reference
          VkAttachmentReference depthAttachmentRef{};
          depthAttachmentRef.attachment = 1;
          depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

          VkSubpassDescription subpassDescription{};
          subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
          subpassDescription.colorAttachmentCount = 1;
          subpassDescription.pColorAttachments = &colorAttachmentRef;
          subpassDescription.inputAttachmentCount = 0;
          subpassDescription.pInputAttachments = VK_NULL_HANDLE;
          subpassDescription.preserveAttachmentCount = 0;
          subpassDescription.pPreserveAttachments = VK_NULL_HANDLE;
          subpassDescription.pResolveAttachments = VK_NULL_HANDLE;
          subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;

          // Dependencies
          VkSubpassDependency dependency{};
          dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
          dependency.dstSubpass = 0;
          dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
          dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
          dependency.srcAccessMask = 0;
          dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
          dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

          // Render pass
          VkRenderPassCreateInfo renderPassInfo{};
          renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

          // Attachments
          renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
          renderPassInfo.pAttachments = attachments.data();

          // Subpasses
          renderPassInfo.subpassCount = 1;
          renderPassInfo.pSubpasses = &subpassDescription;

          // Dependencies
          renderPassInfo.dependencyCount = 1;
          renderPassInfo.pDependencies = &dependency;

          TBO_VK_ASSERT(vkCreateRenderPass(device, &renderPassInfo, VK_NULL_HANDLE, &m_Renderpass));*/
    }

    void VulkanSwapChain::CreateSyncObjects()
    {
        VkDevice device = RendererContext::GetDevice();

        // Present and RenderFinished 
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.flags = 0;

        // Fences
        VkFenceCreateInfo fenceInfo{};
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
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_Images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_SwapchainFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            createInfo.subresourceRange = image_range;
            TBO_VK_ASSERT(vkCreateImageView(device, &createInfo, nullptr, &m_Imageviews[i]));
        }
    }

    void VulkanSwapChain::CreateFramebuffers(u32 width, u32 height)
    {
        VkDevice device = RendererContext::GetDevice();

        for (size_t i = 0; i < RendererContext::FramesInFlight(); i++)
        {
            VkImageView attachments[] = {
                m_Imageviews[i], // Color attachment 
                m_DepthBuffer.As<VulkanImage2D>()->GetImageView()    // Depth buffer
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_Renderpass.As<VulkanRenderPass>()->GetRenderPass();
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = width;
            framebufferInfo.height = height;
            framebufferInfo.layers = 1;

            TBO_VK_ASSERT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_Framebuffers[i]));
        }
    }

}
