#include "tbopch.h"
#include "Turbo/Renderer/Renderer.h"

#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanRenderCommandBuffer.h"
#include "VulkanImage2D.h"
#include "VulkanIndexBuffer.h"
#include "VulkanVertexBuffer.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanShader.h"
#include "VulkanUniformBuffer.h"

#include "Turbo/Core/Engine.h"

namespace Turbo
{
    struct WriteDescriptorSetInfo
    {
        VkWriteDescriptorSet WriteDescriptorSet;
        VkDescriptorBufferInfo BufferInfo;
        bool IsDirty = true;
    };

    static std::map<VulkanShader const*, std::map<UniformBuffer const*, WriteDescriptorSetInfo>> s_CachedWriteDescriptorSets;

    static void UpdateWriteDescriptors(const Ref<UniformBufferSet>& uniformBufferSet, const Ref<VulkanShader>& shader, const std::vector<VulkanShader::UniformBufferInfo>& uniformBufferInfos)
    {
        VkDevice device = RendererContext::GetDevice();

        for (const auto& ubInfo : uniformBufferInfos)
        {
            auto uniformBuffer = uniformBufferSet->Get(0, ubInfo.Binding); // Set is 0 for now

            // Retrieve or create shader map
            auto& shaderMap = s_CachedWriteDescriptorSets[shader.Get()];

            auto& it = shaderMap.find(uniformBuffer.Get());
            if (it == shaderMap.end())
            {
                auto& info = shaderMap[uniformBuffer.Get()] = {};

                // Buffer
                info.BufferInfo.buffer = uniformBuffer.As<VulkanUniformBuffer>()->GetHandle();
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

            auto& cached = shaderMap.at(uniformBuffer.Get());

            if (cached.IsDirty) // Ensure that the descriptor is updated only once a frame
            {
                cached.IsDirty = false;
                vkUpdateDescriptorSets(device, 1, &cached.WriteDescriptorSet, 0, nullptr);
            }
        }
    }

    struct RendererInternal
    {
        CommandQueue RenderQueue;
    };

    static RendererInternal* s_Internal;

    void Renderer::Init()
    {
        s_Internal = new RendererInternal;
    }

    void Renderer::Shutdown()
    {
        delete s_Internal;
    }

    CommandQueue& Renderer::GetCommandQueue()
    {
        return s_Internal->RenderQueue;
    }

    u32 Renderer::GetCurrentFrame()
    {
        auto swapChain = Engine::Get().GetViewportWindow()->GetSwapchain();
        u32 currentFrame = swapChain->GetCurrentFrame();

        return currentFrame;
    }

    void Renderer::BeginFrame()
    {
        // On new frame, invalidate descriptors
        for (auto& [_, writeDescriptorMap] : s_CachedWriteDescriptorSets)
        {
            for (auto& [_, wds] : writeDescriptorMap)
            {
                wds.IsDirty = true;
            }
        }
    }

    void Renderer::Render()
    {
        s_Internal->RenderQueue.Execute();
    }

    // Command buffer functions
    // Command buffer functions
    // Command buffer functions

    void Renderer::SetLineWidth(Ref<RenderCommandBuffer> commandBuffer, f32 lineWidth)
    {
        Renderer::Submit([=]()
        {
            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();
            vkCmdSetLineWidth(vkCommandBuffer, lineWidth);
        });
    }

    void Renderer::SetViewport(Ref<RenderCommandBuffer> commandBuffer, i32 x, i32 y, u32 width, u32 he, f32 min_depth, f32 max_depth)
    {
        VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();

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
        VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();

        VkRect2D scissor = {};
        scissor.offset = { x,y };
        scissor.extent = { width, height };
        vkCmdSetScissor(vkCommandBuffer, 0, 1, &scissor);
    }

    void Renderer::BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass, const glm::vec4& clearColor)
    {
        Renderer::Submit([=]()
        {
            const auto& framebufferConfig = renderPass->GetConfig().TargetFrameBuffer->GetConfig();

            Renderer::SetViewport(commandBuffer, 0, 0, framebufferConfig.Width, framebufferConfig.Height);
            Renderer::SetScissor(commandBuffer, 0, 0, framebufferConfig.Width, framebufferConfig.Height);

            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();
            VkFramebuffer vkFramebuffer = renderPass->GetConfig().TargetFrameBuffer.As<VulkanFrameBuffer>()->GetHandle();
            VkRenderPass vkRenderpass = renderPass.As<VulkanRenderPass>()->GetHandle();

            VkClearValue clearValues[2]{};
            clearValues[0].color = { { clearColor.x, clearColor.y, clearColor.z, clearColor.w } };
            clearValues[1].depthStencil = { 1.0f, 0 };

            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass = vkRenderpass;
            renderPassBeginInfo.renderArea.offset.x = 0;
            renderPassBeginInfo.renderArea.offset.y = 0;
            renderPassBeginInfo.renderArea.extent = { framebufferConfig.Width, framebufferConfig.Height };
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearValues;
            renderPassBeginInfo.framebuffer = vkFramebuffer;

            vkCmdBeginRenderPass(vkCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        });
    }

    void Renderer::EndRenderPass(Ref<RenderCommandBuffer> commandBuffer)
    {
        Renderer::Submit([=]()
        {
            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();
            vkCmdEndRenderPass(vkCommandBuffer);
        });
    }

    void Renderer::Draw(Ref<RenderCommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, Ref<Shader> shader, u32 vertexCount)
    {
        Renderer::Submit([=]()
        {
            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();

            VkBuffer vkVertexBuffer = vertexBuffer.As<VulkanVertexBuffer>()->GetHandle();
            VkPipeline vkPipeline = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineHandle();
            VkPipelineLayout vkPipelineLayout = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineLayoutHandle();

            Ref<VulkanShader> vkShader = shader.As<VulkanShader>();
            VkDescriptorSet vkDescriptorSet = vkShader->GetDescriptorSet();

            // Updating or creating descriptor sets
            const auto& resources = vkShader->GetResources();
            UpdateWriteDescriptors(uniformBufferSet, vkShader, resources.UniformBuffers); // TODO: We need separate write descriptors for each shader

            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, &vkVertexBuffer, offsets);
            vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
            vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet, 0, nullptr);

            vkCmdDraw(vkCommandBuffer, vertexCount, 1, 0, 0);
        });
    }

    void Renderer::DrawIndexed(Ref<RenderCommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, Ref<Shader> shader, u32 indexCount)
    {
        Renderer::Submit([=]()
        {
            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();

            VkBuffer vkVertexBuffer = vertexBuffer.As<VulkanVertexBuffer>()->GetHandle();
            VkBuffer vkIndexBuffer = indexBuffer.As<VulkanIndexBuffer>()->GetHandle();
            VkPipeline vkPipeline = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineHandle();
            VkPipelineLayout vkPipelineLayout = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineLayoutHandle();

            Ref<VulkanShader> vkShader = shader.As<VulkanShader>();
            VkDescriptorSet vkDescriptorSet = vkShader->GetDescriptorSet();

            // Updating or creating descriptor sets
            const auto& resources = vkShader->GetResources();
            UpdateWriteDescriptors(uniformBufferSet, vkShader, resources.UniformBuffers); // TODO: We need separate write descriptors for each shader

            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, &vkVertexBuffer, offsets);
            vkCmdBindIndexBuffer(vkCommandBuffer, vkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
            vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet, 0, nullptr);

            vkCmdDrawIndexed(vkCommandBuffer, indexCount, 1, 0, 0, 0);
        });
    }

    void Renderer::PushConstant(Ref<RenderCommandBuffer> commandBuffer, Ref<GraphicsPipeline> pipeline, u32 size, const void* data)
    {
        Renderer::Submit([=]()
        {
            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();
            VkPipelineLayout vkPipelineLayout = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineLayoutHandle();

            vkCmdPushConstants(vkCommandBuffer, vkPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, data);
        });
    }

    void Renderer::DrawInstanced(Ref<RenderCommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer, Ref<VertexBuffer> instanceBuffer, Ref<IndexBuffer> indexBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, u32 instanceCount, u32 indicesPerInstance)
    {
        Renderer::Submit([=]()
        {
            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();

            VkBuffer vkVertexBuffer = vertexBuffer.As<VulkanVertexBuffer>()->GetHandle();
            VkBuffer vkInstanceBuffer = instanceBuffer.As<VulkanVertexBuffer>()->GetHandle();
            VkBuffer vkIndexBuffer = indexBuffer.As<VulkanIndexBuffer>()->GetHandle();
            VkPipeline vkPipeline = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineHandle();
            VkPipelineLayout vkPipelineLayout = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineLayoutHandle();
            Ref<VulkanShader> vkShader = pipeline->GetConfig().Shader.As<VulkanShader>();
            VkDescriptorSet vkDescriptorSet = vkShader->GetDescriptorSet();

            // Updating or creating descriptor sets
            const auto& resources = vkShader->GetResources();
            UpdateWriteDescriptors(uniformBufferSet, vkShader, resources.UniformBuffers); // TODO: We need separate write descriptors for each shader

            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, &vkVertexBuffer, offsets);
            vkCmdBindVertexBuffers(vkCommandBuffer, 1, 1, &vkInstanceBuffer, offsets);
            vkCmdBindIndexBuffer(vkCommandBuffer, vkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
            vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet, 0, nullptr);

            vkCmdDrawIndexed(vkCommandBuffer, indicesPerInstance, instanceCount, 0, 0, 0);
        });
    }

    void Renderer::DrawStaticMesh(Ref<RenderCommandBuffer> commandBuffer, Ref<StaticMesh> mesh, Ref<VertexBuffer> transformBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, u32 transformOffset, u32 subMeshIndex, u32 instanceCount)
    {
        Renderer::Submit([=]()
        {
            Ref<VertexBuffer> vertexBuffer = mesh->GetVertexBuffer();
            Ref<VertexBuffer> indexBuffer = mesh->GetIndexBuffer();
            Ref<VulkanShader> shader = pipeline->GetConfig().Shader.As<VulkanShader>();

            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();

            VkBuffer vkVertexBuffer = vertexBuffer.As<VulkanVertexBuffer>()->GetHandle();
            VkBuffer vkTransformBuffer = transformBuffer.As<VulkanVertexBuffer>()->GetHandle();
            VkBuffer vkIndexBuffer = indexBuffer.As<VulkanIndexBuffer>()->GetHandle();
            VkPipeline vkPipeline = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineHandle();
            VkPipelineLayout vkPipelineLayout = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineLayoutHandle();
            VkDescriptorSet vkDescriptorSet = shader->GetDescriptorSet();

            // Updating or creating descriptor sets
            const auto& resources =shader->GetResources();
            UpdateWriteDescriptors(uniformBufferSet, shader, resources.UniformBuffers); // TODO: We need separate write descriptors for each shader

            auto& submesh = mesh->GetSubmeshes()[subMeshIndex];

            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, &vkVertexBuffer, offsets);
            VkDeviceSize instanceOffsets[] = { transformOffset };
            vkCmdBindVertexBuffers(vkCommandBuffer, 1, 1, &vkTransformBuffer, instanceOffsets);
            vkCmdBindIndexBuffer(vkCommandBuffer, vkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
            vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet, 0, nullptr);
            vkCmdDrawIndexed(vkCommandBuffer, submesh.IndexCount, instanceCount, submesh.BaseIndex, submesh.BaseVertex, 0);
        });
    }
}
