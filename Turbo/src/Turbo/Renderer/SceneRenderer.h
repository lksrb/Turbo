#pragma once

#include "Turbo/Renderer/Renderer2D.h"

namespace Turbo
{
    class SceneRenderer
    {
    public:
        SceneRenderer();
        ~SceneRenderer();

        void SetViewportSize(u32 width, u32 height);

        u32 GetViewportWidth() const { return m_ViewportWidth; }
        u32 GetViewportHeight() const { return m_ViewportHeight; }

        Renderer2D& GetRenderer2D() { return m_Renderer2D; }
    private:
        Renderer2D m_Renderer2D;

        u32 m_ViewportWidth;
        u32 m_ViewportHeight;
    };
}

