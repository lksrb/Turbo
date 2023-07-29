#pragma once

#include "Turbo/Renderer/RendererContext.h"
#include "Turbo/Renderer/VertexBuffer.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanVertexBuffer : public VertexBuffer
    {
    public:
        VulkanVertexBuffer(const void* vertices, u64 size);
        ~VulkanVertexBuffer();

        void SetData(Ref<RenderCommandBuffer> commandBuffer, const void* vertices, u64 size) override;
        VkBuffer GetHandle() const { return m_Buffer; }
    private:
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_BufferMemory = VK_NULL_HANDLE;

        VkBuffer m_StagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_StagingBufferMemory = VK_NULL_HANDLE;
        void* m_StagingBufferPtr = nullptr;
    };
}
