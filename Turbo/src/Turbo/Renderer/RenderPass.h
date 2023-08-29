#pragma once

#include "Image2D.h"
#include "FrameBuffer.h"

namespace Turbo
{
    class RenderPass : public RefCounted
    {
    public:
        struct Config
        {
            WeakRef<FrameBuffer> TargetFrameBuffer;
            bool ClearOnLoad = true;

            const u32 SubPassCount = 1;
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
