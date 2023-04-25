#include "tbopch.h"
#include "Texture2D.h"

#include "Turbo/Platform/Vulkan/VulkanTexture2D.h"

namespace Turbo
{
    // Texture2D
    Texture2D::Texture2D(const std::string& filepath)
        : m_FilePath(filepath)
    {
        TBO_ENGINE_ASSERT(!m_FilePath.empty());
    }

    Texture2D::Texture2D(u32 color)
        : m_Color(color)
    {
    }

    Texture2D::Texture2D(const Texture2D::Config& config)
        : m_Config(config)
    {
        TBO_ENGINE_ASSERT(m_Config.Width != 0 && m_Config.Height != 0);
    }

    Texture2D::~Texture2D()
    {
    }

    Ref<Texture2D> Texture2D::Create(const std::string& filepath)
    {
        return Ref<VulkanTexture2D>::Create(filepath);
    }

    Ref<Texture2D> Texture2D::Create(u32 color)
    {
        return Ref<VulkanTexture2D>::Create(color);
    }

    Ref<Texture2D> Texture2D::Create(const Texture2D::Config& config)
    {
        return Ref<VulkanTexture2D>::Create(config);
    }

    // SubTexture2D
    SubTexture2D::SubTexture2D(Ref<Texture2D> texture)
        : m_Texture(texture)
    {
    }

    Ref<SubTexture2D> SubTexture2D::CreateFromTexture(Ref<Texture2D> texture, glm::vec2 coords, glm::vec2 spriteSize)
    {
        Ref<SubTexture2D> subTexture2d = Ref<SubTexture2D>::Create(texture);
        subTexture2d->Snip(coords, spriteSize);
        return subTexture2d;
    }

    void SubTexture2D::Snip(glm::vec2 coords, glm::vec2 spriteSize)
    {
        m_SpriteCoords = coords;
        m_SpriteSize = spriteSize;

        glm::vec2 min = { (coords.x * spriteSize.x) / m_Texture->GetWidth(), (coords.y * spriteSize.y) / m_Texture->GetHeight() };
        glm::vec2 max = { ((coords.x + 1) * spriteSize.x) / m_Texture->GetWidth(), ((coords.y + 1) * spriteSize.y) / m_Texture->GetHeight() };

        m_TexCoords[0] = { min.x, min.y };
        m_TexCoords[1] = { max.x, min.y };
        m_TexCoords[2] = { max.x, max.y };
        m_TexCoords[3] = { min.x, max.y };
    }
}
