#include "tbopch.h"
#include "VulkanRenderCommandBuffer.h"

#include "Turbo/Renderer/Renderer.h"
#include "Turbo/Renderer/RendererContext.h"

namespace Turbo
{
    VulkanRenderCommandBuffer::VulkanRenderCommandBuffer()
    {
        VkDevice device = RendererContext::GetDevice();

        // Command buffers
        {
            VkCommandBufferAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.commandPool = RendererContext::GetCommandPool();
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc_info.commandBufferCount = m_CommandBuffers.Size();

            TBO_VK_ASSERT(vkAllocateCommandBuffers(device, &alloc_info, m_CommandBuffers.Data()));
        }

        // Wait fences
        {
            VkFenceCreateInfo fence_create_info = {};
            fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            for (auto& fence : m_WaitFences) 
            {
                TBO_VK_ASSERT(vkCreateFence(device, &fence_create_info, nullptr, &fence));
            }
        }

        RendererContext::SubmitResourceFree([waitFences = m_WaitFences]()
        {
            VkDevice device = RendererContext::GetDevice();
            for (auto& fence : waitFences)
            {
                vkDestroyFence(device, fence, nullptr);
            }
        });
    }

    VulkanRenderCommandBuffer::~VulkanRenderCommandBuffer()
    {
    }

    VkCommandBuffer VulkanRenderCommandBuffer::GetHandle() const
    {
        u32 currentFrame = Renderer::GetCurrentFrame();
        return m_CommandBuffers[currentFrame];
    }

    void VulkanRenderCommandBuffer::Begin()
    {
        Renderer::Submit([this]()
        {
            VkDevice device = RendererContext::GetDevice();
            u32 currentFrame = Renderer::GetCurrentFrame();

            TBO_VK_ASSERT(vkWaitForFences(device, 1, &m_WaitFences[currentFrame], VK_TRUE, UINT64_MAX));
            TBO_VK_ASSERT(vkResetFences(device, 1, &m_WaitFences[currentFrame]));

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            beginInfo.pInheritanceInfo = nullptr;
            TBO_VK_ASSERT(vkBeginCommandBuffer(m_CommandBuffers[currentFrame], &beginInfo));
        });
    }

    void VulkanRenderCommandBuffer::End()
    {
        Renderer::Submit([this]()
        {
            u32 currentFrame = Renderer::GetCurrentFrame();
            TBO_VK_ASSERT(vkEndCommandBuffer(m_CommandBuffers[currentFrame]));
        });
    }

    void VulkanRenderCommandBuffer::Submit()
    {
        Renderer::Submit([this]()
        {
            VkDevice device = RendererContext::GetDevice();
            u32 currentFrame = Renderer::GetCurrentFrame();
            VkQueue graphicsQueue = RendererContext::GetGraphicsQueue();

            VkSubmitInfo submit_info = {};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.pCommandBuffers = &m_CommandBuffers[currentFrame];
            submit_info.commandBufferCount = 1;
            TBO_VK_ASSERT(vkQueueSubmit(graphicsQueue, 1, &submit_info, m_WaitFences[currentFrame]));
        });
    }

}
