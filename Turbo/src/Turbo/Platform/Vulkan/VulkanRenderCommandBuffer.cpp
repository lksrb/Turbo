#include "tbopch.h"
#include "VulkanRenderCommandBuffer.h"

#include "VulkanContext.h"

#include "Turbo/Renderer/Renderer.h"

namespace Turbo
{
    VulkanRenderCommandBuffer::VulkanRenderCommandBuffer()
    {
        VulkanDevice& device = VulkanContext::Get()->GetDevice();

        // Command buffers
        {
            VkCommandBufferAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.commandPool = device.GetCommandPool();
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

        Renderer::SubmitResourceFree([device = device.GetHandle(), waitFences = m_WaitFences]()
        {
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
        u32 currentFrame = Renderer::GetCurrentFrame();
        VkCommandBuffer commandBuffer = m_CommandBuffers[currentFrame];
        VkFence waitFence = m_WaitFences[currentFrame];

        Renderer::Submit([commandBuffer, waitFence]()
        {
            VkDevice device = VulkanContext::Get()->GetDevice();

            TBO_VK_ASSERT(vkWaitForFences(device, 1, &waitFence, VK_TRUE, UINT64_MAX));
            TBO_VK_ASSERT(vkResetFences(device, 1, &waitFence));

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            beginInfo.pInheritanceInfo = nullptr;
            TBO_VK_ASSERT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
        });
    }

    void VulkanRenderCommandBuffer::End()
    {
        u32 currentFrame = Renderer::GetCurrentFrame();
        VkCommandBuffer commandBuffer = m_CommandBuffers[currentFrame];
        Renderer::Submit([commandBuffer]()
        {
            TBO_VK_ASSERT(vkEndCommandBuffer(commandBuffer));
        });
    }

    void VulkanRenderCommandBuffer::Submit()
    {
        u32 currentFrame = Renderer::GetCurrentFrame();
        VkCommandBuffer commandBuffer = m_CommandBuffers[currentFrame];
        VkFence waitFence = m_WaitFences[currentFrame];
        Renderer::Submit([commandBuffer, waitFence]()
        {
            VulkanDevice& device = VulkanContext::Get()->GetDevice();

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.pCommandBuffers = &commandBuffer;
            submitInfo.commandBufferCount = 1;
            TBO_VK_ASSERT(vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, waitFence));
        });
    }

}
