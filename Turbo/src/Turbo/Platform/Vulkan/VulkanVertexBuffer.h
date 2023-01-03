#pragma once

#include "Turbo/Renderer/RendererContext.h"
#include "Turbo/Renderer/VertexBuffer.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanVertexBuffer : public VertexBuffer
    {
    public:
        VulkanVertexBuffer(const VertexBuffer::Config& config);
        ~VulkanVertexBuffer();

        void SetData(void* data, u32 size) override;

        VkBuffer GetBuffer() const { return m_Buffer; }
    private:
        void TransferData(u32 size);
    private:
        VkBuffer m_Buffer;
        VkDeviceMemory m_Memory;

        VkBuffer m_StagingBuffer;
        VkDeviceMemory m_StagingBufferMemory;
        void* m_StagingBufferPtr;
    };
}
