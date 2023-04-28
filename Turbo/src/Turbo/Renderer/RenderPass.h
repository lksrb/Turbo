#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Renderer/Image2D.h"

namespace Turbo
{
    class RenderPass
    {
    public:
        struct Config
        {
            ImageLayout DestinationLayout;
            bool EnableDepthTesting = false;
            bool ClearOnLoad = true; // Do not change
        };

        static Ref<RenderPass> Create(const RenderPass::Config& config = {});
        virtual ~RenderPass();

        const RenderPass::Config& GetConfig() const { return m_Config; }

        virtual void Invalidate() = 0;
    protected:
        RenderPass(const RenderPass::Config& config);

        RenderPass::Config m_Config;
    };
}
