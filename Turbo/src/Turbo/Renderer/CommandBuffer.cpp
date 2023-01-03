#include "tbopch.h"
#include "CommandBuffer.h"

#include "Turbo/Platform/Vulkan/VulkanCommandBuffer.h"

namespace Turbo
{

    CommandBuffer::CommandBuffer(CommandBufferLevel type)
        : m_Type(type)
    {
    }

    CommandBuffer::~CommandBuffer()
    {
    }

    CommandBuffer* CommandBuffer::Create(CommandBufferLevel type)
    {
        return new VulkanCommandBuffer(type);
    }


}
