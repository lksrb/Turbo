#include "tbopch.h"
#include "RendererContext.h"

#include "Turbo/Platform/Vulkan/VulkanContext.h"

namespace Turbo {

    Owned<RendererContext> RendererContext::Create()
    {
        return Owned<VulkanContext>::Create();
    }

}
