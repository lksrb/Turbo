#pragma once

#include "Turbo/Core/Common.h"

#include "Image2D.h"

#include <glm/glm.hpp>

namespace Turbo
{
    class Texture2D
    {
    public:
        struct Config
        {
            ImageFormat Format = ImageFormat_RGBA_SRGB;
            ImageFilter Filter = ImageFilter_Linear;
            std::string Path;
            u32 Width = 1;
            u32 Height = 1;
        };
        
        Texture2D() = default;
        Texture2D(const Texture2D::Config& config);
        virtual ~Texture2D();

        static Ref<Texture2D> Create(const std::string& filepath);
        static Ref<Texture2D> Create(const Texture2D::Config& config);
        static Ref<Texture2D> Create(u32 color);

        const Texture2D::Config& GetConfig() const { return m_Config; }

        u32 GetWidth() const { return m_Config.Width; }
        u32 GetHeight() const { return m_Config.Height; }
        bool IsLoaded() const { return m_IsLoaded; }

        virtual Ref<Image2D> GetImage() const = 0;
        virtual u64 GetHash() const = 0;
        virtual void Invalidate(u32 width, u32 height) = 0;
        virtual void SetData(const void* pixels) = 0;
    protected:
        bool m_IsLoaded = false;

        Texture2D::Config m_Config;
    };

    // TODO: Remove and make this an texture2D feature
    class SubTexture2D
    {
    public:
        SubTexture2D(Ref<Texture2D> texture);

        static Ref<SubTexture2D> CreateFromTexture(Ref<Texture2D> texture, glm::vec2 coords = { 0, 0 }, glm::vec2 spriteSize = { 0, 0 });

        Ref<Texture2D> GetTexture() const { return m_Texture; }

        std::array<glm::vec2, 4> GetTextureCoords() const { return m_TexCoords; }

        glm::vec2 GetSpriteCoords() const { return m_SpriteCoords; }
        glm::vec2 GetSpriteSize() const { return m_SpriteSize; }
        void SetBounds(glm::vec2 coords, glm::vec2 spriteSize);
    protected:
        glm::vec2 m_SpriteSize = {};
        glm::vec2 m_SpriteCoords = {};
        std::array<glm::vec2, 4> m_TexCoords = {};
        Ref<Texture2D> m_Texture;
    };
}
