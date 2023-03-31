#pragma once

#include <vector>

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
}
