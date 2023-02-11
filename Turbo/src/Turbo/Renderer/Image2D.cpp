#include "tbopch.h"
#include "Image2D.h"

#include "Turbo/Platform/Vulkan/VulkanImage2D.h"

namespace Turbo
{
    Image2D::Image2D(const Image2D::Config& config)
        : m_Config(config), m_Width(0), m_Height(0)
    {
    }

    Image2D::~Image2D()
    {
    }

    Ref<Image2D> Image2D::Create(const Image2D::Config& config)
    {
        return Ref<VulkanImage2D>::Create(config);
    }

    void Image2D::SetExtent(u32 width, u32 height)
    {
        m_Width = width;
        m_Height = height;
    }

}
