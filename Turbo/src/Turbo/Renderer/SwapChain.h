#pragma once

#include "Turbo/Core/Memory.h"
#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    class SwapChain
    {
    public:
        static Ref<SwapChain> Create();
        virtual ~SwapChain();

        virtual void NewFrame() = 0;
        virtual void SwapFrame() = 0;
        virtual void Resize(u32 width, u32 height) = 0;
    protected:
        SwapChain();
    };
}
