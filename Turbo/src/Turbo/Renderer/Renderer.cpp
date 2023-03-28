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
#include "Turbo/Platform/Vulkan/VulkanUniformBuffer.h"

namespace Turbo
{
    struct WriteDescriptorSetInfo
    {
        VkWriteDescriptorSet WriteDescriptorSet;
        VkDescriptorBufferInfo BufferInfo;
    };
    static std::unordered_map<UniformBuffer*, WriteDescriptorSetInfo> s_CachedWriteDescriptorSets;

    static void UpdateWriteDescriptors(const Ref<UniformBufferSet>& uniformBufferSet, const Ref<VulkanShader>& shader, const std::vector<VulkanShader::UniformBufferInfo>& uniformBufferInfos)
    {
        static u32 s_LastFrame = -1;

        if (s_LastFrame == Renderer::GetCurrentFrame()) // Ensure that we update uniform buffer sets once a frame
            return;


        VkDevice device = RendererContext::GetDevice();

        for (const auto& ubInfo : uniformBufferInfos)
        {
            Ref<UniformBuffer> uniformBuffer = uniformBufferSet->Get(0, ubInfo.Binding); // Set is 0 for now

            auto& it = s_CachedWriteDescriptorSets.find(uniformBuffer.Get());
            if (it == s_CachedWriteDescriptorSets.end())
            {
                WriteDescriptorSetInfo& info = s_CachedWriteDescriptorSets[uniformBuffer.Get()];
                info = {};

                // Buffer
                info.BufferInfo.buffer = uniformBuffer.As<VulkanUniformBuffer>()->GetBuffer();
                info.BufferInfo.offset = 0;
                info.BufferInfo.range = ubInfo.Size;

                // WriteDescriptorSet
                info.WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                info.WriteDescriptorSet.dstSet = shader->GetDescriptorSet();
                info.WriteDescriptorSet.dstBinding = ubInfo.Binding;
                info.WriteDescriptorSet.dstArrayElement = 0;
                info.WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                info.WriteDescriptorSet.descriptorCount = 1;
                info.WriteDescriptorSet.pBufferInfo = &info.BufferInfo;
            }

            vkUpdateDescriptorSets(device, 1, &s_CachedWriteDescriptorSets.at(uniformBuffer.Get()).WriteDescriptorSet, 0, nullptr);
        }

        s_LastFrame = Renderer::GetCurrentFrame();
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
        VkCommandBuffer vkCommandBuffer = commandbuffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer();

        VkViewport viewport = {};
        viewport.x = static_cast<f32>(x);
        viewport.y = static_cast<f32>(y);
        viewport.width = static_cast<f32>(width);
        viewport.height = static_cast<f32>(he);
        viewport.minDepth = min_depth;
        viewport.maxDepth = max_depth;
        vkCmdSetViewport(vkCommandBuffer, 0, 1, &viewport);
    }

    void Renderer::SetScissor(Ref<RenderCommandBuffer> commandBuffer, i32 x, i32 y, u32 width, u32 height)
    {
        VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer();

        VkRect2D scissor = {};
        scissor.offset = { x,y };
        scissor.extent = { width, height };
        vkCmdSetScissor(vkCommandBuffer, 0, 1, &scissor);
    }

    void Renderer::BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<FrameBuffer> frameBuffer, const glm::vec4& clearColor)
    {
        const FrameBuffer::Config& framebufferConfig = frameBuffer->GetConfig();

        Renderer::SetViewport(commandBuffer, 0, 0, framebufferConfig.Width, framebufferConfig.Height);
        Renderer::SetScissor(commandBuffer, 0, 0, framebufferConfig.Width, framebufferConfig.Height);

        VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer();
        VkFramebuffer vk_framebuffer = frameBuffer.As<VulkanFrameBuffer>()->GetFrameBuffer();
        VkRenderPass vk_renderpass = framebufferConfig.Renderpass.As<VulkanRenderPass>()->GetRenderPass();

        VkClearValue clearValues[2]{};
        clearValues[0].color = { { clearColor.x, clearColor.y, clearColor.z, clearColor.w } };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = vk_renderpass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = { framebufferConfig.Width, framebufferConfig.Height };
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
            UpdateWriteDescriptors(uniformBufferSet, vkShader, resources.UniformBuffers); // TODO: We need separate write descriptors for each shader
        }

        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, &vkVertexBuffer, offsets);
        vkCmdBindIndexBuffer(vkCommandBuffer, vkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

        vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet, 0, nullptr);

        vkCmdDrawIndexed(vkCommandBuffer, indexCount, 1, 0, 0, 0);
    }

}
