#include "tbopch.h"
#include "VulkanBuffer.h"

#include "Turbo/Platform/Vulkan/VulkanUtils.h"

#include "Turbo/Renderer/RendererContext.h"

#include <vulkan/vulkan.h>

namespace Turbo
{ 
    VulkanBuffer::VulkanBuffer(const RendererBuffer::Config& config)
        : RendererBuffer(config)
    {
        VkDevice device = RendererContext::GetDevice();

        VkBufferCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.size = m_Config.Size;
        createInfo.usage = m_Config.UsageFlags;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        TBO_VK_ASSERT(vkCreateBuffer(device, &createInfo, nullptr, &m_Buffer));

        // Query requirements
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, m_Buffer, &memRequirements);

        VkMemoryAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.allocationSize = memRequirements.size;
        allocateInfo.memoryTypeIndex = Vulkan::FindMemoryType(memRequirements.memoryTypeBits, m_Config.MemoryFlags);

        // Allocate memory
        TBO_VK_ASSERT(vkAllocateMemory(device, &allocateInfo, nullptr, &m_Memory));
        
        // Bind buffer with memory
        TBO_VK_ASSERT(vkBindBufferMemory(device, m_Buffer, m_Memory, 0));

        // Access allocated buffer
        TBO_VK_ASSERT(vkMapMemory(device, m_Memory, 0, m_Config.Size, 0, &m_Data));

        if (m_Config.SetDefaultValue)
        {
            memset(m_Data, m_Config.DefaultValue, m_Config.Size);
        }

        if (m_Config.Temporary == false)
        {
            // Add it to deletion queue 
            RendererContext::SubmitResourceFree([device, m_Memory = m_Memory, m_Buffer = m_Buffer]()
            {
                vkUnmapMemory(device, m_Memory);
                vkDestroyBuffer(device, m_Buffer, nullptr);
                vkFreeMemory(device, m_Memory, nullptr);
            });
        }
    }

    VulkanBuffer::~VulkanBuffer()
    {
        if (m_Config.Temporary)
        {
            RendererContext::SubmitRuntimeResourceFree([m_Memory = m_Memory, m_Buffer = m_Buffer]()
            {
                VkDevice device = RendererContext::GetDevice();
                vkUnmapMemory(device, m_Memory);
                vkDestroyBuffer(device, m_Buffer, nullptr);
                vkFreeMemory(device, m_Memory, nullptr);
            });
        }

        m_Data = nullptr;
    }

    void VulkanBuffer::SetData(const void* data)
    {
        memcpy(m_Data, data, m_Config.Size);
    }
}
