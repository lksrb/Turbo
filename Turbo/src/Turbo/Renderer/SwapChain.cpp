#include "tbopch.h"
#include "SwapChain.h"

#include "Turbo/Platform/Vulkan/VulkanSwapChain.h"

namespace Turbo
{
    SwapChain::SwapChain()
    {
    }

    SwapChain::~SwapChain()
    {
    }

    Ref<SwapChain> SwapChain::Create()
    {
        return Ref<VulkanSwapChain>::Create();
    }
}
