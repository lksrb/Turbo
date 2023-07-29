#include "tbopch.h"
#include "VulkanTextureCube.h"

#include "VulkanBuffer.h"
#include "VulkanUtils.h"

#include "turbo/Renderer/RendererContext.h"

#include <stb_image.h>

namespace Turbo
{

    VulkanTextureCube::VulkanTextureCube(const TextureCube::Config& config)
        : TextureCube(config)
    {
        Load();
    }

    VulkanTextureCube::~VulkanTextureCube()
    {

    }

    void VulkanTextureCube::Load()
    {
        // TODO: KTX library to load texture cubes efficiently

#if 1
        static const char* facePaths[6] =
        {
            "SandboxProject/Assets/Environment/Skybox/right.jpg", // +X
            "SandboxProject/Assets/Environment/Skybox/left.jpg", // -X
            "SandboxProject/Assets/Environment/Skybox/top.jpg", // +Y
            "SandboxProject/Assets/Environment/Skybox/bottom.jpg", // -Y
            "SandboxProject/Assets/Environment/Skybox/front.jpg", // +Z
            "SandboxProject/Assets/Environment/Skybox/back.jpg" // -Z
        };

        std::array<u8*, 6> skyboxFacePixels = {};
        for (u64 i = 0; i < skyboxFacePixels.size(); i++)
        {
            int width, height, channels;
            skyboxFacePixels[i] = stbi_load(facePaths[i], &width, &height, &channels, STBI_rgb_alpha);
            m_Config.Width = width;
            m_Config.Height = height;
        }
#endif
        VkDevice device = RendererContext::GetDevice();

        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent = { m_Config.Width , m_Config.Height, 1 };
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageCreateInfo.arrayLayers = 6; // 6 faces
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        TBO_VK_ASSERT(vkCreateImage(device, &imageCreateInfo, nullptr, &m_Image));

        // Image memory
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, m_Image, &memRequirements);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = Vulkan::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        TBO_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_ImageMemory));
        TBO_VK_ASSERT(vkBindImageMemory(device, m_Image, m_ImageMemory, 0));

        RendererBuffer::Config config = {};
        config.Size = m_Config.Width * m_Config.Height * Vulkan::BytesPerPixelFromFormat((ImageFormat)VK_FORMAT_R8G8B8A8_UNORM) * 6; // Per layer
        config.UsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        config.MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        config.Temporary = true;
        VulkanBuffer stagingBuffer(config);

        u64 layerSize = config.Size / 6;

        u8* data = (u8*)stagingBuffer.GetData();
        for (u64 i = 0; i < skyboxFacePixels.size(); i++)
        {
            memcpy(data + layerSize * i, skyboxFacePixels[i], layerSize);
        }

        VkCommandBuffer commandBuffer = RendererContext::CreateCommandBuffer();

        VkImageSubresourceRange subResourceRange = {};
        subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subResourceRange.baseMipLevel = 0;
        subResourceRange.levelCount = 1;
        subResourceRange.layerCount = 6;
        {
            // First execute image memory barrier so we can access the image
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_Image;
            barrier.subresourceRange = subResourceRange;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        }

        std::array<VkBufferImageCopy, 6> bufferCopyRegions = {};
        for (u32 face = 0; face < 6; face++)
        {
            auto& bufferCopyRegion = bufferCopyRegions[face];
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = 0;
            bufferCopyRegion.imageSubresource.baseArrayLayer = face;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = m_Config.Width;
            bufferCopyRegion.imageExtent.height = m_Config.Height;
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = face * layerSize;
        }

        // Copy data
        {
            // Copy mip levels from staging buffer
            vkCmdCopyBufferToImage(
                commandBuffer,
                stagingBuffer.GetHandle(),
                m_Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                (u32)bufferCopyRegions.size(),
                bufferCopyRegions.data());
        }

        // Then execute another image memory barrier to return it to the previous state
        {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_Image;
            barrier.subresourceRange = subResourceRange;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        }

        RendererContext::FlushCommandBuffer(commandBuffer);

        // Image view
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_Image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange = subResourceRange;
        TBO_VK_ASSERT(vkCreateImageView(device, &createInfo, nullptr, &m_ImageView));

        // Create sampler
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        // Anisotropic filtering
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;

        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        TBO_VK_ASSERT(vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler));

        // Add it to deletion queue 
        RendererContext::SubmitResourceFree([sampler = m_Sampler, image = m_Image, imageMemory = m_ImageMemory, imageView = m_ImageView]()
        {
            VkDevice device = RendererContext::GetDevice();
            vkDestroySampler(device, sampler, nullptr);
            vkDestroyImage(device, image, nullptr);
            vkFreeMemory(device, imageMemory, nullptr);
            vkDestroyImageView(device, imageView, nullptr);
        });

        for (auto pixels : skyboxFacePixels)
        {
            stbi_image_free(pixels);
        }
    }

}
