#pragma once

#include "RendererBuffer.h"

#include "Turbo/Core/Common.h"
#include <map>

namespace Turbo
{
    class UniformBuffer
    {
    public:
        struct Config
        {
            u32 Set;
            u32 Binding;
            size_t Size;
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

    class UniformBufferSet
    {
    public:
        UniformBufferSet();
        ~UniformBufferSet();

        void Create(u32 set, u32 binding, size_t data_size);

        void SetData(u32 set, u32 binding, const void* data);

        Ref<UniformBuffer> Get(u32 set, u32 binding) const;

        static Ref<UniformBufferSet> Create();
    private:
        std::map<u32, std::map<u32, std::vector<Ref<UniformBuffer>>>> m_UniformBufferMap;
    };

}
