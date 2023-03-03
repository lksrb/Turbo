#include "tbopch.h"
#include "VulkanTexture2D.h"

#include "Turbo/Renderer/RendererContext.h"

#include "Turbo/Platform/Vulkan/VulkanBuffer.h"

#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push, 0)
    #include <stb_image.h>
#pragma warning(pop)

namespace Turbo
{
    VulkanTexture2D::VulkanTexture2D(const Texture2D::Config& config)
        : Texture2D(config)
    {
        TBO_ENGINE_ASSERT(!m_Config.FilePath.Empty());

        // Load picture from filepath
        int texWidth = 0, texHeight = 0, texChannel = 0;

        // Vertical flip of the texture
        stbi_set_flip_vertically_on_load(1); 
        stbi_uc* pixels = stbi_load(m_Config.FilePath.CStr(), &texWidth, &texHeight, &texChannel, STBI_rgb_alpha);

        // Texture could not be loaded
        if (texWidth * texHeight <= 0)
        {
            TBO_ENGINE_ERROR("Texture cannot be loaded! (\"{0}\")", config.FilePath.CStr());
            stbi_set_flip_vertically_on_load(0);
            return;
        }

        m_IsLoaded = true;

        // Create storage for the loaded texture
        Image2D::Config imageConfig = {};
        imageConfig.Aspect = Image2D::AspectFlags_Color;
        imageConfig.Usage = Image2D::ImageUsageFlags_Sampled | Image2D::ImageUsageFlags_Transfer_Destination;
        imageConfig.Storage = Image2D::MemoryPropertyFlags_DeviceLocal;
        imageConfig.ImageTiling = Image2D::ImageTiling_Optimal;
        imageConfig.ImageFormat = Image2D::Format_RGBA8_SRGB;
        m_TextureImage = Image2D::Create(imageConfig);
        m_TextureImage->Invalidate(texWidth, texHeight);

        // Update width and height
        m_Width = texWidth;
        m_Height = texHeight;

        // Tranfer buffer to GPU memory
        Transfer(pixels, m_Width * m_Height * 4);

        // Free pixels
        stbi_image_free(pixels);

        stbi_set_flip_vertically_on_load(0);
    }

    VulkanTexture2D::VulkanTexture2D(u32 color)
        : Texture2D(color)
    {
        // Create storage for the loaded texture
        Image2D::Config imageConfig = {};
        imageConfig.Aspect = Image2D::AspectFlags_Color;
        imageConfig.Usage = Image2D::ImageUsageFlags_Sampled | Image2D::ImageUsageFlags_Transfer_Destination;
        imageConfig.Storage = Image2D::MemoryPropertyFlags_DeviceLocal;
        imageConfig.ImageTiling = Image2D::ImageTiling_Optimal;
        imageConfig.ImageFormat = Image2D::Format_RGBA8_SRGB;
        m_TextureImage = Image2D::Create(imageConfig);
        m_TextureImage->Invalidate(1, 1);

        m_IsLoaded = true;

        // Update width and height
        m_Width = 1;
        m_Height = 1;

        // Transfer pixels
        Transfer(&color, 4);
    }

    VulkanTexture2D::~VulkanTexture2D()
    {
    }

    void VulkanTexture2D::Invalidate(u32 width, u32 height)
    {
        m_Width = width;
        m_Height = height;
    }

	void VulkanTexture2D::Transfer(const void* pixels, size_t size)
    {
        VkDevice device = RendererContext::GetDevice();

        RendererBuffer::Config config = {};
        config.Size = size;
        config.UsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        config.MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        config.Temporary = true;

        VulkanBuffer stagingBuffer(config);
        stagingBuffer.SetData(pixels);

        // Executing command buffers
         // Pipeline barrier, transition image layout
        VkCommandBuffer commandBuffer = RendererContext::CreateCommandBuffer(true);
        {
            VkImageMemoryBarrier barrier{};
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
        RendererContext::FlushCommandBuffer(commandBuffer);
        {
            // Copy buffer to image barrier
            VkCommandBuffer commandBuffer = RendererContext::CreateCommandBuffer(true);
            {
                VkBufferImageCopy region{};
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;

                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = 1;

                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = { m_Width, m_Height, 1 };

                vkCmdCopyBufferToImage(
                    commandBuffer,
                    stagingBuffer.GetBuffer(),
                    m_TextureImage.As<VulkanImage2D>()->GetImage(),
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &region
                );
            }
            RendererContext::FlushCommandBuffer(commandBuffer);
        }

        {
            // Pipeline barrier, transition image layout 2
            VkCommandBuffer commandBuffer = RendererContext::CreateCommandBuffer(true);
            {
                VkImageMemoryBarrier barrier{};
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
    }
}
