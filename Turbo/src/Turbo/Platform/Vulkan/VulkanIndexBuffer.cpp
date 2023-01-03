#include "tbopch.h"
#include "VulkanIndexBuffer.h"

#include "VulkanUtils.h"

#include "Turbo/Renderer/RendererContext.h"

namespace Turbo
{
    VulkanIndexBuffer::VulkanIndexBuffer(const IndexBuffer::Config& config)
        : IndexBuffer(config), m_Buffer(VK_NULL_HANDLE), m_Memory(VK_NULL_HANDLE)
    {
        VkDevice device = RendererContext::GetDevice();

        // Staging buffer
        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
        {
            VkBufferCreateInfo stagingCreateInfo = {};
            stagingCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingCreateInfo.pNext = nullptr;
            stagingCreateInfo.size = m_Config.Size;
            stagingCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            stagingCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            TBO_VK_ASSERT(vkCreateBuffer(device, &stagingCreateInfo, nullptr, &stagingBuffer));

            VkMemoryRequirements memRequiremets;
            vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequiremets);

            VkMemoryAllocateInfo allocateInfo = {};
            allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocateInfo.pNext = nullptr;
            allocateInfo.allocationSize = m_Config.Size;
            allocateInfo.memoryTypeIndex = Vulkan::FindMemoryType(memRequiremets.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            TBO_VK_ASSERT(vkAllocateMemory(device, &allocateInfo, nullptr, &stagingBufferMemory));
            TBO_VK_ASSERT(vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0));
        }

        // Index buffer
        {
            VkBufferCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.size = m_Config.Size;
            createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            TBO_VK_ASSERT(vkCreateBuffer(device, &createInfo, nullptr, &m_Buffer));

            VkMemoryRequirements memRequiremets;
            vkGetBufferMemoryRequirements(device, m_Buffer, &memRequiremets);

            VkMemoryAllocateInfo allocateInfo = {};
            allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocateInfo.pNext = nullptr;
            allocateInfo.allocationSize = m_Config.Size;
            allocateInfo.memoryTypeIndex = Vulkan::FindMemoryType(memRequiremets.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            TBO_VK_ASSERT(vkAllocateMemory(device, &allocateInfo, nullptr, &m_Memory));
            TBO_VK_ASSERT(vkBindBufferMemory(device, m_Buffer, m_Memory, 0));
        }

        // Set data into the staging buffer
        {
            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, m_Config.Size, 0, &data);
            memcpy(data, m_Config.Indices, m_Config.Size);
            vkUnmapMemory(device, stagingBufferMemory);
        }

        // Transfer indices immediately to GPU
        RendererContext::ImmediateSubmit([this, stagingBuffer](VkCommandBuffer buffer)
        {
            VkDevice device = RendererContext::GetDevice();

            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0; // Optional
            copyRegion.dstOffset = 0; // Optional
            copyRegion.size = m_Config.Size;
            vkCmdCopyBuffer(buffer, stagingBuffer, m_Buffer, 1, &copyRegion);
        });

        // Destroy staging buffer
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

        // Add it to the resource free queue
        auto& resourceFreeQueue = RendererContext::GetResourceQueue();

        resourceFreeQueue.Submit(BUFFER, [device, m_Buffer = m_Buffer, m_Memory = m_Memory]()
        {
            // Destroy index buffer
            vkDestroyBuffer(device, m_Buffer, nullptr);
            vkFreeMemory(device, m_Memory, nullptr);
        });
    }

    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
    }
}
