#include "tbopch.h"
#include "Texture2D.h"

#include "Turbo/Platform/Vulkan/VulkanTexture2D.h"

namespace Turbo
{
    Texture2D::Texture2D(const Texture2D::Config& config)
        : m_Config(config)
    {
    }

    Texture2D::Texture2D(u32 color)
        : m_Color(color)
    {
    }

    Texture2D::~Texture2D()
    {
    }

    Ptr<Texture2D> Texture2D::Create(const Texture2D::Config& config)
    {
        return new VulkanTexture2D(config);
    }

    Ptr<Texture2D> Texture2D::Create(u32 color)
    {
        return new VulkanTexture2D(color);
    }
}
