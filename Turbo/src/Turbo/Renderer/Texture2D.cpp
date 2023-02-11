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

    Ref<Texture2D> Texture2D::Create(const Texture2D::Config& config)
    {
        return Ref<VulkanTexture2D>::Create(config);
    }

    Ref<Texture2D> Texture2D::Create(u32 color)
    {
        return Ref<VulkanTexture2D>::Create(color);
    }

    // SubTexture2D
    SubTexture2D::SubTexture2D(Ref<Texture2D> texture, const glm::vec2& min, const glm::vec2 max)
        : m_Texture(texture)
    {
        m_TexCoords[0] = { min.x, min.y };
        m_TexCoords[1] = { max.x, min.y };
        m_TexCoords[2] = { max.x, max.y };
        m_TexCoords[3] = { min.x, max.y };
    }

    Ref<SubTexture2D> SubTexture2D::CreateFromTexture(Ref<Texture2D> texture, const glm::vec2& coords, const glm::vec2 spriteSize)
    {
        glm::vec2 min = { (coords.x * spriteSize.x) / texture->GetWidth(), (coords.y * spriteSize.y) / texture->GetHeight() };
        glm::vec2 max = { ((coords.x + 1) * spriteSize.x) / texture->GetWidth(), ((coords.y + 1) * spriteSize.y) / texture->GetHeight() };
        return Ref<SubTexture2D>::Create(texture, min, max);
    }
}
