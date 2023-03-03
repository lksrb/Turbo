#pragma once

#include "Turbo/Core/Common.h"

#include "Turbo/Renderer/RendererContext.h"

#include <vulkan/vulkan.h>

namespace Turbo::Vulkan
{
    static uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDevice device = RendererContext::GetPhysicalDevice();
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++)
        {
            if ((type_filter & (1 << i)) && (memProperties.memoryHeaps[i].flags & properties) == properties)
            {
                return i;
            }
        }


        TBO_ENGINE_ASSERT(false, "Could not find proper memory type.");
        return 0;
    }

    // From imgui_impl_vulkan.h
    static VkSurfaceFormatKHR SelectSurfaceFormat()
    {
        static VkColorSpaceKHR request_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

        static std::array<VkFormat, 4> request_formats
        { 
            VK_FORMAT_B8G8R8A8_UNORM, 
            VK_FORMAT_B8G8R8A8_SRGB, 
            VK_FORMAT_R8G8B8A8_SRGB, 
            VK_FORMAT_R8G8B8A8_UNORM, 
        };


        //TBO_ENGINE_ASSERT(request_formats.size() > 0);

        VkPhysicalDevice physical_device = RendererContext::GetPhysicalDevice();
        VkSurfaceKHR surface = RendererContext::GetSurface();

        // Per Spec Format and View Format are expected to be the same unless VK_IMAGE_CREATE_MUTABLE_BIT was set at image creation
        // Assuming that the default behavior is without setting this bit, there is no need for separate Swapchain image and image view format
        // Additionally several new color spaces were introduced with Vulkan Spec v1.0.40,
        // hence we must make sure that a format with the mostly available color space, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, is found and used.
        uint32_t avail_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &avail_count, nullptr);
        std::vector<VkSurfaceFormatKHR> avail_format;
        avail_format.resize((size_t)avail_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &avail_count, avail_format.data());

        // First check if only one format, VK_FORMAT_UNDEFINED, is available, which would imply that any format is available
        if (avail_count == 1)
        {
            if (avail_format[0].format == VK_FORMAT_UNDEFINED)
            {
                VkSurfaceFormatKHR ret;
                ret.format = request_formats[0];
                ret.colorSpace = request_color_space;
                return ret;
            }
            else
            {
                // No point in searching another format
                return avail_format[0];
            }
        }
        else
        {
            // Request several formats, the first found will be used
            for (size_t request_i = 0; request_i < request_formats.size(); request_i++)
                for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
                    if (avail_format[avail_i].format == request_formats[request_i] && avail_format[avail_i].colorSpace == request_color_space)
                        return avail_format[avail_i];

            // If none of the requested image formats could be found, use the first available
            return avail_format[0];
        }
    }
}
