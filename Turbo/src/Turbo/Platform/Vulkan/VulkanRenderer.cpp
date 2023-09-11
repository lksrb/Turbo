#include "tbopch.h"
#include "Turbo/Renderer/Renderer.h"

#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanTexture2D.h"
#include "VulkanRenderPass.h"
#include "VulkanRenderCommandBuffer.h"
#include "VulkanImage2D.h"
#include "VulkanIndexBuffer.h"
#include "VulkanVertexBuffer.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanShader.h"
#include "VulkanUniformBuffer.h"

#include "Turbo/Core/Application.h"
#include "Turbo/Core/Window.h"

#include "Turbo/Renderer/Fly.h"
#include "Turbo/Renderer/ShaderLibrary.h"
#include "Turbo/Renderer/RendererContext.h"
#include "Turbo/Renderer/Mesh.h"
#include "Turbo/Renderer/MaterialAsset.h"

namespace Turbo {

    struct WriteDescriptorSetInfo
    {
        VkWriteDescriptorSet WriteDescriptorSet;
        VkDescriptorBufferInfo BufferInfo;
        bool IsDirty = true;
    };

    static std::map<VulkanShader const*, std::map<UniformBuffer const*, WriteDescriptorSetInfo>> s_CachedWriteDescriptorSets;

