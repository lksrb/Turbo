#pragma once

#include "Turbo/Renderer/Texture2D.h"

#include <filesystem>

namespace Turbo
{
    class Font
    {
    public:
        struct MSDFData;

        Font(const std::filesystem::path& fontPath);
        ~Font();

        Ref<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }
        MSDFData* GetMSDFData() const { return m_Data; }

        static Ref<Font> GetDefaultFont();
    private:
        MSDFData* m_Data = nullptr;
        Ref<Texture2D> m_AtlasTexture;
    };
}
