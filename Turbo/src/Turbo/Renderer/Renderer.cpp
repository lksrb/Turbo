#include "tbopch.h"

#include "Turbo/Renderer/Renderer.h"
#include "Turbo/Renderer/Renderer2D.h"

namespace Turbo
{
    struct Internal
    {
        RenderCommandQueue RenderQueue;
        RenderCommandQueue SecondaryCommandQueue;
    };

    static Internal* s_Internal;

    void Renderer::Initialize()
    {
        s_Internal = new Internal;
    }

    void Renderer::Shutdown()
    {
        delete s_Internal;
    }

    RenderCommandQueue& Renderer::GetRenderCommandQueue()
    {
        return s_Internal->RenderQueue;
    }

    RenderCommandQueue& Renderer::GetSecondaryCommandQueue()
    {
        return s_Internal->SecondaryCommandQueue;
    }

    void Renderer::Begin()
    {

    }

    void Renderer::Render()
    {
        s_Internal->RenderQueue.Execute();
    }

    void Renderer::BuildSecondary()
    {
        s_Internal->RenderQueue.Execute();
    }

}
