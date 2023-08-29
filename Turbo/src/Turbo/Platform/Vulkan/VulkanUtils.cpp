#include "tbopch.h"
#include "VulkanUtils.h"

#include "VulkanContext.h"

namespace Turbo {

    u32 Vulkan::FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDevice physicalDevice = VulkanContext::Get()->GetDevice().GetPhysicalDevice().GetSelectedDevice();
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
        for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        for (u32 i = 0; i < memProperties.memoryHeapCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryHeaps[i].flags & properties) == properties)
            {
                return i;
            }
        }


        TBO_ENGINE_ASSERT(false, "Could not find proper memory type.");
        return 0;
    }

    VkSurfaceFormatKHR Vulkan::SelectSurfaceFormat()
    {
        static VkColorSpaceKHR request_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        static std::array<VkFormat, 4> request_formats = {
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_FORMAT_B8G8R8A8_SRGB,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_FORMAT_R8G8B8A8_UNORM,
        };

        VkPhysicalDevice physicalDevice = VulkanContext::Get()->GetDevice().GetPhysicalDevice().GetSelectedDevice();
        VkSurfaceKHR surface = VulkanContext::Get()->GetSurface();

        // Per Spec Format and View Format are expected to be the same unless VK_IMAGE_CREATE_MUTABLE_BIT was set at image creation
        // Assuming that the default behavior is without setting this bit, there is no need for separate Swapchain image and image view format
        // Additionally several new color spaces were introduced with Vulkan Spec v1.0.40,
        // hence we must make sure that a format with the mostly available color space, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, is found and used.
        u32 availCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availCount, nullptr);
        std::vector<VkSurfaceFormatKHR> availFormats;
        availFormats.resize((u64)availCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availCount, availFormats.data());

        // First check if only one format, VK_FORMAT_UNDEFINED, is available, which would imply that any format is available
        if (availCount == 1)
        {
            if (availFormats[0].format == VK_FORMAT_UNDEFINED)
            {
                VkSurfaceFormatKHR ret;
                ret.format = request_formats[0];
                ret.colorSpace = request_color_space;
                return ret;
            }
            else
            {
                // No point in searching another format
                return availFormats[0];
            }
        }
        else
        {
            // Request several formats, the first found will be used
            for (u64 request_i = 0; request_i < request_formats.size(); request_i++)
                for (u32 avail_i = 0; avail_i < availCount; avail_i++)
                    if (availFormats[avail_i].format == request_formats[request_i] && availFormats[avail_i].colorSpace == request_color_space)
                        return availFormats[avail_i];

            // If none of the requested image formats could be found, use the first available
            return availFormats[0];
        }
    }

    u32 Vulkan::BytesPerPixelFromFormat(ImageFormat format)
    {
        switch (format)
        {
            case ImageFormat_RGB_Unorm:
            case ImageFormat_RGB_SRGB:
                return 3;
            case ImageFormat_RGBA_SRGB:
            case ImageFormat_RGBA_Unorm:
            case ImageFormat_BGRA_Unorm:
            case ImageFormat_BGRA_SRGB:
            case ImageFormat_R_SInt:
                return 4;
            case ImageFormat_RGBA32F:
                return 4 * 4;
        }

        TBO_ENGINE_ASSERT(false, "Invalid format!");
        return 0;
    }
}

