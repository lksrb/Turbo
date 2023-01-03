#pragma once

#include "Turbo/Renderer/RendererBuffer.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanBuffer : public RendererBuffer
    {
    public:
        VulkanBuffer(const RendererBuffer::Config& config);
        ~VulkanBuffer();

        VkBuffer GetBuffer() const { return m_Buffer; }
        VkDeviceMemory GetMemory() const { return m_Memory; }

        void SetData(const void* data) override;
    private:
        VkDeviceMemory m_Memory;
        VkBuffer m_Buffer;
    };
}
