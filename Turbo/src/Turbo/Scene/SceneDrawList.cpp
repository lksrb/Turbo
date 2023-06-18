#include "tbopch.h"
#include "SceneDrawList.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Core/Engine.h"

#include "Turbo/Platform/Vulkan/VulkanSwapChain.h"

namespace Turbo
{
    SceneDrawList::SceneDrawList(const SceneDrawList::Config& config)
        : m_Config(config)
    {
        Init();
    }

    SceneDrawList::~SceneDrawList()
    {
    }

    void SceneDrawList::Init()
    {
        // Separate command buffer
        m_RenderCommandBuffer = RenderCommandBuffer::Create();

        // Color attachment
        FrameBuffer::Attachment colorAttachment = {};
        colorAttachment.ColorMask = FrameBuffer::ColorWriteMask_RGBA;
        colorAttachment.EnableBlend = true;
        colorAttachment.BlendOperation = FrameBuffer::BlendOperation_Add;
        colorAttachment.SrcBlendFactor = FrameBuffer::BlendFactor_SrcAlpha;
        colorAttachment.DstBlendFactor = FrameBuffer::BlendFactor_OneMinus_SrcAlpha;

        // Target Framebuffer
        // This will be the main canvas
        FrameBuffer::Config frameBufferConfig = {};
        frameBufferConfig.ColorAttachment = colorAttachment;
        frameBufferConfig.EnableDepthTesting = true;

        Ref<FrameBuffer> targetFrameBuffer = FrameBuffer::Create(frameBufferConfig);

        // Main render pass
        RenderPass::Config config = {};
        config.TargetFrameBuffer = targetFrameBuffer;
        config.ClearOnLoad = true;
        m_CompositeRenderPass = RenderPass::Create(config);
        m_CompositeRenderPass->Invalidate();

        targetFrameBuffer->SetRenderPass(m_CompositeRenderPass);
        targetFrameBuffer->Invalidate(m_Config.ViewportWidth, m_Config.ViewportHeight);

        // Create Renderer2D
        m_DrawList2D = DrawList2D::Create();
        m_DrawList2D->SetTargetRenderPass(m_CompositeRenderPass);
        m_DrawList2D->Initialize();
    }

    void SceneDrawList::SetCamera(const Camera& camera)
    {
        m_DrawList2D->SetCameraTransform(camera.GetViewProjection());
    }

    void SceneDrawList::Begin()
    {
        m_DrawList2D->Begin();
    }

    void SceneDrawList::End()
    {
        m_DrawList2D->End();

        m_Statistics.Statistics2D = m_DrawList2D->GetStatistics();
    }

    void SceneDrawList::AddQuad(const glm::vec3& position, const glm::vec2& size, f32 rotation, const glm::vec4& color, i32 entity)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        AddQuad(transform, color, entity);
    }

    void SceneDrawList::AddQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        m_DrawList2D->AddQuad(transform, color, entity);
    }

    void SceneDrawList::AddSprite(const glm::mat4& transform, const glm::vec4& color, Ref<SubTexture2D> subTexture, f32 tiling, i32 entity)
    {
        m_DrawList2D->AddSprite(transform, color, subTexture, tiling, entity);
    }

    void SceneDrawList::AddLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, i32 entity)
    {
        m_DrawList2D->AddLine(p0, p1, color, entity);
    }

    void SceneDrawList::AddCircle(const glm::mat4& transform, const glm::vec4& color, f32 thickness, f32 fade, i32 entity)
    {
        m_DrawList2D->AddCircle(transform, color, thickness, fade, entity);
    }

    void SceneDrawList::AddRect(const glm::vec3& position, const glm::vec2& size, f32 rotation, const glm::vec4& color, i32 entity)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        AddRect(transform, color, entity);
    }

    void SceneDrawList::AddRect(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        m_DrawList2D->AddRect(transform, color, entity);
    }

    void SceneDrawList::AddString(const glm::mat4& transform, const glm::vec4& color, Ref<Font> font, const std::string& string, f32 kerningOffset, f32 lineSpacing, i32 entity)
    {
        m_DrawList2D->AddString(transform, color, font, string, kerningOffset, lineSpacing, entity);
    }

    void SceneDrawList::OnViewportResize(u32 width, u32 height)
    {
        m_Config.ViewportWidth = width;
        m_Config.ViewportHeight = height;

        // Should framebuffer resize?
        /*Renderer::Submit([this, width, height]()
        {
            //m_CompositeRenderPass->GetConfig().TargetFrameBuffer->Invalidate(width, height);
            //m_DrawList2D->OnViewportResize(width, height);
        });*/
    }

    Ref<Image2D> SceneDrawList::GetFinalImage() const
    {
        // Returns final image that is produced by the draw list
        return m_CompositeRenderPass->GetConfig().TargetFrameBuffer->GetColorAttachment();
    }
}
