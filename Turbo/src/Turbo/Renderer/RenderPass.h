#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Renderer/Framebuffer.h"

namespace Turbo
{
    class RenderPass
    {
    public:
        struct Config
        {
        };

        static RenderPass* Create(const RenderPass::Config& config);
        virtual ~RenderPass();

        const RenderPass::Config& GetConfig() const { return m_Config; }

        virtual void Invalidate() = 0;
    protected:
        RenderPass(const RenderPass::Config& config);

        RenderPass::Config m_Config;
    };
}
