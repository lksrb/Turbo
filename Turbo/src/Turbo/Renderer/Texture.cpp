#include "tbopch.h"
#include "Texture.h"

#include "Turbo/Platform/Vulkan/VulkanTexture2D.h"
#include "Turbo/Platform/Vulkan/VulkanTextureCube.h"

namespace Turbo
{
    Texture2D::Texture2D(const Texture2D::Config& config)
        : m_Config(config)
    {
    }

	Ref<Texture2D> Texture2D::Create(const Texture2D::Config& config, const void* data, u64 size)
	{
        return Ref<VulkanTexture2D>::Create(config, data, size);
	}

	Ref<Texture2D> Texture2D::Create(const std::string& filepath)
    {
        Texture2D::Config config = {};
        config.Path = filepath;
        return Texture2D::Create(config);
    }

    Ref<Texture2D> Texture2D::Create(u32 color)
    {
        return Ref<VulkanTexture2D>::Create(color);
    }

    Ref<Texture2D> Texture2D::Create(const Texture2D::Config& config)
    {
        return Ref<VulkanTexture2D>::Create(config);
    }

    TextureCube::TextureCube(const TextureCube::Config& config)
        : m_Config(config)
    {
    }

    Ref<TextureCube> TextureCube::Create(const TextureCube::Config& config)
    {
        return Ref<VulkanTextureCube>::Create(config);
    }

}