#pragma once

#include "Turbo/Core/Common.h"

#include "Shader.h"
#include "RenderPass.h"

namespace Turbo 
{
    enum class PrimitiveTopology : u32
    {
        Triangle = 0,
        Line
    };

    class GraphicsPipeline
    {
    public:
        struct Config 
        {
            Ptr<RenderPass> Renderpass;
            Ptr<Shader> Shader;
            Ptr<FrameBuffer> TargetFramebuffer;
            PrimitiveTopology Topology;
            bool DepthTesting;
        };

        static GraphicsPipeline* Create(const GraphicsPipeline::Config& config);
        virtual ~GraphicsPipeline();

        virtual void Invalidate() = 0;
    protected:
        GraphicsPipeline(const GraphicsPipeline::Config& config);

        GraphicsPipeline::Config m_Config;
    };
}
