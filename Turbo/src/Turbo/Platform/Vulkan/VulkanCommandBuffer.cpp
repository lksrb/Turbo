#include "tbopch.h"
#include "VulkanCommandBuffer.h"

#include "Turbo/Renderer/Renderer.h"
#include "Turbo/Renderer/RendererContext.h"

namespace Turbo
{
    VulkanCommandBuffer::VulkanCommandBuffer(CommandBufferLevel type)
        : CommandBuffer(type)
    {
        u32 frames_in_flight = RendererContext::FramesInFlight();
        VkDevice device = RendererContext::GetDevice();

        // Command buffers
        {
            m_CommandBuffers.resize(frames_in_flight);
            VkCommandBufferAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.commandPool = RendererContext::GetCommandPool();
            alloc_info.level = static_cast<VkCommandBufferLevel>(type);
            alloc_info.commandBufferCount = frames_in_flight;

            TBO_VK_ASSERT(vkAllocateCommandBuffers(device, &alloc_info, m_CommandBuffers.data()));
        }

        // Wait fences
        {
            VkFenceCreateInfo fence_create_info = {};
            fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            m_WaitFences.resize(frames_in_flight);
            for (auto& fence : m_WaitFences)
                TBO_VK_ASSERT(vkCreateFence(device, &fence_create_info, nullptr, &fence));
        }
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        VkDevice device = RendererContext::GetDevice();
        for (auto& fence : m_WaitFences)
            vkDestroyFence(device, fence, nullptr);
    }

    VkCommandBuffer VulkanCommandBuffer::GetCommandBuffer() const
    {
        u32 current_frame = Renderer::GetCurrentFrame();
        return m_CommandBuffers[current_frame];
    }

    void VulkanCommandBuffer::Begin()
    {
        u32 current_frame = Renderer::GetCurrentFrame();

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = (m_Type == CommandBufferLevel::Primary) ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        beginInfo.pInheritanceInfo = nullptr;
        TBO_VK_ASSERT(vkBeginCommandBuffer(m_CommandBuffers[current_frame], &beginInfo));
    }

    void VulkanCommandBuffer::End()
    {
        u32 current_frame = Renderer::GetCurrentFrame();

        TBO_VK_ASSERT(vkEndCommandBuffer(m_CommandBuffers[current_frame]));
    }

    void VulkanCommandBuffer::Submit()
    {
        // Secondary command buffers cannot be submitted directly into queue
        TBO_ENGINE_ASSERT(m_Type == CommandBufferLevel::Primary, "Cannot submit secondary command buffers directly!");

        VkDevice device = RendererContext::GetDevice();
        u32 current_frame = Renderer::GetCurrentFrame();
        VkQueue graphics_queue = RendererContext::GetGraphicsQueue();

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pCommandBuffers = &m_CommandBuffers[current_frame];
        submit_info.commandBufferCount = 1;

        TBO_VK_ASSERT(vkWaitForFences(device, 1, &m_WaitFences[current_frame], VK_TRUE, UINT64_MAX));
        TBO_VK_ASSERT(vkResetFences(device, 1, &m_WaitFences[current_frame]));
        TBO_VK_ASSERT(vkQueueSubmit(graphics_queue, 1, &submit_info, m_WaitFences[current_frame]));
    }

}
