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

    Ref<CommandBuffer> CommandBuffer::Create(CommandBufferLevel type)
    {
        return Ref<VulkanCommandBuffer>::Create(type);
    }


}
