#pragma once

#include "Turbo/Core/Common.h"

#include "Turbo/Renderer/RendererContext.h"

#include <vulkan/vulkan.h>

namespace Turbo::Vulkan
{
    static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDevice device = RendererContext::GetPhysicalDevice();
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryHeaps[i].flags & properties) == properties)
            {
                return i;
            }
        }


        TBO_ENGINE_ASSERT(false, "Could not find proper memory type.");
        return 0;
    }
}
