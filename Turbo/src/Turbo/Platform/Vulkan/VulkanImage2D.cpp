#include "tbopch.h"
#include "VulkanImage2D.h"

#include "Turbo/Renderer/RendererContext.h"

namespace Turbo
{
    namespace Utils
    {
        static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
        {
            VkDevice device = RendererContext::GetDevice();

            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(RendererContext::GetPhysicalDevice(), &memProperties);
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

    VulkanImage2D::VulkanImage2D(const Image2D::Config& config) 
        : Image2D(config)
    {
    }

    VulkanImage2D::~VulkanImage2D()
    {
    }

    void VulkanImage2D::Invalidate(u32 width, u32 height)
    {
        SetExtent(width, height);

        VkDevice device = RendererContext::GetDevice();

        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.pNext = nullptr;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = m_Width;
        imageCreateInfo.extent.height = m_Height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.usage = m_Config.Usage;
        imageCreateInfo.format = static_cast<VkFormat>(m_Config.Format);
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.tiling = static_cast<VkImageTiling>(m_Config.Tiling);
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkFormatProperties validProperties;
        vkGetPhysicalDeviceFormatProperties(RendererContext::GetPhysicalDevice(), static_cast<VkFormat>(m_Config.Format), &validProperties);

        TBO_VK_ASSERT(vkCreateImage(device, &imageCreateInfo, nullptr, &m_Image));

        // Image memory
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, m_Image, &memRequirements);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;

        allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits, m_Config.MemoryStorage);

        TBO_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_ImageMemory));
        TBO_VK_ASSERT(vkBindImageMemory(device, m_Image, m_ImageMemory, 0));

        // Image view
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_Image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = static_cast<VkFormat>(m_Config.Format);
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        VkImageSubresourceRange image_range = { m_Config.Aspect, 0, 1, 0, 1 };
        createInfo.subresourceRange = image_range;
        TBO_VK_ASSERT(vkCreateImageView(device, &createInfo, nullptr, &m_ImageView));

        // Sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = static_cast<VkFilter>(m_Config.Filter);
        samplerInfo.minFilter = static_cast<VkFilter>(m_Config.Filter);
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        /*samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = device->GetPhysicalDevice().GetSupportDetails().Properties.limits.maxSamplerAnisotropy;*/
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        TBO_VK_ASSERT(vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler));

        // Add it to deletion queue 
        RendererContext::SubmitResourceFree([device, sampler = m_Sampler, image = m_Image, imageMemory = m_ImageMemory, imageView = m_ImageView]()
        {
            vkDestroySampler(device, sampler, nullptr);
            vkDestroyImage(device, image, nullptr);
            vkFreeMemory(device, imageMemory, nullptr);
            vkDestroyImageView(device, imageView, nullptr);
        });
    }
}
