#pragma once

#include "Turbo/Renderer/RenderCommandBuffer.h"

namespace Turbo
{
    enum class VertexBufferType : u32 { None = 0, Static, Dynamic };

    class VertexBuffer
    {
    public:

        VertexBuffer() = default;
        virtual ~VertexBuffer() = default;

        static Ref<VertexBuffer> Create(const void* vertices, u64 size); // Static buffer
        static Ref<VertexBuffer> Create(u64 size);

        VertexBufferType GetType() const { return m_Type; }

        virtual void SetData(Ref<RenderCommandBuffer> commandBuffer, const void* vertices, u64 size) = 0;
    protected:
        VertexBufferType m_Type = VertexBufferType::None;
        u64 m_Size = 0;
    };
}
