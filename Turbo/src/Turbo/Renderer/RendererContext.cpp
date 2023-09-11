#include "tbopch.h"
#include "RendererContext.h"

#include "Turbo/Platform/Vulkan/VulkanContext.h"

namespace Turbo {

    RendererContext* RendererContext::Create()
    {
        return new VulkanContext();
    }

}
