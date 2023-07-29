#include "tbopch.h"
#include "VulkanImage2D.h"

#include "Turbo/Renderer/RendererContext.h"

#include "Turbo/Platform/Vulkan/VulkanUtils.h"

namespace Turbo
{
    // For debug purposes
    static std::vector<VulkanImage2D*> s_LiveInstances;

    VulkanImage2D::VulkanImage2D(const Image2D::Config& config) 
        : Image2D(config)
    {
        s_LiveInstances.emplace_back(this);
    }

    VulkanImage2D::~VulkanImage2D()
    {
        // Add it to deletion queue 
        RendererContext::SubmitResourceFree([sampler = m_Sampler, image = m_Image, imageMemory = m_ImageMemory, imageView = m_ImageView]()
        {
            VkDevice device = RendererContext::GetDevice();
            vkDestroySampler(device, sampler, nullptr);
            vkDestroyImage(device, image, nullptr);
            vkFreeMemory(device, imageMemory, nullptr);
            vkDestroyImageView(device, imageView, nullptr);
        });

        auto it = std::find(s_LiveInstances.begin(), s_LiveInstances.end(), this);
        s_LiveInstances.erase(it);
    }

    void VulkanImage2D::Invalidate(u32 width, u32 height)
    {
        SetExtent(width, height);

        VkDevice device = RendererContext::GetDevice();

        if (m_Image)
        {
            RendererContext::SubmitRuntimeResourceFree([device, sampler = m_Sampler, image = m_Image, imageMemory = m_ImageMemory, imageView = m_ImageView]()
            {
                vkDestroySampler(device, sampler, nullptr);
                vkDestroyImage(device, image, nullptr);
                vkFreeMemory(device, imageMemory, nullptr);
                vkDestroyImageView(device, imageView, nullptr);
            });
        }

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
        TBO_VK_ASSERT(vkCreateImage(device, &imageCreateInfo, nullptr, &m_Image));

        // Image memory
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, m_Image, &memRequirements);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;

        allocInfo.memoryTypeIndex = Vulkan::FindMemoryType(memRequirements.memoryTypeBits, m_Config.MemoryStorage);

        TBO_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_ImageMemory));
        TBO_VK_ASSERT(vkBindImageMemory(device, m_Image, m_ImageMemory, 0));

        // Image view
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_Image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = static_cast<VkFormat>(m_Config.Format);
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange = { m_Config.Aspect, 0, 1, 0, 1 };
        TBO_VK_ASSERT(vkCreateImageView(device, &createInfo, nullptr, &m_ImageView));

        // Sampler
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = static_cast<VkFilter>(m_Config.Filter);
        samplerInfo.minFilter = static_cast<VkFilter>(m_Config.Filter);
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        TBO_VK_ASSERT(vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler));
    }
}
