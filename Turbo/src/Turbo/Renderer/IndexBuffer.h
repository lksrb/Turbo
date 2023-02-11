#pragma once

#include "Turbo/Core/Common.h"

namespace Turbo
{
    class IndexBuffer
    {
    public:
        struct Config
        {
            size_t Size;
            u32* Indices;
        };

        static Ref<IndexBuffer> Create(const IndexBuffer::Config& config);
        virtual ~IndexBuffer();
    protected:
        IndexBuffer(const IndexBuffer::Config& config);

        IndexBuffer::Config m_Config;
    };
}
