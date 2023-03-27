#include "tbopch.h"
#include "RenderCommandBuffer.h"

#include "Turbo/Platform/Vulkan/VulkanCommandBuffer.h"

namespace Turbo
{
    RenderCommandBuffer::RenderCommandBuffer()
    {
    }

    RenderCommandBuffer::~RenderCommandBuffer()
    {
    }

    Ref<RenderCommandBuffer> RenderCommandBuffer::Create()
    {
        return Ref<VulkanCommandBuffer>::Create();
    }
}
