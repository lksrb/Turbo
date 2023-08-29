#include "tbopch.h"
#include "RendererContext.h"

#include "Turbo/Platform/Vulkan/VulkanContext.h"

namespace Turbo {

    Ref<RendererContext> RendererContext::Create()
    {
        return Ref<VulkanContext>::Create();
    }

}
