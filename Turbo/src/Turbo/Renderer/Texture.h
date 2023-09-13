#pragma once

#include "Turbo/Asset/Asset.h"

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
        };

        Texture2D() = default;
        Texture2D(const Texture2D::Config& config);
        virtual ~Texture2D() = default;

        template<u64 Size>
        static Ref<Texture2D> Create(const std::array<u8, Size>& data)
        {
            Texture2D::Config config;
            config.Filter = ImageFilter_Linear;
            config.Format = ImageFormat_RGBA_SRGB;
            return Create(config, data.data(), data.size());
        }


        static Ref<Texture2D> Create(const Texture2D::Config& config, const void* data, u64 size);
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

        AssetType GetAssetType() const override { return AssetType_Texture2D; }
        static constexpr AssetType GetStaticAssetType() { return AssetType_Texture2D; }
    protected:
        bool m_IsLoaded = false;
        Texture2D::Config m_Config;
    };

    class TextureCube : public Asset
    {
    public:
        struct Config
        {
            std::string Path; // Path to one file that contains all 6 texture (something like minecraft)
            u32 Width = 1;
            u32 Height = 1;
        };

        TextureCube(const TextureCube::Config& config);
        virtual ~TextureCube() = default;

        static Ref<TextureCube> Create(const TextureCube::Config& config = {});

        const TextureCube::Config& GetConfig() const { return m_Config; }
        AssetType GetAssetType() const override { return AssetType_Count; /* TODO: Create asset */ };
    protected:
        TextureCube::Config m_Config;
    };
}
