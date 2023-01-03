#pragma once

#include "Turbo/Core/Common.h"

namespace Turbo
{
    // Renderer-Api agnostic buffer wrapper
    class RendererBuffer
    {
    public:
        struct Config
        {
            size_t Size;
            u32 UsageFlags;     // VkBufferUsageFlags
            u32 MemoryFlags;    // VkMemoryPropertyFlags
            bool Temporary;
        };
        static RendererBuffer* Create(const RendererBuffer::Config& config);
        virtual ~RendererBuffer();

        virtual void SetData(const void* data) = 0;

        size_t GetSize() const { return m_Config.Size; }
        void* GetData() const { return m_Data; }
    protected:
        RendererBuffer(const RendererBuffer::Config& config);

        void* m_Data;
        RendererBuffer::Config m_Config;
    };
}
