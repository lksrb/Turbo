#pragma once

#include "Turbo/Renderer/RenderCommandBuffer.h"
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

        void SetData(void* data, size_t size) override;

        VkBuffer GetHandle() const { return m_Buffer; }
    private:
        void TransferData(size_t size);
    private:
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_BufferMemory = VK_NULL_HANDLE;

        VkBuffer m_StagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_StagingBufferMemory = VK_NULL_HANDLE;
        void* m_StagingBufferPtr = nullptr;

        Ref<RenderCommandBuffer> m_TranferCommandBuffer;
    };
}
