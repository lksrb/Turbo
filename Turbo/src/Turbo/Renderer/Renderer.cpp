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
        const Ref<SwapChain>& swap_chain = Engine::Get().GetViewportWindow()->GetSwapchain();
        const u32 current_frame = swap_chain->GetCurrentFrame();

        return current_frame;
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

    void Renderer::BeginRenderPass(Ref<RenderCommandBuffer> commandbuffer, Ref<FrameBuffer> framebuffer, const glm::vec4& clear_color)
    {
        const FrameBuffer::Config& framebuffer_config = framebuffer->GetConfig();

        Renderer::SetViewport(commandbuffer, 0, 0, framebuffer_config.Width, framebuffer_config.Height);
        Renderer::SetScissor(commandbuffer, 0, 0, framebuffer_config.Width, framebuffer_config.Height);

        VkCommandBuffer vk_commandbuffer = commandbuffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer();
        VkFramebuffer vk_framebuffer = framebuffer.As<VulkanFrameBuffer>()->GetFrameBuffer();
        VkRenderPass vk_renderpass = framebuffer_config.Renderpass.As<VulkanRenderPass>()->GetRenderPass();

        VkClearValue clearValues[2]{};
        clearValues[0].color = { { clear_color.x, clear_color.y, clear_color.z, clear_color.w } };
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

        vkCmdBeginRenderPass(vk_commandbuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void Renderer::EndRenderPass(Ref<RenderCommandBuffer> commandbuffer)
    {
        VkCommandBuffer vk_commandbuffer = commandbuffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer();

        vkCmdEndRenderPass(vk_commandbuffer);
    }

    void Renderer::DrawIndexed(Ref<RenderCommandBuffer> commandbuffer, Ref<VertexBuffer> vertexbuffer, Ref<IndexBuffer> indexbuffer, Ref<GraphicsPipeline> pipeline, Ref<Shader> shader, u32 index_count)
    {
        VkCommandBuffer vk_commandbuffer = commandbuffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer();

        VkBuffer vk_vertex_buffer = vertexbuffer.As<VulkanVertexBuffer>()->GetBuffer();
        VkBuffer vk_index_buffer = indexbuffer.As<VulkanIndexBuffer>()->GetBuffer();
        VkPipeline vk_pipeline = pipeline.As<VulkanGraphicsPipeline>()->GetPipeline();
        VkPipelineLayout vk_pipeline_layout = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineLayout();
        VkDescriptorSet vk_descriptor_set = shader.As<VulkanShader>()->GetDescriptorSet();

        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(vk_commandbuffer, 0, 1, &vk_vertex_buffer, offsets);
        vkCmdBindIndexBuffer(vk_commandbuffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindPipeline(vk_commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

        if (vk_descriptor_set) // TODO: Make this more convenient
            vkCmdBindDescriptorSets(vk_commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &vk_descriptor_set, 0, nullptr);

        vkCmdDrawIndexed(vk_commandbuffer, index_count, 1, 0, 0, 0);
    }

}
