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
        TBO_ENGINE_ASSERT(config.ColorAttachment.Image);

        return Ref<VulkanFrameBuffer>::Create(config);
    }

}
