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

    SwapChain* SwapChain::Create()
    {
        return new VulkanSwapChain();
    }
}
