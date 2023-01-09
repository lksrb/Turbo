#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Core/Filepath.h"

#include <glm/glm.hpp>

namespace Turbo
{
    class Texture2D
    {
    public:
        struct Config
        {
            Filepath FilePath;
        };

        static Ptr<Texture2D> Create(const Texture2D::Config& config);
        static Ptr<Texture2D> Create(u32 color);

        u32 GetWidth() const { return m_Width; }
        u32 GetHeight() const { return m_Height; }

        virtual u64 GetHash() const = 0;
        virtual void Invalidate(u32 width, u32 height) = 0;
        virtual ~Texture2D();
    protected:
        Texture2D(const Texture2D::Config& config);
        Texture2D(u32 color);

        u32 m_Width;
        u32 m_Height;

        u32 m_Color;
        Texture2D::Config m_Config;
    };

    class SubTexture2D
    {
    public:
        static Ptr<SubTexture2D> CreateFromTexture(Ptr<Texture2D> texture, const glm::vec2& coords, const glm::vec2 spriteSize);

        Ptr<Texture2D> GetTexture() const { return m_Texture; }

        std::array<glm::vec2, 4> GetCoords() const { return m_TexCoords; }
    private:
        SubTexture2D(Ptr<Texture2D> texture, const glm::vec2& min, const glm::vec2 max);

        std::array<glm::vec2, 4> m_TexCoords;

        Ptr<Texture2D> m_Texture;
    };
}
