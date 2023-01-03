#include "tbopch.h"
#include "VertexBuffer.h"

#include "Turbo/Platform/Vulkan/VulkanVertexBuffer.h"

namespace Turbo
{
    VertexBuffer::VertexBuffer(const VertexBuffer::Config& config)
        : m_Config(config)
    {
    }

    VertexBuffer* VertexBuffer::Create(const VertexBuffer::Config& config)
    {
        return new VulkanVertexBuffer(config);
    }

    VertexBuffer::~VertexBuffer()
    {

    }

}