    static void UpdateWriteDescriptors(const Ref<UniformBufferSet>& uniformBufferSet, const Ref<VulkanShader>& shader, const std::vector<VulkanShader::UniformBufferInfo>& uniformBufferInfos)
    {
        VkDevice device = VulkanContext::Get()->GetDevice();

        for (const auto& ubInfo : uniformBufferInfos)
        {
            auto uniformBuffer = uniformBufferSet->Get(0, ubInfo.Binding); // Set is 0 for now

            // Retrieve or create shader map
            auto& shaderMap = s_CachedWriteDescriptorSets[shader.Get()];

            const auto& it = shaderMap.find(uniformBuffer.Get());
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

    struct VulkanRenderer
    {
        CommandQueue RenderQueue;

        CommandQueue ResourceQueue;
        Fly<CommandQueue> RuntimeResourceQueues;

        Ref<VertexBuffer> CubeMapVertexBuffer;
        Ref<IndexBuffer> CubeMapIndexBuffer;
        Ref<Texture2D> WhiteTexture;
        Ref<MaterialAsset> WhiteMaterial;
    };

    static VulkanRenderer* s_Renderer;

    void Renderer::Init()
    {
        s_Renderer = new VulkanRenderer;

        {
            // Since we dont need normals we can efficiently render a cube map
            // NOTE: There is possibly a better way (could be quad?)
            constexpr std::array<glm::vec3, 8> skyboxVertices = {
                glm::vec3{-1.0f, -1.0f,  1.0f},
                glm::vec3{ 1.0f, -1.0f, -1.0f},
                glm::vec3{ 1.0f, -1.0f,  1.0f},
                glm::vec3{-1.0f,  1.0f,  1.0f},

                glm::vec3{-1.0f, -1.0f, -1.0f},
                glm::vec3{ 1.0f, -1.0f, -1.0f},
                glm::vec3{ 1.0f,  1.0f, -1.0f},
                glm::vec3{-1.0f,  1.0f, -1.0f},
            };

            constexpr std::array<u32, 36> skyboxIndices = {
                0, 1, 2, 2, 3, 0,
                5, 4, 7, 7, 6, 5,
                4, 0, 3, 3, 7, 4,
                1, 5, 6, 6, 2, 1,
                3, 2, 6, 6, 7, 3,
                4, 5, 1, 1, 0, 4,
            };

            s_Renderer->CubeMapVertexBuffer = VertexBuffer::Create(skyboxVertices.data(), skyboxVertices.size() * sizeof(glm::vec3));
            s_Renderer->CubeMapIndexBuffer = IndexBuffer::Create(skyboxIndices.data(), (u32)skyboxIndices.size());
        }

        // Load shaders
        // TODO: Only load shaders that are used
        ShaderLibrary::Init();

        // Create default white texture
        s_Renderer->WhiteTexture = Texture2D::Create(0xffffffff);

        // Create default white material asset
        //s_Renderer->WhiteMaterial = Ref<MaterialAsset>::Create();
    }

    void Renderer::Shutdown()
    {
        GetResourceQueue().Execute();

        // Ensure that everything is freed
        for (auto& queue : s_Renderer->RuntimeResourceQueues)
            queue.Execute();

        ShaderLibrary::Shutdown();
        delete s_Renderer;
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

        GetResourceRuntimeQueue().Execute();
    }

    void Renderer::Render()
    {
        s_Renderer->RenderQueue.Execute();
    }

    void Renderer::Wait()
    {
        VulkanContext::Get()->GetDevice().WaitIdle();
    }

    // Command buffer functions
    // Command buffer functions
    // Command buffer functions

    void Renderer::SetLineWidth(Ref<RenderCommandBuffer> commandBuffer, f32 lineWidth)
    {
        Renderer::Submit([commandBuffer, lineWidth]()
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

    void Renderer::BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass)
    {
        Renderer::Submit([commandBuffer, renderPass]()
        {
            const auto& framebufferConfig = renderPass->GetConfig().TargetFrameBuffer->GetConfig();

            Renderer::SetViewport(commandBuffer, 0, 0, framebufferConfig.Width, framebufferConfig.Height);
            Renderer::SetScissor(commandBuffer, 0, 0, framebufferConfig.Width, framebufferConfig.Height);

            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();
            WeakRef<VulkanFrameBuffer> vkFramebuffer = renderPass->GetConfig().TargetFrameBuffer.As<VulkanFrameBuffer>();
            const auto& clearValues = vkFramebuffer->GetClearValues();
            VkRenderPass vkRenderpass = renderPass.As<VulkanRenderPass>()->GetHandle();

            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass = vkRenderpass;
            renderPassBeginInfo.renderArea.offset.x = 0;
            renderPassBeginInfo.renderArea.offset.y = 0;
            renderPassBeginInfo.renderArea.extent = { framebufferConfig.Width, framebufferConfig.Height };
            renderPassBeginInfo.clearValueCount = (u32)clearValues.size();
            renderPassBeginInfo.pClearValues = clearValues.data();
            renderPassBeginInfo.framebuffer = vkFramebuffer->GetHandle();

            vkCmdBeginRenderPass(vkCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        });
    }

    void Renderer::EndRenderPass(Ref<RenderCommandBuffer> commandBuffer)
    {
        Renderer::Submit([commandBuffer]()
        {
            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();
            vkCmdEndRenderPass(vkCommandBuffer);
        });
    }

    void Renderer::Draw(Ref<RenderCommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, Ref<Shader> shader, u32 vertexCount)
    {
        Renderer::Submit([commandBuffer, vertexBuffer, uniformBufferSet, pipeline, shader, vertexCount]()
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
        Renderer::Submit([commandBuffer, vertexBuffer, indexBuffer, uniformBufferSet, pipeline, shader, indexCount]()
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
        Renderer::Submit([commandBuffer, pipeline, size, data]()
        {
            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();
            VkPipelineLayout vkPipelineLayout = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineLayoutHandle();

            vkCmdPushConstants(vkCommandBuffer, vkPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, data);
        });
    }

    void Renderer::DrawInstanced(Ref<RenderCommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer, Ref<VertexBuffer> instanceBuffer, Ref<IndexBuffer> indexBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, u32 instanceCount, u32 indicesPerInstance)
    {
        Renderer::Submit([commandBuffer, vertexBuffer, instanceBuffer, indexBuffer, uniformBufferSet, pipeline, instanceCount, indicesPerInstance]()
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
        Renderer::Submit([commandBuffer, mesh, transformBuffer, uniformBufferSet, pipeline, transformOffset, subMeshIndex, instanceCount]()
        {
            Ref<MeshSource> meshSource = mesh->GetMeshSource();

            Ref<VertexBuffer> vertexBuffer = meshSource->GetVertexBuffer();
            Ref<VertexBuffer> indexBuffer = meshSource->GetIndexBuffer();
            Ref<VulkanShader> shader = meshSource->GetMeshShader();

            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();

            VkBuffer vkVertexBuffer = vertexBuffer.As<VulkanVertexBuffer>()->GetHandle();
            VkBuffer vkTransformBuffer = transformBuffer.As<VulkanVertexBuffer>()->GetHandle();
            VkBuffer vkIndexBuffer = indexBuffer.As<VulkanIndexBuffer>()->GetHandle();
            VkPipeline vkPipeline = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineHandle();
            VkPipelineLayout vkPipelineLayout = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineLayoutHandle();
            VkDescriptorSet vkDescriptorSet = shader->GetDescriptorSet();

            // Updating or creating descriptor sets
            const auto& resources = shader->GetResources();
            UpdateWriteDescriptors(uniformBufferSet, shader, resources.UniformBuffers); // TODO: We need separate write descriptors for each shader

            const auto& submesh = meshSource->GetSubmeshes()[subMeshIndex];

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

    void Renderer::DrawSkybox(Ref<RenderCommandBuffer> commandBuffer, Ref<GraphicsPipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet)
    {
        Renderer::Submit([commandBuffer, pipeline, uniformBufferSet]()
        {
            VkCommandBuffer vkCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();

            VkBuffer vkVertexBuffer = s_Renderer->CubeMapVertexBuffer.As<VulkanVertexBuffer>()->GetHandle();
            VkBuffer vkIndexBuffer = s_Renderer->CubeMapIndexBuffer.As<VulkanIndexBuffer>()->GetHandle();
            VkPipeline vkPipeline = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineHandle();
            VkPipelineLayout vkPipelineLayout = pipeline.As<VulkanGraphicsPipeline>()->GetPipelineLayoutHandle();

            Ref<VulkanShader> vkShader = pipeline->GetConfig().Shader.As<VulkanShader>();
            VkDescriptorSet vkDescriptorSet = vkShader->GetDescriptorSet();

            // Updating or creating descriptor sets
            const auto& resources = vkShader->GetResources();
            UpdateWriteDescriptors(uniformBufferSet, vkShader, resources.UniformBuffers); // TODO: We need separate write descriptors for each shader

            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, &vkVertexBuffer, offsets);
            vkCmdBindIndexBuffer(vkCommandBuffer, vkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
            vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet, 0, nullptr);

            vkCmdDrawIndexed(vkCommandBuffer, 36, 1, 0, 0, 0);
        });
    }


    void Renderer::CopyImageToBuffer(Ref<RenderCommandBuffer> commandBuffer, Ref<Image2D> image, Ref<RendererBuffer> rendererBuffer)
    {
        Renderer::Submit([commandBuffer, image, rendererBuffer]()
        {
            VkBuffer hostBuffer = rendererBuffer.As<VulkanBuffer>()->GetHandle();

            VkCommandBuffer currentCommandBuffer = commandBuffer.As<VulkanRenderCommandBuffer>()->GetHandle();
            VkImage selectionBufferAttachment = image.As<VulkanImage2D>()->GetImage();

            // Handle layout transition
            {
                VkImageMemoryBarrier imageBarrier = {};
                imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                imageBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                imageBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                imageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                imageBarrier.image = selectionBufferAttachment;

                vkCmdPipelineBarrier(currentCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
            }

            // Copy buffer
            {
                VkBufferImageCopy bufferImageCopyInfo{};
                bufferImageCopyInfo.bufferOffset = 0;
                bufferImageCopyInfo.bufferRowLength = 0;
                bufferImageCopyInfo.bufferImageHeight = 0;
                bufferImageCopyInfo.imageOffset = { 0, 0, 0 };
                bufferImageCopyInfo.imageExtent = { image->GetWidth(), image->GetHeight(), 1 };

                VkImageSubresourceLayers imageSubresource{};
                imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageSubresource.mipLevel = 0;
                imageSubresource.baseArrayLayer = 0;
                imageSubresource.layerCount = 1;
                bufferImageCopyInfo.imageSubresource = imageSubresource;

                vkCmdCopyImageToBuffer(currentCommandBuffer, selectionBufferAttachment, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, hostBuffer, 1, &bufferImageCopyInfo);
            }

            // Handle layout transition
            {
                VkImageMemoryBarrier imageBarrier = {};
                imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                imageBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                imageBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageBarrier.image = selectionBufferAttachment;

                vkCmdPipelineBarrier(currentCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
            }
        });
    }

    u32 Renderer::GetCurrentFrame()
    {
        return Application::Get().GetViewportWindow()->GetSwapchain()->GetCurrentFrame();
    }

    Ref<Texture2D> Renderer::GetWhiteTexture()
    {
        return s_Renderer->WhiteTexture;
    }

    Ref<MaterialAsset> Renderer::GetWhiteMaterial()
    {
        return s_Renderer->WhiteMaterial;
    }

    CommandQueue& Renderer::GetCommandQueue()
    {
        return s_Renderer->RenderQueue;
    }

    RendererContext* Renderer::GetContext()
    {
        return Application::Get().GetViewportWindow()->GetRendererContext();
    }

    CommandQueue& Renderer::GetResourceQueue()
    {
        return s_Renderer->ResourceQueue;
    }

    CommandQueue& Renderer::GetResourceRuntimeQueue()
    {
        u32 currentFrame = Renderer::GetCurrentFrame();
        return s_Renderer->RuntimeResourceQueues[currentFrame];
    }

}
