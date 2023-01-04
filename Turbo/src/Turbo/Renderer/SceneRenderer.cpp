#include "tbopch.h"
#include "SceneRenderer.h"

#include "Turbo/Scene/Scene.h"

namespace Turbo
{
    SceneRenderer::SceneRenderer()
        : m_ViewportWidth(0), m_ViewportHeight(0)
    {
    }

    SceneRenderer::~SceneRenderer()
    {
    }

    void SceneRenderer::SetViewportSize(u32 width, u32 height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;
    }

}
