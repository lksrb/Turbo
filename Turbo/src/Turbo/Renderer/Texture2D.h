#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Core/Filepath.h"

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
}
