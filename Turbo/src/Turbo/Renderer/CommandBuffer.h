#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    enum class CommandBufferLevel : u32
    {
        Primary = 0,
        Secondary
    };

    class CommandBuffer
    {
    public:
        static CommandBuffer* Create(CommandBufferLevel type);
        virtual ~CommandBuffer();

        virtual void Begin() = 0;
        virtual void End() = 0;
    protected:
        CommandBuffer(CommandBufferLevel type);

        CommandBufferLevel m_Type;
    };
}
