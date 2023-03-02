#include "tbopch.h"

#include "Renderer2D.h"

#include "Turbo/Core/Engine.h"
#include "Turbo/Benchmark/ScopeTimer.h"

#include "Turbo/Platform/Vulkan/VulkanSwapChain.h"
#include "Turbo/Platform/Vulkan/VulkanBuffer.h"
#include "Turbo/Platform/Vulkan/VulkanRenderPass.h"
#include "Turbo/Platform/Vulkan/VulkanCommandBuffer.h"
#include "Turbo/Platform/Vulkan/VulkanImage2D.h"
#include "Turbo/Platform/Vulkan/VulkanIndexBuffer.h"
#include "Turbo/Platform/Vulkan/VulkanVertexBuffer.h"
#include "Turbo/Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "Turbo/Platform/Vulkan/VulkanShader.h"

#include <vulkan/vulkan.h>

#include <glm/ext/matrix_transform.hpp>

namespace Turbo
{
    Renderer2D::Renderer2D()
    {
    }

    Renderer2D::~Renderer2D()
    {
        Shutdown();
    }

    void Renderer2D::InitializeRender()
    {
        // Renderpass
      /*  RenderPass::Config renderPassConfig = {};
        //renderPassConfig.DepthAttachment = m_DepthImage;
        m_RenderPass = RenderPass::Create(renderPassConfig);
        m_RenderPass->Invalidate();*/

        // TODO: Separate swapchain from rendering
        Ref<VulkanSwapChain> swapChain = Engine::Get().GetViewportWindow()->GetSwapchain().As<VulkanSwapChain>();
        //m_RenderPass = swapChain->GetRenderPass();

        // Render images
        /*for (u32 i = 0; i < RendererContext::FramesInFlight(); ++i)
        {
            Image2D::Config config = {};
            config.ImageFormat = Image2D::Format_BGRA8_SRGB;
            config.Aspect = Image2D::AspectFlags_Color;
            config.Storage = Image2D::MemoryPropertyFlags_DeviceLocal;
            config.Usage = Image2D::ImageUsageFlags_ColorAttachment | Image2D::ImageUsageFlags_Sampled | Image2D::ImageUsageFlags_Transfer_Source;
            config.ImageTiling = Image2D::ImageTiling_Optimal;

            m_RenderImages[i] = Image2D::Create(config);
            m_RenderImages[i]->Invalidate(m_ViewportWidth, m_ViewportHeight);
        }

        // Framebuffers
        for (u32 i = 0; i < RendererContext::FramesInFlight(); ++i)
        {
            FrameBuffer::Config framebufferConfig = {};

            // Color attachment
            FrameBuffer::Attachment colorAttachment = {};
            colorAttachment.Image = m_RenderImages[i];
            colorAttachment.ColorMask = FrameBuffer::ColorWriteMask_RGBA;
            colorAttachment.EnableBlend = true;
            colorAttachment.BlendOperation = FrameBuffer::BlendOperation_Add;
            colorAttachment.SrcBlendFactor = FrameBuffer::BlendFactor_SrcAlpha;
            colorAttachment.DstBlendFactor = FrameBuffer::BlendFactor_OneMinus_SrcAlpha;
            framebufferConfig.Attachments[0] = colorAttachment;
            framebufferConfig.AttachmentsCount = 1;
            framebufferConfig.DepthBuffer = swapChain->GetDepthBuffer();
            framebufferConfig.Renderpass = swapChain->GetRenderPass();

            m_Framebuffers[i] = FrameBuffer::Create(framebufferConfig);
            m_Framebuffers[i]->Invalidate(m_ViewportWidth, m_ViewportHeight);
        }*/

        m_CommandBuffer = CommandBuffer::Create(CommandBufferLevel::Primary);

        // Default clear color
        m_ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    void Renderer2D::Initialize()
    {
        // Framebuffers, commandbuffers, ...
        InitializeRender();

        // Quad setup
        {
            // Index buffer
            {
                u32* quadIndices = new u32[MaxIndices];
                u32 offset = 0;
                for (u32 i = 0; i < MaxIndices; i += 6)
                {
                    quadIndices[i + 0] = offset + 0;
                    quadIndices[i + 1] = offset + 1;
                    quadIndices[i + 2] = offset + 2;

                    quadIndices[i + 3] = offset + 2;
                    quadIndices[i + 4] = offset + 3;
                    quadIndices[i + 5] = offset + 0;

                    offset += 4;
                }

                IndexBuffer::Config config = {};
                config.Size = MaxIndices * sizeof(u32);
                config.Indices = quadIndices;
                m_QuadIndexBuffer = IndexBuffer::Create(config);

                delete[] quadIndices;
            }
            // Vertex buffer
            m_QuadVertexBufferBase = m_QuadVertexBufferPointer = new QuadVertex[MaxVertices];

            m_QuadVertexBuffer = VertexBuffer::Create({ MaxVertices * sizeof(QuadVertex) });

            // Shader
            Shader::Config shaderConfig = {};
            shaderConfig.Language = ShaderLanguage::GLSL;
            shaderConfig.ShaderPath = "assets\\Shaders\\Renderer2D_Quad.glsl";
            m_QuadShader = Shader::Create(shaderConfig);

            // Graphics pipeline
            GraphicsPipeline::Config config = {};
            config.Shader = m_QuadShader;
            config.Renderpass = m_RenderPass;
            config.Topology = PrimitiveTopology::Triangle;
            config.DepthTesting = true;
            config.TargetFramebuffer = m_Framebuffers[0]; // All framebuffers are identical.
            m_QuadPipeline = GraphicsPipeline::Create(config);
            m_QuadPipeline->Invalidate();
        }

        m_QuadMaterial = Material::Create({ m_QuadShader });

        // White texture for texture-less quads
        m_WhiteTexture = Texture2D::Create(0xffffffff);

        // Set white texture at the top of the stack
        m_TextureSlots[0] = m_WhiteTexture;
    }

    void Renderer2D::Shutdown()
    {
        delete m_QuadVertexBufferBase;
    }

    void Renderer2D::Begin(const Camera& camera)
    {
        // Enable drawing
        m_BeginDraw = true;

        m_QuadMaterial->Set("u_Camera", camera.GetViewProjection());

        // Reset texture indexing
        m_TextureSlotsIndex = 1;

        // Reset textures
        for (uint32_t i = 1; i < m_TextureSlots.size(); i++)
            m_TextureSlots[i] = nullptr;

        StartBatch();
    }

    void Renderer2D::StartBatch()
    {
        m_QuadVertexBufferPointer = m_QuadVertexBufferBase;

        // Reset statistics
        m_Statistics.Reset();
    }

    void Renderer2D::End()
    {
        Flush();
        m_BeginDraw = false;
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, f32 rotation, const glm::vec4& color, i32 entityID)
    {
        TBO_ENGINE_ASSERT(m_BeginDraw, "Call Begin() before issuing a draw command!");

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, color, entityID);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, i32 entityID)
    {
        TBO_ENGINE_ASSERT(m_BeginDraw, "Call Begin() before issuing a draw command!");

        /*if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices) // // TODO(Urby): Flushing batch renderer
            NextBatch();*/

        constexpr u32 quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        for (u32 i = 0; i < quadVertexCount; ++i)
        {
            m_QuadVertexBufferPointer->Position = transform * QuadVertexPositions[i];
            m_QuadVertexBufferPointer->Color = color;
            m_QuadVertexBufferPointer->EntityID = entityID;
            m_QuadVertexBufferPointer->TextureIndex = 0;
            m_QuadVertexBufferPointer->TexCoord = textureCoords[i];
            m_QuadVertexBufferPointer->TilingFactor = 0.0f;
            m_QuadVertexBufferPointer++;
        }

        m_Statistics.QuadIndexCount += 6;

        m_Statistics.QuadCount++;
    }

