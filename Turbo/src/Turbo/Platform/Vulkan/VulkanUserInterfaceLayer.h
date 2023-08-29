#pragma once

#include "Turbo/UI/UserInterfaceLayer.h"
#include "Turbo/Renderer/Fly.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanUserInterfaceLayer : public UserInterfaceLayer
    {
    public:
        VulkanUserInterfaceLayer() = default;
        ~VulkanUserInterfaceLayer() = default;

        void OnAttach() override;
        void OnDetach() override;

        void Begin() override;
        void End() override;

        void OnEvent(Event& e) override;
    private:
        // ImGui vulkan objects
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        Fly<VkCommandBuffer> m_SecondaryCommandBuffers;
    };
}
