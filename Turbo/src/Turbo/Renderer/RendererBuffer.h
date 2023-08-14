#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    enum BufferUsageFlags_ : u32
    {
        BufferUsageFlags_Transfer_Dst = 1 << 1
    };

    enum MemoryPropertyFlags_ : u32
    {
        MemoryPropertyFlags_HostVisible = 1 << 1,
        MemoryPropertyFlags_HostCoherent = 1 << 2,
        MemoryPropertyFlags_HostCached = 1 << 3

    };

    using BufferUsageFlags = u32;
    using MemoryPropertyFlags = u32;

    class RendererBuffer : public RefCounted
    {
    public:
        struct Config
        {
            u64 Size;
            BufferUsageFlags UsageFlags;     // VkBufferUsageFlags
            MemoryPropertyFlags MemoryFlags;    // VkMemoryPropertyFlags
            bool Temporary;
            bool SetDefaultValue = false;
            i32 DefaultValue;
        };
        static Ref<RendererBuffer> Create(const RendererBuffer::Config& config);
        virtual ~RendererBuffer();

        virtual void SetData(const void* data) = 0;

        size_t Size() const { return m_Config.Size; }
        const void* GetData() const { return m_Data; }
    protected:
        RendererBuffer(const RendererBuffer::Config& config);

        void* m_Data = nullptr;
        RendererBuffer::Config m_Config;
    };
}
