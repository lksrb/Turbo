#pragma once

#include "Turbo/Renderer/IndexBuffer.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanIndexBuffer : public IndexBuffer
    {
    public:
        VulkanIndexBuffer(const IndexBuffer::Config& config);
        ~VulkanIndexBuffer();
        
        VkBuffer GetHandle() const { return m_Buffer; }
    private:
        VkBuffer m_Buffer;
        VkDeviceMemory m_Memory;
    };
}

