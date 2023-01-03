#pragma once

#include "Turbo/UI/UserInterface.h"

#include "Turbo/Renderer/RendererContext.h"

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
        void ImGuiStyleSpectrum();
        void CreateImGuiContext();
    private:
        bool m_BlockEvents;
        // ImGui vulkan objects
        VkDescriptorPool m_DescriptorPool;
        VkRenderPass m_RenderPass;
        VkCommandBuffer m_SecondaryBuffers[TBO_MAX_FRAMESINFLIGHT];
    };
}
