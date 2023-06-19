#pragma once

#include "Turbo/Renderer/UniformBuffer.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanUniformBuffer : public UniformBuffer
    {
    public:
        VulkanUniformBuffer(const UniformBuffer::Config& config);
        ~VulkanUniformBuffer();

        void Invalidate() override;

        VkBuffer GetHandle() const { return m_Buffer; }
        void SetData(const void* data) override;
    private:
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
    };
}
