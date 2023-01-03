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

        VkCommandBuffer GetCommandBuffer() const { return m_CommandBuffer; }

        void Begin() override;
        void End() override;
    private:
        VkCommandBuffer m_CommandBuffer;
    };
}
