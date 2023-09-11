#pragma once

namespace Turbo
{
    class IndexBuffer : public RefCounted
    {
    public:
        virtual ~IndexBuffer() = default;

        static Ref<IndexBuffer> Create(const std::vector<u32>& indices);
        static Ref<IndexBuffer> Create(const u32* indices, u32 count);
    protected:
        u64 m_Size = 0;
    };
}
