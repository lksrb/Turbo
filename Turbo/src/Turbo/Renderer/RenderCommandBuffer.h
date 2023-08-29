#pragma once

namespace Turbo
{
    // Set of command buffers
    class RenderCommandBuffer : public RefCounted
    {
    public:
        RenderCommandBuffer() = default;
        virtual ~RenderCommandBuffer() = default;

        static Ref<RenderCommandBuffer> Create();

        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void Submit() = 0;
    };
}
