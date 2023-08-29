#include "tbopch.h"
#include "RenderCommandBuffer.h"

#include "Turbo/Platform/Vulkan/VulkanRenderCommandBuffer.h"

namespace Turbo {

    Ref<RenderCommandBuffer> RenderCommandBuffer::Create()
    {
        return Ref<VulkanRenderCommandBuffer>::Create();
    }
}
