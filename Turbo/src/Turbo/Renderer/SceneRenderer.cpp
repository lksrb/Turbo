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
                config.DestinationLayout = RenderPass::ImageLayout_Shader_ReadOnly_Optimal;
                m_Renderpass = RenderPass::Create(config);
                m_Renderpass->Invalidate();
            }

            // Create framebuffers that will be used for rendering
            m_FinalFramebuffers.resize(frames_in_flight);

            // Separate framebuffers 
            for (u32 i = 0; i < frames_in_flight; ++i)
            {
                FrameBuffer::Config framebuffer_config = {};

                // Color attachment
                {
                    Image2D::Config config = {};
                    config.ImageFormat = Image2D::Format_RGBA8_Unorm;
                    config.Aspect = Image2D::AspectFlags_Color;
                    config.Storage = Image2D::MemoryPropertyFlags_DeviceLocal;
                    config.Usage = Image2D::ImageUsageFlags_ColorAttachment | Image2D::ImageUsageFlags_Sampled;
                    config.ImageTiling = Image2D::ImageTiling_Optimal;

                    Ref<Image2D> render_image = Image2D::Create(config);
                    render_image->Invalidate(m_Config.ViewportWidth, m_Config.ViewportHeight);

                    FrameBuffer::Attachment color_attachment = {};
                    color_attachment.Image = render_image;
                    color_attachment.ColorMask = FrameBuffer::ColorWriteMask_RGBA;
                    color_attachment.EnableBlend = true;
                    color_attachment.BlendOperation = FrameBuffer::BlendOperation_Add;
                    color_attachment.SrcBlendFactor = FrameBuffer::BlendFactor_SrcAlpha;
                    color_attachment.DstBlendFactor = FrameBuffer::BlendFactor_OneMinus_SrcAlpha;

                    // Framebuffer
                    framebuffer_config.ColorAttachment = color_attachment;
                    framebuffer_config.DepthBuffer = nullptr;
                    framebuffer_config.Renderpass = m_Renderpass;

                    m_FinalFramebuffers[i] = FrameBuffer::Create(framebuffer_config);
                    m_FinalFramebuffers[i]->Invalidate(m_Config.ViewportWidth, m_Config.ViewportHeight);
                }
            }

            m_Renderer2D->SetRenderTarget(m_FinalFramebuffers, m_Renderpass);
            m_Renderer2D->OnViewportResize(m_Config.ViewportWidth, m_Config.ViewportHeight);

            // Separate command buffer
            m_RenderCommandBuffer = CommandBuffer::Create(CommandBufferLevel::Primary);
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

        m_Renderer2D->OnViewportResize(m_Config.ViewportWidth, m_Config.ViewportHeight);
    }

    Ref<Image2D> SceneRenderer::GetFinalImage() const
    {
        // Returns final image that is produced by the scene renderer
        u32 current_frame = Renderer::GetCurrentFrame();
        return m_FinalFramebuffers[current_frame]->GetConfig().ColorAttachment.Image; // Color attachment
    }
}
