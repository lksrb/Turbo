#include "tbopch.h"
#include "RendererBuffer.h"

#include "Turbo/Platform/Vulkan/VulkanBuffer.h"

namespace Turbo
{
    RendererBuffer::RendererBuffer(const RendererBuffer::Config& config)
        : m_Config(config), m_Data(nullptr)
    {
    }

    RendererBuffer::~RendererBuffer()
    {
    }

    Ref<RendererBuffer> RendererBuffer::Create(const RendererBuffer::Config& config)
    {
        TBO_ENGINE_ASSERT(config.Size != 0);
        return Ref<VulkanBuffer>::Create(config);
    }


}
