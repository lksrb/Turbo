#include "tbopch.h"
#include "RenderCommandQueue.h"

namespace Turbo
{
    RenderCommandQueue::RenderCommandQueue()
    {
        memset(this, 0, sizeof(*this));
        // Allocate 10 MB
        m_Buffer = m_BufferPointer = new u8[1 * 1024 * 1024];
        memset(m_BufferPointer, 0, 1 * 1024 * 1024);
    }

    RenderCommandQueue::~RenderCommandQueue()
    {
        delete[] m_Buffer;
        m_Buffer = nullptr;
    }

    void* RenderCommandQueue::Allocate(RenderCommandFn func, size_t size)
    {
        // TODO: alignment
        *(RenderCommandFn*)m_BufferPointer = func;
        m_BufferPointer += sizeof(RenderCommandFn);

        *(u32*)m_BufferPointer = static_cast<u32>(size);
        m_BufferPointer += sizeof(u32);

        void* memory = m_BufferPointer;
        m_BufferPointer += size;

        ++m_CommandCount;

        return memory;
    }

    void RenderCommandQueue::Execute()
    {
        u8* buffer = m_Buffer;

        for (u32 i = 0; i < m_CommandCount; i++)
        {
            RenderCommandFn function = *(RenderCommandFn*)buffer;
            buffer += sizeof(RenderCommandFn);

            u32 size = *(u32*)buffer;
            buffer += sizeof(u32);
            function(buffer);
            buffer += size;
        }

        m_BufferPointer = m_Buffer;
        m_CommandCount = 0;
    }
}
