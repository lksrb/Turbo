#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    enum class CommandBufferLevel : u32
    {
        Primary = 0,
        Secondary
    };

    // Set of command buffers
    class CommandBuffer
    {
    public:
        static Ref<CommandBuffer> Create(CommandBufferLevel type);
        virtual ~CommandBuffer();

        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void Submit() = 0;
    protected:
        CommandBuffer(CommandBufferLevel type);

        CommandBufferLevel m_Type;
    };
}
