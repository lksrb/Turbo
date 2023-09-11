#pragma once

#include <filesystem>

namespace Turbo {

    class Texture2D;

    class Font : public RefCounted
    {
    public:
        struct MSDFData;

        Font(const std::filesystem::path& fontPath);
        ~Font();

        Ref<Texture2D> GetAtlasTexture() const;
        MSDFData* GetMSDFData() const { return m_Data; }

        static Ref<Font> GetDefaultFont();
    private:
        MSDFData* m_Data = nullptr;
        Ref<Texture2D> m_AtlasTexture;
    };
}
