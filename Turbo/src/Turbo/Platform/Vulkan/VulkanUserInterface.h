#pragma once

#include "Turbo/UI/UserInterface.h"
#include "Turbo/Renderer/Fly.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanUserInterface : public UserInterface
    {
    public:
        VulkanUserInterface();
        ~VulkanUserInterface();

        void BeginUI() override;
        void EndUI() override;
        void OnEvent(Event& e) override;
    private:
        void CreateImGuiContext();
    private:
        // ImGui vulkan objects
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
        Fly<VkCommandBuffer> m_SecondaryBuffers;
    };
}
