#pragma once

#include "RendererBuffer.h"
#include "Fly.h"

#include "Turbo/Core/Common.h"
#include <map>

namespace Turbo
{
    class UniformBuffer : public RefCounted
    {
    public:
        struct Config
        {
            u32 Set;
            u32 Binding;
            u64 Size;
        };

        UniformBuffer(const UniformBuffer::Config& config);
        virtual ~UniformBuffer();

        static Ref<UniformBuffer> Create(const UniformBuffer::Config& config);

        virtual void Invalidate() = 0;
        virtual void SetData(const void* data) = 0;
    protected:
        void* m_Data = nullptr;

        UniformBuffer::Config m_Config;
    };

    class UniformBufferSet : public RefCounted
    {
    public:
        UniformBufferSet();
        ~UniformBufferSet();

        void Create(u32 set, u32 binding, u64 data_size);

        void SetData(u32 set, u32 binding, const void* data);

        Ref<UniformBuffer> Get(u32 set, u32 binding) const;

        static Ref<UniformBufferSet> Create();
    private:
        std::map<u32, std::map<u32, Fly<Ref<UniformBuffer>>>> m_UniformBufferMap;
    };

}
