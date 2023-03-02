#include "tbopch.h"
#include "FrameBuffer.h"

#include "Turbo/Platform/Vulkan/VulkanFrameBuffer.h"

namespace Turbo
{
    FrameBuffer::FrameBuffer(const FrameBuffer::Config& config)
        : m_Config(config)
    {
    }

    FrameBuffer::~FrameBuffer()
    {
    }

    Ref<FrameBuffer> FrameBuffer::Create(const FrameBuffer::Config& config)
    {
        return Ref<VulkanFrameBuffer>::Create(config);
    }

}
