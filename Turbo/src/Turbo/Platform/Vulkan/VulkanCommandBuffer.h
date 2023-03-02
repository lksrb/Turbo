#pragma once

#include "Turbo/Renderer/CommandBuffer.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanCommandBuffer : public CommandBuffer
    {
    public:
        VulkanCommandBuffer(CommandBufferLevel type);
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
