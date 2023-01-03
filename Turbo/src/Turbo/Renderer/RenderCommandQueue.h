#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    using RenderCommandFn = void(*)(void*);

    class RenderCommandQueue
    {
    public:
        RenderCommandQueue();
        ~RenderCommandQueue();

        RenderCommandQueue(const RenderCommandQueue&) = delete;

        void* Allocate(RenderCommandFn func, size_t size);
        void Execute();
    private:
        u32 m_CommandCount;
        u8* m_Buffer;
        u8* m_BufferPointer;
    };
}
