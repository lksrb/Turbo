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
            std::string FilePath;
        };

        static Ref<Texture2D> Create(const Texture2D::Config& config);
        static Ref<Texture2D> Create(u32 color);

        u32 GetWidth() const { return m_Width; }
        u32 GetHeight() const { return m_Height; }
        bool IsLoaded() const { return m_IsLoaded; }

        std::string GetFilepath() const { return m_Config.FilePath; }

        virtual Ref<Image2D> GetImage() const = 0;
        virtual u64 GetHash() const = 0;
        virtual void Invalidate(u32 width, u32 height) = 0;
        virtual ~Texture2D();
    protected:
        Texture2D(const Texture2D::Config& config);
        Texture2D(u32 color);

        u32 m_Width;
        u32 m_Height;
        
        u32 m_Color;

        bool m_IsLoaded = false;
        Texture2D::Config m_Config;
    };

    // TODO: Remove and make this an texture2D feature
    class SubTexture2D
    {
    public:
        SubTexture2D(Ref<Texture2D> texture, const glm::vec2& min, const glm::vec2 max);

        static Ref<SubTexture2D> CreateFromTexture(Ref<Texture2D> texture, const glm::vec2& coords, const glm::vec2 spriteSize);

        Ref<Texture2D> GetTexture() const { return m_Texture; }

        std::array<glm::vec2, 4> GetCoords() const { return m_TexCoords; }
    protected:
        std::array<glm::vec2, 4> m_TexCoords;
        Ref<Texture2D> m_Texture;
    };
}
