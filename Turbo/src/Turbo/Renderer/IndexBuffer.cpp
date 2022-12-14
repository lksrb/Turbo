#include "tbopch.h"
#include "IndexBuffer.h"

#include "Turbo/Platform/Vulkan/VulkanIndexBuffer.h"

namespace Turbo
{
    IndexBuffer::IndexBuffer(const IndexBuffer::Config& config)
        : m_Config(config)
    {
    }

    IndexBuffer::~IndexBuffer()
    {
    }

    IndexBuffer* IndexBuffer::Create(const IndexBuffer::Config& config)
    {
        return new VulkanIndexBuffer(config);
    }

}
