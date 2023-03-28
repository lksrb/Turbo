#include "tbopch.h"
#include "RenderCommandBuffer.h"

#include "Turbo/Platform/Vulkan/VulkanRenderCommandBuffer.h"

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
        return Ref<VulkanRenderCommandBuffer>::Create();
    }
}
