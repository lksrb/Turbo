#pragma once

namespace Turbo
{
    class VertexBuffer
    {
    public:
        struct Config
        {
            size_t Size;
        };
        
        static Ref<VertexBuffer> Create(const VertexBuffer::Config& config);
        virtual ~VertexBuffer();

        virtual void SetData(void* data, size_t size) = 0;
    protected:
        VertexBuffer(const VertexBuffer::Config& config);

        VertexBuffer::Config m_Config;
    };
}
