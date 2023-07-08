#pragma once

#include "Turbo/Asset/Asset.h"

#include "Turbo/Core/Common.h"

#include "Image2D.h"

#include <glm/glm.hpp>

namespace Turbo
{
    class Texture2D : public Asset
    {
    public:
        struct Config
        {
            ImageFormat Format = ImageFormat_RGBA_SRGB;
            ImageFilter Filter = ImageFilter_Linear;
            std::string Path;
            u32 Width = 1;
            u32 Height = 1;

            bool IsSpriteSheet = false;
            glm::vec2 SpriteCoords;
            glm::vec2 SpriteSize;
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

        void SetTextureCoords(const glm::vec2& coords, const glm::vec2 spriteSize);
        void ResetTextureCoords();
        void SetSpriteSheet(bool isSpriteSheet) { m_Config.IsSpriteSheet = isSpriteSheet; }

        std::array<glm::vec2, 4> GetTextureCoords() { return m_TextureCoords; }

        virtual Ref<Image2D> GetImage() const = 0;
        virtual u64 GetHash() const = 0;
        virtual void Invalidate(u32 width, u32 height) = 0;
        virtual void SetData(const void* pixels) = 0;

        AssetType GetAssetType() const override { return AssetType_Texture2D; }
    protected:
        std::array<glm::vec2, 4> m_TextureCoords = {
            glm::vec2{ 0.0f, 0.0f },
            glm::vec2{ 1.0f, 0.0f },
            glm::vec2{ 1.0f, 1.0f },
            glm::vec2{ 0.0f, 1.0f } 
        };
        bool m_IsLoaded = false;
        Texture2D::Config m_Config;
    };
}
