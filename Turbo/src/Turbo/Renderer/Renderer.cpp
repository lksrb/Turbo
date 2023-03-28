#include "tbopch.h"
#include "Renderer.h"

#include "Renderer2D.h"
#include "SwapChain.h"

#include "Turbo/Core/Engine.h"

#include "Turbo/Platform/Vulkan/VulkanSwapChain.h"
#include "Turbo/Platform/Vulkan/VulkanBuffer.h"
#include "Turbo/Platform/Vulkan/VulkanRenderPass.h"
#include "Turbo/Platform/Vulkan/VulkanRenderCommandBuffer.h"
#include "Turbo/Platform/Vulkan/VulkanImage2D.h"
#include "Turbo/Platform/Vulkan/VulkanIndexBuffer.h"
#include "Turbo/Platform/Vulkan/VulkanVertexBuffer.h"
#include "Turbo/Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "Turbo/Platform/Vulkan/VulkanShader.h"

namespace Turbo
{
    static void GetOrCreateWriteDescriptors(const Ref<UniformBufferSet>& uniformBufferSet, const std::vector<VulkanShader::UniformBufferInfo>& uniformBuffers)
    {
        //static std::unordered_map<std::string, VkWriteDescriptorSet> s_CachedWriteDescriptorSets;

      /*  for (auto& uniformBuffer : uniformBuffers)
        {
            auto& it = s_CachedWriteDescriptorSets.find(uniformBuffer.Name);

            if (it != s_CachedWriteDescriptorSets.end())
            {

            }
            // Create new vulkan buffer for resource
            RendererBuffer::Config bufferConfig = {};
            bufferConfig.Size = uniformBuffer.Size;
            bufferConfig.UsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            bufferConfig.MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            s_CachedWriteDescriptorSets[uniformBuffer.Name] = Ref<VulkanBuffer>::Create(bufferConfig);

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_UniformBufferMap[uniformBuffer.Name]->GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = uniformBuffer.Size;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = shader->GetDescriptorSet();
            descriptorWrite.dstBinding = uniformBuffer.Binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }
*/

    }

    struct RendererInternal
    {
        RenderCommandQueue RenderQueue;
    };

    static RendererInternal* s_Internal;

    void Renderer::Initialize()
    {
        s_Internal = new RendererInternal;
    }

    void Renderer::Shutdown()
    {
        delete s_Internal;
    }

    RenderCommandQueue& Renderer::GetRenderCommandQueue()
    {
        return s_Internal->RenderQueue;
    }

    u32 Renderer::GetCurrentFrame()
    {
        const Ref<SwapChain>& swapChain = Engine::Get().GetViewportWindow()->GetSwapchain();
        const u32 currentFrame = swapChain->GetCurrentFrame();

        return currentFrame;
    }

    void Renderer::BeginFrame()
    {

    }

    void Renderer::Render()
    {
        s_Internal->RenderQueue.Execute();
    }

    // Command buffer functions
    // Command buffer functions
    // Command buffer functions

    void Renderer::SetViewport(Ref<RenderCommandBuffer> commandbuffer, i32 x, i32 y, u32 width, u32 he, f32 min_depth, f32 max_depth)
    {
        VkCommandBuffer vk_commandbuffer = commandbuffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer();

        VkViewport viewport = {};
        viewport.x = static_cast<f32>(x);
        viewport.y = static_cast<f32>(y);
        viewport.width = static_cast<f32>(width);
        viewport.height = static_cast<f32>(he);
        viewport.minDepth = min_depth;
        viewport.maxDepth = max_depth;
        vkCmdSetViewport(vk_commandbuffer, 0, 1, &viewport);
    }

    void Renderer::SetScissor(Ref<RenderCommandBuffer> commandbuffer, i32 x, i32 y, u32 width, u32 height)
    {
        VkCommandBuffer vk_commandbuffer = commandbuffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer();

        VkRect2D scissor = {};
        scissor.offset = { x,y };
        scissor.extent = { width, height };
        vkCmdSetScissor(vk_commandbuffer, 0, 1, &scissor);
    }

    void Renderer::BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<FrameBuffer> frameBuffer, const glm::vec4& clearColor)
    {
        const FrameBuffer::Config& framebuffer_config = frameBuffer->GetConfig();

        Renderer::SetViewport(commandBuffer, 0, 0, framebuffer_config.Width, framebuffer_config.Height);
        Renderer::SetScissor(commandBuffer, 0, 0, framebuffer_config.Width, framebuffer_config.Height);

        VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer();
        VkFramebuffer vk_framebuffer = frameBuffer.As<VulkanFrameBuffer>()->GetFrameBuffer();
        VkRenderPass vk_renderpass = framebuffer_config.Renderpass.As<VulkanRenderPass>()->GetRenderPass();

        VkClearValue clearValues[2]{};
        clearValues[0].color = { { clearColor.x, clearColor.y, clearColor.z, clearColor.w } };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = vk_renderpass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = { framebuffer_config.Width, framebuffer_config.Height };
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;
        renderPassBeginInfo.framebuffer = vk_framebuffer;

        vkCmdBeginRenderPass(vkCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void Renderer::EndRenderPass(Ref<RenderCommandBuffer> commandBuffer)
    {
        VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer();

        vkCmdEndRenderPass(vkCommandBuffer);
    }

    void Renderer::DrawIndexed(Ref<RenderCommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, Ref<Shader> shader, u32 indexCount)
    {
        VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer();

        VkBuffer vkVertexBuffer = vertexBuffer.As<VulkanVertexBuffer>()->GetBuffer();
        VkBuffer vkIndexBuffer = indexBuffer.As<VulkanIndexBuffer>()->GetBuffer();
        VkPipeline vkPipeline = pipeline.As<VulkanGraphicsPipeline>()->GetPipeline();
        VkPipelineLayout vkPipelineLayout = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineLayout();

        Ref<VulkanShader> vkShader = shader.As<VulkanShader>();
        VkDescriptorSet vkDescriptorSet = vkShader->GetDescriptorSet();

        // Updating or creating descriptor sets
        {
            const auto& resources = vkShader->GetResources();
            GetOrCreateWriteDescriptors(uniformBufferSet, resources.UniformBuffers);
        }

        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, &vkVertexBuffer, offsets);
        vkCmdBindIndexBuffer(vkCommandBuffer, vkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

        vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet, 0, nullptr);

        vkCmdDrawIndexed(vkCommandBuffer, indexCount, 1, 0, 0, 0);
    }

}
