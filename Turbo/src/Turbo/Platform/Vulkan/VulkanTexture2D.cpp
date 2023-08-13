#include "tbopch.h"
#include "VulkanTexture2D.h"

#include "VulkanBuffer.h"
#include "VulkanUtils.h"

#include "Turbo/Renderer/RendererContext.h"


#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push, 0)
#include <stb_image.h>
#pragma warning(pop)

namespace Turbo
{
    VulkanTexture2D::VulkanTexture2D(u32 color)
    {
        // Create storage for the loaded texture
        CreateImage2D();

        // Transfer pixels
        SetData(&color);

        m_IsLoaded = true;
    }

    VulkanTexture2D::VulkanTexture2D(const Texture2D::Config& config)
        : Texture2D(config)
    {
        void* pixels = nullptr;
        if (!m_Config.Path.empty())
        {
            // Load picture from filepath
            int texWidth = 0, texHeight = 0, texChannel = 0;

            // Vertical flip of the texture

            stbi_set_flip_vertically_on_load(1);
            pixels = stbi_load(m_Config.Path.c_str(), &texWidth, &texHeight, &texChannel, STBI_rgb_alpha);
            stbi_set_flip_vertically_on_load(0);

            // Texture could not be loaded
            if (texWidth * texHeight <= 0)
            {
                TBO_ENGINE_ERROR("Texture cannot be loaded! (\"{0}\")", m_Config.Path.c_str());
                return;
            }

            // Update width and height
            m_Config.Width = texWidth;
            m_Config.Height = texHeight;
        }

        CreateImage2D();

        // Tranfer buffer to GPU memory
        if (pixels)
        {
            SetData(pixels);

            // Free pixels
            stbi_image_free(pixels);
        }

        m_IsLoaded = true;
    }

    VulkanTexture2D::VulkanTexture2D(const Texture2D::Config& config, const void* data, u64 size)
        : Texture2D(config)
    {
        // Decode png
        int width, height, channels;

        // Vertical flip of the texture
        stbi_set_flip_vertically_on_load(1);
        u8* pixels = stbi_load_from_memory((const stbi_uc*)data, (int)size, &width, &height, &channels, STBI_rgb_alpha);
        stbi_set_flip_vertically_on_load(0);

        m_Config.Width = width;
        m_Config.Height = height;

        CreateImage2D();

        if (pixels)
        {
            SetData(pixels);
            stbi_image_free(pixels);
        }

        m_IsLoaded = true;
    }

    VulkanTexture2D::~VulkanTexture2D()
    {
    }

    void VulkanTexture2D::Invalidate(u32 width, u32 height)
    {
        m_Config.Width = width;
        m_Config.Height = height;

        // TODO:

        TBO_ENGINE_ASSERT(false);
    }

    void VulkanTexture2D::SetData(const void* pixels)
    {
        RendererBuffer::Config config = {};
        config.Size = m_Config.Width * m_Config.Height * Vulkan::BytesPerPixelFromFormat(m_Config.Format);
        config.UsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        config.MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        config.Temporary = true;
        VulkanBuffer stagingBuffer(config);
        stagingBuffer.SetData(pixels);

        // Executing command buffers
        // Pipeline barrier, transition image layout
        VkCommandBuffer commandBuffer = RendererContext::CreateCommandBuffer();
        {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_TextureImage.As<VulkanImage2D>()->GetImage();
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        }

        // Copy buffer to image barrier
        {
            VkBufferImageCopy region = {};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = { m_Config.Width, m_Config.Height, 1 };

            vkCmdCopyBufferToImage(
                commandBuffer,
                stagingBuffer.GetHandle(),
                m_TextureImage.As<VulkanImage2D>()->GetImage(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
            );
        }

        // Pipeline barrier, transition image layout 2
        {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_TextureImage.As<VulkanImage2D>()->GetImage();
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
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
    }

    void VulkanTexture2D::CreateImage2D()
    {
        // Create storage for the loaded texture
        Image2D::Config imageConfig = {};
        imageConfig.Aspect = ImageAspect_Color;
        imageConfig.Usage = ImageUsage_Sampled | ImageUsage_Transfer_Destination;
        imageConfig.MemoryStorage = MemoryStorage_DeviceLocal;
        imageConfig.Tiling = ImageTiling_Optimal;
        imageConfig.Format = m_Config.Format;
        imageConfig.Filter = m_Config.Filter;
        imageConfig.DebugName = m_Config.Path;
        m_TextureImage = Image2D::Create(imageConfig);
        m_TextureImage->Invalidate(m_Config.Width, m_Config.Height);
    }

}
