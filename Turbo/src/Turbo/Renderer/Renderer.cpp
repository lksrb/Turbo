#include "tbopch.h"
#include "Renderer.h"

#include "Turbo/Core/Engine.h"

#include "Renderer2D.h"
#include "SwapChain.h"

namespace Turbo
{
    struct Internal
    {
        RenderCommandQueue RenderQueue;
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

    u32 Renderer::GetCurrentFrame()
    {
        const Ref<SwapChain>& swapChain = Engine::Get().GetViewportWindow()->GetSwapchain();
        const u32 currentFrame = swapChain->GetCurrentFrame();

        return currentFrame;
    }

    void Renderer::Begin()
    {

    }

    void Renderer::Render()
    {
        s_Internal->RenderQueue.Execute();
    }
}
