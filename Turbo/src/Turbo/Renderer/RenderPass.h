#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Renderer/Image2D.h"

namespace Turbo
{
    class RenderPass
    {
    public:
        enum ImageLayout : u32
        {
            ImageLayout_ReadOnly_Optimal = 1000314000,
            ImageLayout_PresentSrcKhr = 1000001002,
            ImageLayout_Shader_ReadOnly_Optimal = 5
        };

        struct Config
        {
            Ref<Image2D> DepthAttachment;
            ImageLayout DestinationLayout;
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
