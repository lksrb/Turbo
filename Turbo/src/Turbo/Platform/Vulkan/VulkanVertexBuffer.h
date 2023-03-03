#pragma once

#include "Turbo/Renderer/CommandBuffer.h"
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
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;

        VkBuffer m_StagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_StagingBufferMemory = VK_NULL_HANDLE;
        void* m_StagingBufferPtr = nullptr;

        Ref<CommandBuffer> m_TranferCommandBuffer;
    };
}
