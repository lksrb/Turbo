#pragma once

#include "Turbo/Renderer/RenderCommandBuffer.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanCommandBuffer : public RenderCommandBuffer
    {
    public:
        VulkanCommandBuffer();
        ~VulkanCommandBuffer();

        VkCommandBuffer GetCommandBuffer() const;

        void Begin() override;
        void End() override;

        void Submit() override;
    private:
        std::vector<VkFence> m_WaitFences;
        std::vector<VkCommandBuffer> m_CommandBuffers;
    };
}
