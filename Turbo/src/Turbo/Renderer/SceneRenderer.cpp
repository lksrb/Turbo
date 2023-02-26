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

    void SceneRenderer::SetViewportSize(u32 width, u32 height)
    {
        m_Config.ViewportWidth = width;
        m_Config.ViewportHeight = height;
    }

    Ref<Image2D> SceneRenderer::GetFinalImage() const
    {
        const Ref<SwapChain>& swapChain = Engine::Get().GetViewportWindow()->GetSwapchain();
        const u32 currentFrame = swapChain->GetCurrentFrame();

        // Returns final image that is produced by the scene renderer
        return m_FinalFramebuffers[currentFrame]->GetConfig().Attachments[0].Image; // Color attachment
    }

    void SceneRenderer::Init()
    {
        // Create Renderer2D
        m_Renderer2D = Renderer2D::Create();

        if (m_Config.RenderIntoTexture)
        {
            m_FinalFramebuffers.resize(TBO_MAX_FRAMESINFLIGHT);

            // Create framebuffers that will be used for rendering
            const Ref<VulkanSwapChain>& swapChain = Engine::Get().GetViewportWindow()->GetSwapchain().As<VulkanSwapChain>();

            // Separate framebuffers 
            for (u32 i = 0; i < RendererContext::FramesInFlight(); ++i)
            {
                FrameBuffer::Config framebufferConfig = {};

                // Color attachment
                {
                    Image2D::Config config = {};
                    config.ImageFormat = Image2D::Format_BGRA8_SRGB;
                    config.Aspect = Image2D::AspectFlags_Color;
                    config.Storage = Image2D::MemoryPropertyFlags_DeviceLocal;
                    config.Usage = Image2D::ImageUsageFlags_ColorAttachment | Image2D::ImageUsageFlags_Sampled | Image2D::ImageUsageFlags_Transfer_Source;
                    config.ImageTiling = Image2D::ImageTiling_Optimal;

                    Ref<Image2D> renderImage = Image2D::Create(config);
                    renderImage->Invalidate(m_Config.ViewportWidth, m_Config.ViewportHeight);

                    FrameBuffer::Attachment colorAttachment = {};
                    colorAttachment.Image = renderImage;
                    colorAttachment.ColorMask = FrameBuffer::ColorWriteMask_RGBA;
                    colorAttachment.EnableBlend = true;
                    colorAttachment.BlendOperation = FrameBuffer::BlendOperation_Add;
                    colorAttachment.SrcBlendFactor = FrameBuffer::BlendFactor_SrcAlpha;
                    colorAttachment.DstBlendFactor = FrameBuffer::BlendFactor_OneMinus_SrcAlpha;

                    // Framebuffer
                    framebufferConfig.Attachments[0/*REWRITE THIS*/] = colorAttachment;
                    framebufferConfig.AttachmentsCount = 1;
                    framebufferConfig.DepthBuffer = swapChain->GetDepthBuffer();
                    framebufferConfig.Renderpass = swapChain->GetRenderPass();

                    m_FinalFramebuffers[i] = FrameBuffer::Create(framebufferConfig);
                    m_FinalFramebuffers[i]->Invalidate(m_Config.ViewportWidth, m_Config.ViewportHeight);
                }
            }

            m_Renderer2D->SetRenderTarget(m_FinalFramebuffers);
        }
        else
        {
            TBO_ENGINE_ASSERT(false, "Not implemented yet!");
        }

        // Initialize Renderer2D
        m_Renderer2D->Initialize();
    }

}
