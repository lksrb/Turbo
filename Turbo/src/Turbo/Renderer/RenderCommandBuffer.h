#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    // Set of command buffers
    class RenderCommandBuffer : public RefCounted
    {
    public:
        RenderCommandBuffer();
        virtual ~RenderCommandBuffer();

        static Ref<RenderCommandBuffer> Create();

        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void Submit() = 0;
    };
}
