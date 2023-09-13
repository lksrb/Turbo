#include "tbopch.h"
#include "SwapChain.h"

#include "Turbo/Platform/Vulkan/VulkanSwapChain.h"

namespace Turbo {

    Owned<SwapChain> SwapChain::Create()
    {
        return Owned<VulkanSwapChain>::Create();
    }
}
