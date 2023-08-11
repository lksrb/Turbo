#include "tbopch.h"
#include "VulkanVertexBuffer.h"

#include "Turbo/Renderer/Renderer.h"
#include "VulkanUtils.h"
#include "VulkanRenderCommandBuffer.h"

namespace Turbo
{
    VulkanVertexBuffer::VulkanVertexBuffer(const void* vertices, u64 size)
    {
        m_Size = size;

        VkDevice device = RendererContext::GetDevice();

        // Staging buffer
        {
            VkBufferCreateInfo bufferCreateInfo = {};
            bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.pNext = nullptr;
            bufferCreateInfo.size = m_Size;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            TBO_VK_ASSERT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &m_StagingBuffer));

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, m_StagingBuffer, &memRequirements);

            VkMemoryAllocateInfo allocateInfo = {};
            allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocateInfo.pNext = nullptr;
            allocateInfo.allocationSize = memRequirements.size;
            allocateInfo.memoryTypeIndex = Vulkan::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            TBO_VK_ASSERT(vkAllocateMemory(device, &allocateInfo, nullptr, &m_StagingBufferMemory));
            TBO_VK_ASSERT(vkBindBufferMemory(device, m_StagingBuffer, m_StagingBufferMemory, 0));
            TBO_VK_ASSERT(vkMapMemory(device, m_StagingBufferMemory, 0, m_Size, 0, &m_StagingBufferPtr));
        }

        // Vertex buffer
        {
            VkBufferCreateInfo bufferCreateInfo = {};
            bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.pNext = nullptr;
            bufferCreateInfo.size = m_Size;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            TBO_VK_ASSERT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &m_Buffer));

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, m_Buffer, &memRequirements);
            
            VkMemoryAllocateInfo allocateInfo = {};
            allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocateInfo.pNext = nullptr;
            allocateInfo.allocationSize = memRequirements.size;
            allocateInfo.memoryTypeIndex = Vulkan::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            TBO_VK_ASSERT(vkAllocateMemory(device, &allocateInfo, nullptr, &m_BufferMemory));
            TBO_VK_ASSERT(vkBindBufferMemory(device, m_Buffer, m_BufferMemory, 0));
        }

        // Dynamic types will be used throughout the entire runtime - a render command buffer is needed
        // Static types will create a temporary command buffer to send data into GPU
        m_Type = VertexBufferType::Dynamic;

        if (vertices)
        {
            m_Type = VertexBufferType::Static;

            memcpy(m_StagingBufferPtr, vertices, size);

            RendererContext::ImmediateSubmit([this, vertices, size](VkCommandBuffer commandBuffer)
            {
                VkBufferCopy copyRegion{};
                copyRegion.srcOffset = 0; // Optional
                copyRegion.dstOffset = 0; // Optional
                copyRegion.size = size;
                vkCmdCopyBuffer(commandBuffer, m_StagingBuffer, m_Buffer, 1, &copyRegion);
            });

            // Destroying staging buffer since its no longer needed
            vkUnmapMemory(device, m_StagingBufferMemory);
            vkFreeMemory(device, m_StagingBufferMemory, nullptr);
            vkDestroyBuffer(device, m_StagingBuffer, nullptr);

            m_StagingBuffer = VK_NULL_HANDLE;
            m_StagingBufferMemory = VK_NULL_HANDLE;
            m_StagingBufferPtr = nullptr;
        }

        // Add it to the resource free queue
        RendererContext::SubmitResourceFree([device, stagingBuffer = m_StagingBuffer, stagingBufferMemory = m_StagingBufferMemory,
          buffer = m_Buffer, bufferMemory = m_BufferMemory, type = m_Type]()
        {
            if (type == VertexBufferType::Dynamic)
            {
                // Unmapping
                vkUnmapMemory(device, stagingBufferMemory);

                // Destroy staging buffer
                vkDestroyBuffer(device, stagingBuffer, nullptr);
                vkFreeMemory(device, stagingBufferMemory, nullptr);
            }

            // Destroy vertex buffer
            vkDestroyBuffer(device, buffer, nullptr);
            vkFreeMemory(device, bufferMemory, nullptr);
        });
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
    }

    void VulkanVertexBuffer::SetData(Ref<RenderCommandBuffer> commandBuffer, const void* vertices, u64 size)
    {
        TBO_ENGINE_ASSERT(m_Type == VertexBufferType::Dynamic, "Only dynamic vertex buffers are allowed to call VertexBuffer::SetData");

        if (size == 0)
            return;

        // Copy data to staging buffer
        memcpy(m_StagingBufferPtr, vertices, size);

        // Transfer data to vertex buffer
        Renderer::Submit([commandBuffer, stagingBuffer = m_StagingBuffer, buffer = m_Buffer, size]()
        {
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0; // Optional
            copyRegion.dstOffset = 0; // Optional
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle(), stagingBuffer, buffer, 1, &copyRegion);
        });

    }
}
