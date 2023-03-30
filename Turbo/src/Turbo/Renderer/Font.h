#pragma once

#include "Turbo/Renderer/Texture2D.h"

#include <filesystem>

namespace Turbo
{
    class Font
    {
    public:
        Font(const std::filesystem::path& fontPath);
        ~Font();

        Ref<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }
    private:
        struct MSDFData;
        MSDFData* m_Data = nullptr;
        Ref<Texture2D> m_AtlasTexture;
    };
}
