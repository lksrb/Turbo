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
        
        static VertexBuffer* Create(const VertexBuffer::Config& config);
        virtual ~VertexBuffer();

        virtual void SetData(void* data, u32 size) = 0;
    protected:
        VertexBuffer(const VertexBuffer::Config& config);

        VertexBuffer::Config m_Config;
    };
}
