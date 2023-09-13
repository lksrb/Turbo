#pragma once

#include "Shader.h"
#include "RenderPass.h"
#include "VertexBufferLayout.h"

namespace Turbo {

    enum class PrimitiveTopology : u32
    {
        Triangle = 0,
        Line
    };

    class Pipeline : public RefCounted
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

        static Ref<Pipeline> Create(const Pipeline::Config& config);
        virtual ~Pipeline();

        const Pipeline::Config& GetConfig() const { return m_Config; }

        virtual void Invalidate() = 0;
    protected:
        Pipeline(const Pipeline::Config& config);

        Pipeline::Config m_Config;
    };
}
