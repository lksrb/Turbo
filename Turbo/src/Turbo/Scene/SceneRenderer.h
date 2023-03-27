#pragma once

#include "Turbo/Renderer/Renderer2D.h"

namespace Turbo
{
    class SceneRenderer
    {
    public:
        struct Config
        {
            u32 ViewportWidth;
            u32 ViewportHeight;
            bool RenderIntoTexture;
        };

        SceneRenderer(const SceneRenderer::Config& config);
        ~SceneRenderer();

        void BeginRender();
        void EndRender();

        void OnViewportSize(u32 width, u32 height);
        u32 GetViewportWidth() const { return m_Config.ViewportWidth; }
        u32 GetViewportHeight() const { return m_Config.ViewportHeight; }
        Ref<Renderer2D> GetRenderer2D() { return m_Renderer2D; }
        Ref<Image2D> GetFinalImage() const;

        void ClearFramebuffer();
    private:
        void Init();
    private:
        Ref<FrameBuffer> m_TargetFramebuffer;
        Ref<RenderCommandBuffer> m_RenderCommandBuffer;

        Ref<RenderPass> m_Renderpass;
        Ref<Renderer2D> m_Renderer2D;

        SceneRenderer::Config m_Config;
    };
}

