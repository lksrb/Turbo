#include "tbopch.h"
#include "Texture2D.h"

#include "Turbo/Platform/Vulkan/VulkanTexture2D.h"

namespace Turbo
{
    // Texture2D
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

    // SubTexture2D
    SubTexture2D::SubTexture2D(Ptr<Texture2D> texture, const glm::vec2& min, const glm::vec2 max)
        : m_Texture(texture)
    {
        m_TexCoords[0] = { min.x, min.y };
        m_TexCoords[1] = { max.x, min.y };
        m_TexCoords[2] = { max.x, max.y };
        m_TexCoords[3] = { min.x, max.y };
    }

    Ptr<SubTexture2D> SubTexture2D::CreateFromTexture(Ptr<Texture2D> texture, const glm::vec2& coords, const glm::vec2 spriteSize)
    {
        glm::vec2 min = { (coords.x * spriteSize.x) / texture->GetWidth(), (coords.y * spriteSize.y) / texture->GetHeight() };
        glm::vec2 max = { ((coords.x + 1) * spriteSize.x) / texture->GetWidth(), ((coords.y + 1) * spriteSize.y) / texture->GetHeight() };
        return new SubTexture2D(texture, min, max);
    }
}
