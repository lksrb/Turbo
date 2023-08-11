#include "tbopch.h"
#include "VulkanIndexBuffer.h"

#include "VulkanUtils.h"

#include "Turbo/Renderer/RendererContext.h"

namespace Turbo
{
    VulkanIndexBuffer::VulkanIndexBuffer(const u32* indices, u32 count)
    {
        m_Size = count * sizeof(u32);

        VkDevice device = RendererContext::GetDevice();

        // Staging buffer
        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
        {
            VkBufferCreateInfo stagingCreateInfo = {};
            stagingCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingCreateInfo.pNext = nullptr;
            stagingCreateInfo.size = m_Size;
            stagingCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            stagingCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            TBO_VK_ASSERT(vkCreateBuffer(device, &stagingCreateInfo, nullptr, &stagingBuffer));

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

            VkMemoryAllocateInfo allocateInfo = {};
            allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocateInfo.pNext = nullptr;
            allocateInfo.allocationSize = memRequirements.size;
            allocateInfo.memoryTypeIndex = Vulkan::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            TBO_VK_ASSERT(vkAllocateMemory(device, &allocateInfo, nullptr, &stagingBufferMemory));
            TBO_VK_ASSERT(vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0));
        }

        // Index buffer
        {
            VkBufferCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.size = m_Size;
            createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            TBO_VK_ASSERT(vkCreateBuffer(device, &createInfo, nullptr, &m_Buffer));

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

        // Set data into the staging buffer
        {
            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, m_Size, 0, &data);
            memcpy(data, indices, m_Size);
            vkUnmapMemory(device, stagingBufferMemory);
        }

        // Transfer indices immediately to GPU
        RendererContext::ImmediateSubmit([this, stagingBuffer](VkCommandBuffer commandBuffer)
        {
            VkDevice device = RendererContext::GetDevice();

            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0; // Optional
            copyRegion.dstOffset = 0; // Optional
            copyRegion.size = m_Size;
            vkCmdCopyBuffer(commandBuffer, stagingBuffer, m_Buffer, 1, &copyRegion);
        });

        // Destroy staging buffer
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

        // Add it to the resource free queue
        RendererContext::SubmitResourceFree([device, buffer = m_Buffer, bufferMemory = m_BufferMemory]()
        {
            // Destroy index buffer
            vkDestroyBuffer(device, buffer, nullptr);
            vkFreeMemory(device, bufferMemory, nullptr);
        });
    }
}
