#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#include "Turbo/Renderer/Image2D.h"

#include <vulkan/vulkan.h>

namespace Turbo::Vulkan
{
    u32 FindMemoryType(u32 type_filter, VkMemoryPropertyFlags properties);

    // From imgui_impl_vulkan.h
    VkSurfaceFormatKHR SelectSurfaceFormat();

    u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);

    u32 BytesPerPixelFromFormat(ImageFormat format);
}
