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
        TBO_ENGINE_ASSERT(config.AttachmentsCount <= TBO_FRAMEBUFFER_MAX_ATTACHMENTS);
        return Ref<VulkanFrameBuffer>::Create(config);
    }

}
