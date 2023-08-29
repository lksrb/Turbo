#pragma once

#include "Turbo/Renderer/RendererContext.h"
#include "Turbo/Renderer/SwapChain.h"
#include "Turbo/Renderer/Image2D.h"
#include "Turbo/Renderer/RenderPass.h"
#include "Turbo/Renderer/Fly.h"

#include "Turbo/Platform/Vulkan/VulkanRenderPass.h"
#include "Turbo/Platform/Vulkan/VulkanFrameBuffer.h"

#include <vulkan/vulkan.h>

namespace Turbo 
{
    class VulkanSwapChain : public SwapChain
    {
    public:
        VulkanSwapChain();
        ~VulkanSwapChain();

        void NewFrame() override;
        void SwapFrame() override;

        void Resize(u32 width, u32 height) override;

        VkCommandBuffer GetCurrentRenderCommandBuffer() const { return m_RenderCommandBuffers[m_CurrentFrame]; }
        VkFramebuffer GetCurrentFramebuffer() const { return m_Framebuffers[m_ImageIndex]; }
        VkRenderPass GetRenderPass() const { return m_Renderpass; }

        u32 GetCurrentFrame() const override { return m_CurrentFrame; }
        u32 GetCurrentImageIndex() const override { return m_ImageIndex; }
    private:
        void Initialize();
        void Shutdown();

        void CreateRenderpass();
        void CreateSyncObjects();

        void CreateImageviews();
        void CreateFramebuffers(u32 width, u32 height);

        void SubmitCommandBuffers();
        void PresentFrame();

        void Cleanup();
    private:
        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        VkRenderPass m_Renderpass = VK_NULL_HANDLE;

        Fly<VkSemaphore> m_RenderFinishedSemaphores;
        Fly<VkSemaphore> m_PresentSemaphores;
        Fly<VkFence> m_InFlightFences;
        Fly<VkImageView> m_Imageviews;
        Fly<VkImage> m_Images;
        Fly<VkFramebuffer> m_Framebuffers;
        Fly<VkCommandBuffer> m_RenderCommandBuffers;

        std::array<u32, 2> m_SwapchainQueues;
        VkSharingMode m_QueueShareMode;
        VkSurfaceFormatKHR m_SwapChainFormat;

        u32 m_CurrentFrame = 0;
        u32 m_ImageIndex = 0;
    };

}
