#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    enum ImageFormat : u32
    {
        ImageFormat_None = 0,
        ImageFormat_RGB_Unorm = 23,
        ImageFormat_RGB_UInt = 27,
        ImageFormat_RGB_SRGB = 29,

        ImageFormat_RGBA_SRGB = 43,
        ImageFormat_RGBA_Unorm = 37,

        ImageFormat_BGRA_Unorm = 44,
        ImageFormat_BGRA_SRGB = 50,

        ImageFormat_RGBA32F = 109,

        ImageFormat_D32_SFloat_S8_Uint = 130
    };

    enum ImageTiling : u32
    {
        ImageTiling_Optimal = 0,
        ImageTiling_Linear = 1,
    };

    enum ImageAspect : u32
    {
        ImageAspect_Color = 0x00000001,
        ImageAspect_Depth = 0x00000002,
        ImageAspect_Stencil = 0x00000004
    };

    enum MemoryStorage_ : u32
    {
        MemoryStorage_DeviceLocal = 0x00000001,
        MemoryStorage_HostVisible = 0x00000002,
        MemoryStorage_HostCoherent = 0x00000004,
        MemoryStorage_HostCached = 0x00000008
    };

    enum ImageUsageFlags_ : u32
    {
        ImageUsage_Transfer_Source = 0x00000001,
        ImageUsage_Transfer_Destination = 0x00000002,
        ImageUsage_Sampled = 0x00000004,
        ImageUsage_Storage = 0x00000008,
        ImageUsage_ColorAttachment = 0x00000010,
        ImageUsage_DepthStencilSttachment = 0x00000020,
    };

    enum ImageFilter : u32
    {
        ImageFilter_Nearest = 0,
        ImageFilter_Linear
    };

    enum ImageLayout : u32
    {
        ImageLayout_ReadOnly_Optimal = 1000314000,
        ImageLayout_PresentSrcKhr = 1000001002,
        ImageLayout_Shader_ReadOnly_Optimal = 5
    };

    using MemoryStorage = u32;
    using ImageUsage = u32;

    class Image2D
    {
    public:
        struct Config
        {
            ImageFormat Format;                         // VkFormat
            ImageTiling Tiling;                         // VkImageTiling
            ImageAspect Aspect;                         // VkImageAspectFlags
            MemoryStorage MemoryStorage;                // VkMemoryPropertyFlags
            ImageUsage Usage;                           // VkImageUsageFlags
            ImageFilter Filter = ImageFilter_Linear;    // VkFilter
        };

        virtual ~Image2D();

        static Ref<Image2D> Create(const Image2D::Config& config);

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
