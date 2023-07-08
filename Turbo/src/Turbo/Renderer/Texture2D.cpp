#include "tbopch.h"
#include "Texture2D.h"

#include "Turbo/Platform/Vulkan/VulkanTexture2D.h"

#define TBO_CACHE_TEXTURES 0

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

            if (config.IsSpriteSheet)
            {
                texture->SetTextureCoords(config.SpriteCoords, config.SpriteSize);
            }

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

    void Texture2D::SetTextureCoords(const glm::vec2& coords, const glm::vec2 spriteSize)
    {
        m_Config.SpriteCoords = coords;
        m_Config.SpriteSize = spriteSize;

        glm::vec2 min = { (m_Config.SpriteCoords.x * m_Config.SpriteSize.x) / GetWidth(), (m_Config.SpriteCoords.y * m_Config.SpriteSize.y) / GetHeight() };
        glm::vec2 max = { ((m_Config.SpriteCoords.x + 1) * m_Config.SpriteSize.x) / GetWidth(), ((m_Config.SpriteCoords.y + 1) * m_Config.SpriteSize.y) / GetHeight() };

        m_TextureCoords[0] = { min.x, min.y };
        m_TextureCoords[1] = { max.x, min.y };
        m_TextureCoords[2] = { max.x, max.y };
        m_TextureCoords[3] = { min.x, max.y };
    }

    void Texture2D::ResetTextureCoords()
    {
        m_TextureCoords = { glm::vec2{ 0.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f }, glm::vec2{ 1.0f, 1.0f }, glm::vec2{ 0.0f, 1.0f } };
    }
}
