#pragma once

#include "Turbo/Renderer/Texture.h"
#include "Turbo/Renderer/Image2D.h"

#include "Turbo/Platform/Vulkan/VulkanImage2D.h"

#include <vulkan/vulkan.h>

namespace Turbo
{
    class VulkanTexture2D : public Texture2D
    {
    public:
        VulkanTexture2D(const Texture2D::Config& config);
        VulkanTexture2D(const Texture2D::Config& config, const void* data, u64 size);
        VulkanTexture2D(u32 color);
        ~VulkanTexture2D();

        void Invalidate(u32 width, u32 height) override;
        void SetData(const void* pixels) override;

        u64 GetHash() const override { return (u64)m_TextureImage.As<VulkanImage2D>()->GetImageView(); }

        Ref<Image2D> GetImage() const override { return m_TextureImage; }
    private:
        void CreateImage2D();

        Ref<Image2D> m_TextureImage;
    };
}
