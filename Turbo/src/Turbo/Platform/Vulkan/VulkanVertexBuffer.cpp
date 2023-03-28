#include "tbopch.h"
#include "VulkanVertexBuffer.h"

#include "VulkanUtils.h"
#include "VulkanRenderCommandBuffer.h"

namespace Turbo
{
    VulkanVertexBuffer::VulkanVertexBuffer(const VertexBuffer::Config& config)
        : VertexBuffer(config)
    {
        VkDevice device = RendererContext::GetDevice();

        // Staging buffer
        {
            VkBufferCreateInfo bufferCreateInfo = {};
            bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.pNext = nullptr;
            bufferCreateInfo.size = m_Config.Size;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            TBO_VK_ASSERT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &m_StagingBuffer));

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, m_StagingBuffer, &memRequirements);

            VkMemoryAllocateInfo allocateInfo = {};
            allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocateInfo.pNext = nullptr;
            allocateInfo.allocationSize = m_Config.Size;
            allocateInfo.memoryTypeIndex = Vulkan::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            TBO_VK_ASSERT(vkAllocateMemory(device, &allocateInfo, nullptr, &m_StagingBufferMemory));
            TBO_VK_ASSERT(vkBindBufferMemory(device, m_StagingBuffer, m_StagingBufferMemory, 0));
            TBO_VK_ASSERT(vkMapMemory(device, m_StagingBufferMemory, 0, m_Config.Size, 0, &m_StagingBufferPtr));
        }

        // Vertex buffer
        {
            VkBufferCreateInfo bufferCreateInfo = {};
            bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.pNext = nullptr;
            bufferCreateInfo.size = m_Config.Size;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            TBO_VK_ASSERT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &m_Buffer));

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, m_Buffer, &memRequirements);

            VkMemoryAllocateInfo allocateInfo = {};
            allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocateInfo.pNext = nullptr;
            allocateInfo.allocationSize = m_Config.Size;
            allocateInfo.memoryTypeIndex = Vulkan::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            TBO_VK_ASSERT(vkAllocateMemory(device, &allocateInfo, nullptr, &m_Memory));
            TBO_VK_ASSERT(vkBindBufferMemory(device, m_Buffer, m_Memory, 0));
        }

        // Add it to the resource free queue
        auto& resourceFreeQueue = RendererContext::GetResourceQueue();

        resourceFreeQueue.Submit(BUFFER, [device,
            m_StagingBuffer = m_StagingBuffer,
            m_StagingBufferMemory = m_StagingBufferMemory,
            m_Buffer = m_Buffer,
            m_Memory = m_Memory]()
        {
            // Unmapping
            vkUnmapMemory(device, m_StagingBufferMemory);

            // Destroy staging buffer
            vkDestroyBuffer(device, m_StagingBuffer, nullptr);
            vkFreeMemory(device, m_StagingBufferMemory, nullptr);

            // Destroy vertex buffer
            vkDestroyBuffer(device, m_Buffer, nullptr);
            vkFreeMemory(device, m_Memory, nullptr);
        });

        m_TranferCommandBuffer = RenderCommandBuffer::Create();
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
    }

    void VulkanVertexBuffer::SetData(void* data, u32 size)
    {
        // Copy data to staging buffer
        memcpy(m_StagingBufferPtr, data, size);

        // Transfer data to vertex buffer
        TransferData(size);
    }

    void VulkanVertexBuffer::TransferData(u32 size)
    {
        m_TranferCommandBuffer->Begin();

        VkDevice device = RendererContext::GetDevice();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = static_cast<VkDeviceSize>(size);
        vkCmdCopyBuffer(m_TranferCommandBuffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(), m_StagingBuffer, m_Buffer, 1, &copyRegion);

        m_TranferCommandBuffer->End();
        m_TranferCommandBuffer->Submit();
    }

}
