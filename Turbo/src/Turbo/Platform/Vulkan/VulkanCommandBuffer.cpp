#include "tbopch.h"
#include "VulkanCommandBuffer.h"

#include "Turbo/Renderer/RendererContext.h"

namespace Turbo
{
    VulkanCommandBuffer::VulkanCommandBuffer(CommandBufferLevel type)
        : CommandBuffer(type), m_CommandBuffer(VK_NULL_HANDLE)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = RendererContext::GetCommandPool();
        allocInfo.level = static_cast<VkCommandBufferLevel>(type);
        allocInfo.commandBufferCount = 1;

        TBO_VK_ASSERT(vkAllocateCommandBuffers(RendererContext::GetDevice(), &allocInfo, &m_CommandBuffer));
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
    }

    void VulkanCommandBuffer::Begin()
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = (m_Type == CommandBufferLevel::Primary) ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        beginInfo.pInheritanceInfo = nullptr;
        TBO_VK_ASSERT(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));
    }

    void VulkanCommandBuffer::End()
    {
        TBO_VK_ASSERT(vkEndCommandBuffer(m_CommandBuffer));
    }

}
