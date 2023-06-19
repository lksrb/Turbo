#pragma once

#include "Turbo/Renderer/RenderCommandBuffer.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanRenderCommandBuffer : public RenderCommandBuffer
    {
    public:
        VulkanRenderCommandBuffer();
        ~VulkanRenderCommandBuffer();

        VkCommandBuffer GetHandle() const;

        void Begin() override;
        void End() override;

        void Submit() override;
    private:
        std::vector<VkFence> m_WaitFences;
        std::vector<VkCommandBuffer> m_CommandBuffers;
    };
}
