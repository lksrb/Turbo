#pragma once

#include "Turbo/Renderer/RendererContext.h"
#include "Turbo/Renderer/SwapChain.h"

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

        void SubmitSecondary(VkCommandBuffer bufferToSubmit);
        
        VkCommandBuffer GetCurrentRenderCommandBuffer() const;
        VkFramebuffer GetCurrentFramebuffer() const;
        VkRenderPass GetRenderPass() const;
        u32 GetCurrentFrame() const;
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
        VkSwapchainKHR m_Swapchain;
        VkRenderPass m_Renderpass;
        VkSemaphore m_RenderFinishedSemaphores[TBO_MAX_FRAMESINFLIGHT];
        VkSemaphore m_PresentSemaphores[TBO_MAX_FRAMESINFLIGHT];
        VkFence m_InFlightFences[TBO_MAX_FRAMESINFLIGHT];
        VkImageView m_Imageviews[TBO_MAX_FRAMESINFLIGHT];
        VkImage m_Images[TBO_MAX_FRAMESINFLIGHT];
        VkFramebuffer m_Framebuffers[TBO_MAX_FRAMESINFLIGHT];
        VkCommandBuffer m_RenderCommandBuffers[TBO_MAX_FRAMESINFLIGHT];

        VkFormat m_SwapchainFormat;

        u32 m_CurrentFrame;
        u32 m_ImageIndex;

        std::vector<VkCommandBuffer> m_SecondaryCommandBuffers;
    };

}
