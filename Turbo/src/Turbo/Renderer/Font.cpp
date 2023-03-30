#include "tbopch.h"
#include "Font.h"

#undef INFINITE
#include <msdf-atlas-gen.h>
#include <GlyphGeometry.h>

namespace Turbo
{
    struct Font::MSDFData
    {
        std::vector<msdf_atlas::GlyphGeometry> Glyphs;
        msdf_atlas::FontGeometry FontGeometry;
    };


    template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
    static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, f32 fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
        const msdf_atlas::FontGeometry& fontGeometry, u32 width, u32 height)
    {
        msdf_atlas::GeneratorAttributes attributes;
        attributes.config.overlapSupport = true;
        attributes.scanlinePass = true;

        msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
        generator.setAttributes(attributes);
        generator.setThreadCount(8);
        generator.generate(glyphs.data(), (int)glyphs.size());

        msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

        Texture2D::Config config = {};
        config.Width = bitmap.width;
        config.Height = bitmap.height;
        config.Format = Image2D::Format_RGBA_SRGB;

        Ref<Texture2D> texture = Texture2D::Create(config);
        texture->SetData(bitmap.pixels);
        return texture;
    }

    Font::Font(const std::filesystem::path& filepath)
        : m_Data(new Font::MSDFData)
    {
        msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
        TBO_ENGINE_ASSERT(ft);
        std::string fileString = filepath.string();
        msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString.c_str());
        TBO_ENGINE_ASSERT(font);

        struct CharsetRange
        {
            u32 Begin, End;
        };

        // From imgui_draw.cpp
        static const CharsetRange charsetRanges[] =
        {
            { 0x0020, 0x00FF } // Basic Latin + Latin Supplement
        };

        msdf_atlas::Charset charset;
        for (CharsetRange range : charsetRanges)
        {
            for (u32 c = range.Begin; c <= range.End; ++c)
                charset.add(c);
        }

        f64 fontScale = 1.0f;
        m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->Glyphs);
        i32 glyphsLoaded = m_Data->FontGeometry.loadCharset(font, fontScale, charset);
        TBO_ENGINE_INFO("Loaded {0} glyphs from font (out of {1})", glyphsLoaded, charset.size());

        f64 emSize = 40.0;

        msdf_atlas::TightAtlasPacker atlasPacker;
        //atlasPacker.setDimensionsConstraint(TightAtlasPacker::DimensionsConstraint::EVEN_SQUARE);
        atlasPacker.setPixelRange(2.0);
        atlasPacker.setMiterLimit(1.0);
        atlasPacker.setPadding(0);
        atlasPacker.setScale(emSize);
        i32 remaining = atlasPacker.pack(m_Data->Glyphs.data(), static_cast<i32>(m_Data->Glyphs.size()));
        TBO_ENGINE_ASSERT(remaining == 0);

        int width, height;
        atlasPacker.getDimensions(width, height);
        emSize = atlasPacker.getScale();

        TBO_ENGINE_INFO("Generating texture font atlas...");
        m_AtlasTexture = CreateAndCacheAtlas<u8, f32, 3, msdf_atlas::msdfGenerator>("Test", (f32)emSize, m_Data->Glyphs, m_Data->FontGeometry, width, height);

        // if MSDF || MTSDF


#if 0
        msdfgen::Shape shape;
        if (msdfgen::loadGlyph(shape, font, 'C'))
        {
            shape.normalize();
            //                      max. angle
            msdfgen::edgeColoringSimple(shape, 3.0);
            //           image width, height
            msdfgen::Bitmap<float, 3> msdf(32, 32);
            //                     range, scale, translation
            msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
            msdfgen::savePng(msdf, "output.png");
        }
#endif
        msdfgen::destroyFont(font);
        msdfgen::deinitializeFreetype(ft);
    }

    Font::~Font()
    {
        delete m_Data;
        m_Data = nullptr;
    }

}
