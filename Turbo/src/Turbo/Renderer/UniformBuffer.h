#pragma once

#include "RendererBuffer.h"

#include "Turbo/Core/Common.h"

namespace Turbo
{
    class UniformBuffer
    {
    public:
        struct Config
        {
            u32 Binding;
            u32 Set;
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
        struct Config
        {
            u32 Binding;
            u32 Set;
            size_t Size;
        };

        UniformBufferSet(const UniformBufferSet::Config& config);
        ~UniformBufferSet();

        void SetData(const void* data);

        static Ref<UniformBufferSet> Create(const UniformBufferSet::Config& config);
    private:
        std::vector<Ref<UniformBuffer>> m_Buffers;

        UniformBufferSet::Config m_Config;
    };

}
