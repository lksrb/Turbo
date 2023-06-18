#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Renderer/Image2D.h"
#include "Turbo/Renderer/FrameBuffer.h"

namespace Turbo
{
    class RenderPass
    {
    public:
        struct Config
        {
            Ref<FrameBuffer> TargetFrameBuffer;
            bool ClearOnLoad = true;
        };

        static Ref<RenderPass> Create(const RenderPass::Config& config);
        virtual ~RenderPass();

        RenderPass::Config GetConfig() const { return m_Config; }

        virtual void Invalidate() = 0;
    protected:
        RenderPass(const RenderPass::Config& config);

        RenderPass::Config m_Config;
    };
}
