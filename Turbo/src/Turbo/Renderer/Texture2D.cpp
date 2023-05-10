#include "tbopch.h"
#include "Texture2D.h"

#include "Turbo/Platform/Vulkan/VulkanTexture2D.h"

#define TBO_CACHE_TEXTURES 1

namespace Turbo
{
    static std::unordered_map<std::string, Ref<Texture2D>> s_CachedTextures;

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
        return Texture2D::Create(config);
    }

    Ref<Texture2D> Texture2D::Create(u32 color)
    {
        return Ref<VulkanTexture2D>::Create(color);
    }

    Ref<Texture2D> Texture2D::Create(const Texture2D::Config& config)
    {
#if TBO_CACHE_TEXTURES
        // Cache textures for better texture slots management
        if (s_CachedTextures.find(config.Path) == s_CachedTextures.end())
        {
            auto& texture = s_CachedTextures[config.Path];
            texture = Ref<VulkanTexture2D>::Create(config);
            if (texture == nullptr)
            {
                s_CachedTextures.erase(config.Path);
                return nullptr;
            }
        }
        
        return s_CachedTextures.at(config.Path);
#else
        return Ref<VulkanTexture2D>::Create(config);
#endif
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
