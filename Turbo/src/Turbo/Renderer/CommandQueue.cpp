#include "tbopch.h"
#include "CommandQueue.h"

namespace Turbo
{
    CommandQueue::CommandQueue(u64 size)
    {
        m_Buffer = m_BufferPointer = new u8[size];
        memset(m_BufferPointer, 0, size);
    }

    CommandQueue::~CommandQueue()
    {
        delete[] m_Buffer;
        m_Buffer = nullptr;
    }

    void* CommandQueue::Allocate(RenderCommandFn func, size_t size)
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

    void CommandQueue::Execute()
    {
        u8* buffer = m_Buffer;

        for (u32 i = 0; i < m_CommandCount; ++i)
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
