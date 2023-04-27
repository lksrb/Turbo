#include "tbopch.h"
#include "Texture2D.h"

#include "Turbo/Platform/Vulkan/VulkanTexture2D.h"

namespace Turbo
{
    Texture2D::Texture2D(const Texture2D::Config& config)
        : m_Config(config)
    {
    }

    Texture2D::~Texture2D()
    {
    }

    Ref<Texture2D> Texture2D::Create(const std::string& filepath)
    {
        Texture2D::Config config = {};
        config.Path = filepath;
        return Ref<VulkanTexture2D>::Create(config);
    }

    Ref<Texture2D> Texture2D::Create(u32 color)
    {
        return Ref<VulkanTexture2D>::Create(color);
    }

    Ref<Texture2D> Texture2D::Create(const Texture2D::Config& config)
    {
        return Ref<VulkanTexture2D>::Create(config);
    }

    SubTexture2D::SubTexture2D(Ref<Texture2D> texture)
        : m_Texture(texture)
    {
    }

    Ref<SubTexture2D> SubTexture2D::CreateFromTexture(Ref<Texture2D> texture, glm::vec2 coords, glm::vec2 spriteSize)
    {
        Ref<SubTexture2D> subTexture = Ref<SubTexture2D>::Create(texture);
        subTexture->SetBounds(coords, spriteSize);
        return subTexture;
    }

    void SubTexture2D::SetBounds(glm::vec2 coords, glm::vec2 spriteSize)
    {
        m_SpriteCoords = coords;
        m_SpriteSize.x = spriteSize.x ? spriteSize.x : m_Texture->GetWidth();
        m_SpriteSize.y = spriteSize.y ? spriteSize.y : m_Texture->GetHeight();

        glm::vec2 min = { (coords.x * m_SpriteSize.x) / m_Texture->GetWidth(), (coords.y * m_SpriteSize.y) / m_Texture->GetHeight() };
        glm::vec2 max = { ((coords.x + 1) * m_SpriteSize.x) / m_Texture->GetWidth(), ((coords.y + 1) * m_SpriteSize.y) / m_Texture->GetHeight() };

        m_TexCoords[0] = { min.x, min.y };
        m_TexCoords[1] = { max.x, min.y };
        m_TexCoords[2] = { max.x, max.y };
        m_TexCoords[3] = { min.x, max.y };
    }
}
