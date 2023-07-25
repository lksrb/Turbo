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

        VkBuffer GetHandle() const { return m_Buffer; }
        VkDeviceMemory GetMemory() const { return m_Memory; }

        void SetData(const void* data) override;
    private:
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
    };
}
