#include "tbopch.h"
#include "VulkanUniformBuffer.h"

#include "Turbo/Platform/Vulkan/VulkanUtils.h"

namespace Turbo
{
    VulkanUniformBuffer::VulkanUniformBuffer(const UniformBuffer::Config& config)
        : UniformBuffer(config)
    {
    }

    VulkanUniformBuffer::~VulkanUniformBuffer()
    {
    }

    void VulkanUniformBuffer::Invalidate()
    {
        VkDevice device = RendererContext::GetDevice();

        VkBufferCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.size = m_Config.Size;
        createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        TBO_VK_ASSERT(vkCreateBuffer(device, &createInfo, nullptr, &m_Buffer));

        // Query requirements
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, m_Buffer, &memRequirements);

        VkMemoryAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.allocationSize = memRequirements.size;
        allocateInfo.memoryTypeIndex = Vulkan::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // Allocate memory
        TBO_VK_ASSERT(vkAllocateMemory(device, &allocateInfo, nullptr, &m_Memory));

        // Bind buffer with memory
        TBO_VK_ASSERT(vkBindBufferMemory(device, m_Buffer, m_Memory, 0));

        // Access allocated buffer
        TBO_VK_ASSERT(vkMapMemory(device, m_Memory, 0, m_Config.Size, 0, &m_Data));

        // Add it to deletion queue         
        RendererContext::SubmitResourceFree([device, m_Memory = m_Memory, m_Buffer = m_Buffer]()
        {
            vkUnmapMemory(device, m_Memory);
            vkDestroyBuffer(device, m_Buffer, nullptr);
            vkFreeMemory(device, m_Memory, nullptr);
        });
    }

    void VulkanUniformBuffer::SetData(const void* data)
    {
        memcpy(m_Data, data, m_Config.Size);
    }

}
