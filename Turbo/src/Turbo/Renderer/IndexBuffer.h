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
            const u32* Indices;

            Config(u32* indices, size_t size) : Indices(indices), Size(size) {}
            Config(const std::vector<u32>& indices) : Indices(indices.data()), Size(indices.size() * sizeof(u32)) {}
        };

        static Ref<IndexBuffer> Create(const IndexBuffer::Config& config);
        virtual ~IndexBuffer();
    protected:
        IndexBuffer(const IndexBuffer::Config& config);

        IndexBuffer::Config m_Config;
    };
}
