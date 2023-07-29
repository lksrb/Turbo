#pragma once

#include "Turbo/Renderer/IndexBuffer.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanIndexBuffer : public IndexBuffer
    {
    public:
        VulkanIndexBuffer(const u32* indices, u32 count);
        ~VulkanIndexBuffer() = default;
        
        VkBuffer GetHandle() const { return m_Buffer; }
    private:
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_BufferMemory = VK_NULL_HANDLE;
    };
}

