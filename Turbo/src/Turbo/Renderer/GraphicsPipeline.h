#pragma once

#include "Turbo/Core/Common.h"

#include "Shader.h"
#include "RenderPass.h"
#include "VertexBufferLayout.h"

namespace Turbo 
{
    enum class PrimitiveTopology : u32
    {
        Triangle = 0,
        Line
    };

    class GraphicsPipeline : public RefCounted
    {
    public:
        struct Config 
        {
            Ref<RenderPass> Renderpass;
            Ref<Shader> Shader;
            PrimitiveTopology Topology;
            VertexBufferLayout Layout;
            VertexBufferLayout InstanceLayout;
            bool DepthTesting;

            // Temporary
            u32 SubpassIndex = 0;
        };

        static Ref<GraphicsPipeline> Create(const GraphicsPipeline::Config& config);
        virtual ~GraphicsPipeline();

        const GraphicsPipeline::Config& GetConfig() const { return m_Config; }

        virtual void Invalidate() = 0;
    protected:
        GraphicsPipeline(const GraphicsPipeline::Config& config);

        GraphicsPipeline::Config m_Config;
    };
}
