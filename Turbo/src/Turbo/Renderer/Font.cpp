#include "tbopch.h"
#include "Font.h"

#include "Font-Internal.h"

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREAD_COUNT 8

namespace Turbo
{
    template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
    static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, f32 fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
        const msdf_atlas::FontGeometry& fontGeometry, u32 width, u32 height)
    {
        msdf_atlas::GeneratorAttributes attributes;
        attributes.config.overlapSupport = true;
        attributes.scanlinePass = true;

        msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
        generator.setAttributes(attributes);
        generator.setThreadCount(THREAD_COUNT);
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


        // if MSDF || MTSDF
        u64 coloringSeed = 0;
        bool expensiveColoring = true;
        if (expensiveColoring)
        {
            msdf_atlas::Workload([&glyphs = m_Data->Glyphs, &coloringSeed](int i, int threadNo) -> bool {
                unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT);
                glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
                return true;
            }, static_cast<int>(m_Data->Glyphs.size())).finish(THREAD_COUNT);
        }
        else
        {
            unsigned long long glyphSeed = coloringSeed;
            for (msdf_atlas::GlyphGeometry& glyph : m_Data->Glyphs)
            {
                glyphSeed *= LCG_MULTIPLIER;
                glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
            }
        }

        TBO_ENGINE_INFO("Generating texture font atlas...");
        m_AtlasTexture = CreateAndCacheAtlas<u8, f32, 4, msdf_atlas::mtsdfGenerator>("Test", (f32)emSize, m_Data->Glyphs, m_Data->FontGeometry, width, height);
        TBO_ENGINE_INFO("Generated texture font atlas!");
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

    Ref<Font> Font::GetDefaultFont()
    {
        static Ref<Font> s_DefaultFont;

        if (!s_DefaultFont)
        {
            s_DefaultFont = Ref<Font>::Create("Assets\\Fonts\\OpenSans\\OpenSans-Regular.ttf");
        }

        return s_DefaultFont;
    }

}
