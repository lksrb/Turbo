#include "tbopch.h"
#include "SceneRenderer.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Core/Engine.h"

#include "Turbo/Platform/Vulkan/VulkanSwapChain.h"

namespace Turbo
{
    SceneRenderer::SceneRenderer(const SceneRenderer::Config& config)
        : m_Config(config)
    {
        Init();
    }

    SceneRenderer::~SceneRenderer()
    {
    }

    void SceneRenderer::Init()
    {
        // Create Renderer2D
        m_Renderer2D = Renderer2D::Create();

        if (m_Config.RenderIntoTexture)
        {
            u32 frames_in_flight = RendererContext::FramesInFlight();

            // Separate RenderPass
            {
                RenderPass::Config config = {};
                config.DestinationLayout = ImageLayout_Shader_ReadOnly_Optimal;
                config.EnableDepthTesting = true;
                m_Renderpass = RenderPass::Create(config);
                m_Renderpass->Invalidate();
            }

            // Create framebuffers that will be used for rendering
            {
                // Color attachment
                FrameBuffer::Attachment colorAttachment = {};
                colorAttachment.ColorMask = FrameBuffer::ColorWriteMask_RGBA;
                colorAttachment.EnableBlend = true;
                colorAttachment.BlendOperation = FrameBuffer::BlendOperation_Add;
                colorAttachment.SrcBlendFactor = FrameBuffer::BlendFactor_SrcAlpha;
                colorAttachment.DstBlendFactor = FrameBuffer::BlendFactor_OneMinus_SrcAlpha;

                // Framebuffer
                FrameBuffer::Config frameBufferConfig = {};
                frameBufferConfig.ColorAttachment = colorAttachment;
                frameBufferConfig.EnableDepthTesting = true;
                frameBufferConfig.Renderpass = m_Renderpass;

                m_TargetFramebuffer = FrameBuffer::Create(frameBufferConfig);
                m_TargetFramebuffer->Invalidate(m_Config.ViewportWidth, m_Config.ViewportHeight);
            }

            m_Renderer2D->SetRenderTarget(m_TargetFramebuffer);

            // Separate command buffer
            m_RenderCommandBuffer = RenderCommandBuffer::Create();
        }
        else
        {
//            Ref<VulkanSwapChain> swap_chain = Engine::Get().GetViewportWindow()->GetSwapchain().As<VulkanSwapChain>();

            TBO_ENGINE_ASSERT(false, "Not implemented yet!");
        }

        // Initialize Renderer2D
        m_Renderer2D->Initialize();
    }

    void SceneRenderer::BeginRender()
    {

    }

    void SceneRenderer::EndRender()
    {
        if (m_Config.RenderIntoTexture) // Render into ImGui viewport window
        {
            // Currently only renderer2D is submitting work so this is empty
            // TODO: More renderers  
/*
            Renderer::Submit([this]()
            {
                m_RenderCommandBuffer->Begin();
                {
                     
                }
                m_RenderCommandBuffer->End();
                m_RenderCommandBuffer->Submit();
            });*/
        }
        else // Render into window
        {
            TBO_ENGINE_ASSERT(false, "Not implemented yet.");
/*          Renderer::Submit([this]()
            {
                // Swapchain primary command buffer
                const Window* viewportWindow = Engine::Get().GetViewportWindow();
                Ref<VulkanSwapChain> swapChain = viewportWindow->GetSwapchain().As<VulkanSwapChain>();
                u32 width = viewportWindow->GetWidth();
                u32 height = viewportWindow->GetHeight();
                u32 currentFrame = swapChain->GetCurrentFrame();

                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.pNext = nullptr;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                beginInfo.pInheritanceInfo = nullptr;
                VkCommandBuffer currentBuffer = swapChain->GetCurrentRenderCommandBuffer();
                TBO_VK_ASSERT(vkBeginCommandBuffer(currentBuffer, &beginInfo));

                VkClearValue clearValues[2]{};
                clearValues[0].color = { {0.0f, 0.0f,0.0f, 1.0f} };
                clearValues[1].depthStencil = { 1.0f, 0 };

                VkRenderPassBeginInfo renderPassBeginInfo = {};
                renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassBeginInfo.renderPass = swapChain->GetRenderPass();
                renderPassBeginInfo.renderArea.offset.x = 0;
                renderPassBeginInfo.renderArea.offset.y = 0;
                renderPassBeginInfo.renderArea.extent = { width, height };
                renderPassBeginInfo.clearValueCount = 2; // Color
                renderPassBeginInfo.pClearValues = clearValues;
                renderPassBeginInfo.framebuffer = swapChain->GetCurrentFramebuffer();

                vkCmdBeginRenderPass(currentBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
                {
                    // Execute every secondary command buffers
                    vkCmdExecuteCommands(currentBuffer, 1, &m_SecondaryBuffers[currentFrame]);
                }

                vkCmdEndRenderPass(currentBuffer);

                // End swapchain's primary secondary buffer
                TBO_VK_ASSERT(vkEndCommandBuffer(currentBuffer));
            });*/
        }
    }

    void SceneRenderer::OnViewportSize(u32 width, u32 height)
    {
        m_Config.ViewportWidth = width;
        m_Config.ViewportHeight = height;

        // TODO: Invalidate framebuffers
    }

    Ref<Image2D> SceneRenderer::GetFinalImage() const
    {
        // Returns final image that is produced by the scene renderer
        return m_TargetFramebuffer->GetColorAttachment(); 
    }
}
