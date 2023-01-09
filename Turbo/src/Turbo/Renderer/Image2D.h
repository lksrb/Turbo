#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    class Image2D
    {
    public:
        enum Format : u32
        {
            Format_RGBA8_SRGB = 43,
            Format_BGRA8_Unorm = 44,
            Format_BGRA8_SRGB = 50,
            Format_D32_SFloat_S8_Uint = 130
        };

        enum ImageTiling : u32
        {
            ImageTiling_Optimal = 0,
            ImageTiling_Linear = 1,
        };

        enum AspectFlags : u32
        {
            AspectFlags_Color   = 0x00000001,
            AspectFlags_Depth   = 0x00000002,
            AspectFlags_Stencil = 0x00000004
        };

        enum MemoryPropertyFlags_ : u32
        {
            MemoryPropertyFlags_DeviceLocal = 0x00000001,
            MemoryPropertyFlags_HostVisible = 0x00000002,
            MemoryPropertyFlags_HostCoherent = 0x00000004,
            MemoryPropertyFlags_HostCached = 0x00000008
        };

        enum ImageUsageFlags_ : u32
        {
            ImageUsageFlags_Transfer_Source = 0x00000001,
            ImageUsageFlags_Transfer_Destination = 0x00000002,
            ImageUsageFlags_Sampled = 0x00000004,
            ImageUsageFlags_Storage = 0x00000008,
            ImageUsageFlags_ColorAttachment = 0x00000010,
            ImageUsageFlags_DepthStencilSttachment = 0x00000020,
        };

        using MemoryPropertyFlags   = u32;
        using ImageUsageFlags       = u32;
        using MemoryPropertyFlags   = u32;

        struct Config
        {
            Format                  ImageFormat;    // VkFormat
            ImageTiling             ImageTiling;    // VkImageTiling
            AspectFlags             Aspect;         // VkImageAspectFlags
            MemoryPropertyFlags     Storage;        // VkMemoryPropertyFlags
            ImageUsageFlags         Usage;          // VkImageUsageFlags
        };

        virtual ~Image2D();

        static Image2D* Create(const Image2D::Config& config);

        const Image2D::Config& GetConfig() const { return m_Config; }
        u32 GetWidth() const { return m_Width; }
        u32 GetHeight() const { return m_Height; }

        void SetExtent(u32 width, u32 height);
        virtual void Invalidate(u32 width, u32 height) = 0;
    protected:
        Image2D(const Image2D::Config& config);
    protected:
        u32 m_Width;
        u32 m_Height;

        Image2D::Config m_Config;
    };
}