    void Renderer2D::DrawSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, f32 tiling, i32 entityID /*= -1*/)
    {
        TBO_ENGINE_ASSERT(m_BeginDraw, "Call Begin() before issuing a draw command!");

        if (m_Statistics.QuadIndexCount >= Renderer2D::MaxIndices)
            TBO_ENGINE_ASSERT(false); // TODO(Urby): Flush and reset

        u32 textureIndex = 0; // White Texture
        constexpr u32 quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        if (texture)
        {
            for (u32 i = 1; i < m_TextureSlotsIndex; ++i)
            {
                // If the texture is in the stack, just modify textureIndex
                if (texture->GetHash() == m_TextureSlots[i]->GetHash())
                {
                    textureIndex = i;
                    break;
                }
            }

            // If the texture is not present in texture stack, add it
            if (textureIndex == 0)
            {
                TBO_ENGINE_ASSERT(m_TextureSlotsIndex < MaxTextureSlots); // TODO: Flush and reset

                textureIndex = m_TextureSlotsIndex;
                m_TextureSlots[m_TextureSlotsIndex] = texture;
                m_TextureSlotsIndex++;
            }
        }

        for (u32 i = 0; i < quadVertexCount; ++i)
        {
            m_QuadVertexBufferPointer->Position = transform * QuadVertexPositions[i];
            m_QuadVertexBufferPointer->Color = color;
            m_QuadVertexBufferPointer->EntityID = entityID;
            m_QuadVertexBufferPointer->TextureIndex = textureIndex;
            m_QuadVertexBufferPointer->TexCoord = textureCoords[i];
            m_QuadVertexBufferPointer->TilingFactor = tiling;
            m_QuadVertexBufferPointer++;
        }

        m_Statistics.QuadIndexCount += 6;

        m_Statistics.QuadCount++;
    }

    void Renderer2D::DrawSprite(const glm::mat4& transform, const glm::vec4& color, Ref<SubTexture2D> subTexture, f32 tiling, i32 entityID /*= -1*/)
    {
        TBO_ENGINE_ASSERT(m_BeginDraw, "Call Begin() before issuing a draw command!");

        if (m_Statistics.QuadIndexCount >= Renderer2D::MaxIndices)
            TBO_ENGINE_ASSERT(false); // TODO(Urby): Flush and reset

        u32 textureIndex = 0; // White Texture
        constexpr u32 quadVertexCount = 4;
        glm::vec2 textureCoords[4];
        memcpy(textureCoords, subTexture->GetCoords().data(), 4 * sizeof(glm::vec2));

        if (subTexture)
        {
            for (u32 i = 1; i < m_TextureSlotsIndex; ++i)
            {
                // If the texture is in the stack, just modify textureIndex
                if (subTexture->GetTexture()->GetHash() == m_TextureSlots[i]->GetHash())
                {
                    textureIndex = i;
                    break;
                }
            }

            // If the texture is not present in texture stack, add it
            if (textureIndex == 0)
            {
                TBO_ENGINE_ASSERT(m_TextureSlotsIndex < MaxTextureSlots); // TODO: Flush and reset

                textureIndex = m_TextureSlotsIndex;
                m_TextureSlots[m_TextureSlotsIndex] = subTexture->GetTexture();
                m_TextureSlotsIndex++;
            }
        }

        for (u32 i = 0; i < quadVertexCount; ++i)
        {
            m_QuadVertexBufferPointer->Position = transform * QuadVertexPositions[i];
            m_QuadVertexBufferPointer->Color = color;
            m_QuadVertexBufferPointer->EntityID = entityID;
            m_QuadVertexBufferPointer->TextureIndex = textureIndex;
            m_QuadVertexBufferPointer->TexCoord = textureCoords[i];
            m_QuadVertexBufferPointer->TilingFactor = tiling;
            m_QuadVertexBufferPointer++;
        }

        m_Statistics.QuadIndexCount += 6;

        m_Statistics.QuadCount++;
    }

    void Renderer2D::OnViewportResize(u32 width, u32 height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;
    }

    void Renderer2D::Flush()
    {
        Renderer::Submit([this]()
        {
            if (m_Statistics.QuadIndexCount)
            {
                // Texture slots
                for (u32 i = 0; i < m_TextureSlots.size(); ++i)
                {
                    if (m_TextureSlots[i])
                    {
                        m_QuadMaterial->Set("u_Textures", m_TextureSlots[i], i);
                    }
                    else
                    {
                        m_QuadMaterial->Set("u_Textures", m_WhiteTexture, i);
                    }
                }

                m_QuadMaterial->Update();

                uint32_t dataSize = (uint32_t)((uint8_t*)m_QuadVertexBufferPointer - (uint8_t*)m_QuadVertexBufferBase);
                m_QuadVertexBuffer->SetData(m_QuadVertexBufferBase, dataSize); // TODO: Figure out how to submit transfering data
            }

            // Record buffer
            u32 currentFrame = Renderer::GetCurrentFrame();
            u32 windowWidth = m_ViewportWidth;
            u32 windowHeight = m_ViewportHeight;


          /*  VkCommandBuffer currentBuffer = m_RenderCommandBuffer.As<VulkanCommandBuffer>()->GetCommandBuffer();

            / * VkCommandBufferInheritanceInfo inheritanceInfo = {};
             inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
             inheritanceInfo.renderPass = m_RenderPass.As<VulkanRenderPass>()->GetRenderPass();
             inheritanceInfo.framebuffer = m_Framebuffers[currentFrame].As<VulkanFrameBuffer>()->GetFrameBuffer();* /

            VkCommandBufferBeginInfo cmdBufInfo = {};
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            cmdBufInfo.pInheritanceInfo = nullptr;*/

            //TBO_VK_ASSERT(vkBeginCommandBuffer(currentBuffer, &cmdBufInfo));
            m_CommandBuffer->Begin();
            VkCommandBuffer currentBuffer = m_CommandBuffer.As<VulkanCommandBuffer>()->GetCommandBuffer();
            {
                // TODO: Abstract this
                VkClearValue clearValues[2]{};
                clearValues[0].color = { {0.0f, 0.0f,0.0f, 1.0f} };
                clearValues[1].depthStencil = { 1.0f, 0 };

                VkRenderPassBeginInfo renderPassBeginInfo = {};
                renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassBeginInfo.renderPass = m_RenderPass.As<VulkanRenderPass>()->GetRenderPass();
                renderPassBeginInfo.renderArea.offset.x = 0;
                renderPassBeginInfo.renderArea.offset.y = 0;
                renderPassBeginInfo.renderArea.extent = { m_ViewportWidth, m_ViewportHeight };
                renderPassBeginInfo.clearValueCount = 2; // Color
                renderPassBeginInfo.pClearValues = clearValues;
                renderPassBeginInfo.framebuffer = m_Framebuffers[currentFrame].As<VulkanFrameBuffer>()->GetFrameBuffer();

                vkCmdBeginRenderPass(currentBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
                // Draw
                if (m_Statistics.QuadIndexCount)
                {
                    VkViewport viewport = {};
                    viewport.x = 0.0f;
                    viewport.y = 0.0f;
                    viewport.width = static_cast<f32>(windowWidth);
                    viewport.height = static_cast<f32>(windowHeight);
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                    vkCmdSetViewport(currentBuffer, 0, 1, &viewport);

                    VkRect2D scissor = {};
                    scissor.offset = { 0,0 };
                    scissor.extent = { windowWidth, windowHeight };
                    vkCmdSetScissor(currentBuffer, 0, 1, &scissor);

                    // ---
                    VkBuffer vertexBuffer = m_QuadVertexBuffer.As<VulkanVertexBuffer>()->GetBuffer();
                    VkBuffer indexBuffer = m_QuadIndexBuffer.As<VulkanIndexBuffer>()->GetBuffer();
                    VkPipeline pipeline = m_QuadPipeline.As<VulkanGraphicsPipeline>()->GetPipeline();
                    VkPipelineLayout pipelineLayout = m_QuadPipeline.As<VulkanGraphicsPipeline>()->GetPipelineLayout();
                    VkDescriptorSet descriptorSet = m_QuadShader.As<VulkanShader>()->GetDescriptorSet();

                    VkDeviceSize offsets[] = { 0 };
                    vkCmdBindVertexBuffers(currentBuffer, 0, 1, &vertexBuffer, offsets);
                    vkCmdBindIndexBuffer(currentBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                    vkCmdBindPipeline(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

                    if (descriptorSet) // TODO: Make this more convenient
                        vkCmdBindDescriptorSets(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

                    vkCmdDrawIndexed(currentBuffer, m_Statistics.QuadIndexCount, 1, 0, 0, 0);
                }

                vkCmdEndRenderPass(currentBuffer);

                m_CommandBuffer->End();
                m_CommandBuffer->Submit();
            }
            //TBO_VK_ASSERT(vkEndCommandBuffer(currentBuffer));

            // Submit secondary buffer
            //swapChain->SubmitSecondary(currentBuffer);

            // Increment draw calls
            m_Statistics.DrawCalls++;
        });

    }

}
