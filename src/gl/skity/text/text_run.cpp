#include "gl/skity/text/text_run.hpp"
#include "gl/skity/text/typeface.hpp"

namespace skity {

    TextRun::TextRun(const std::shared_ptr<Typeface>& typeface, std::vector<GlyphInfo> info,
                     float font_size)
        : typeface_(typeface)
        , glyph_info_(std::move(info))
        , font_size_(font_size)
    {
    }

    TextRun::~TextRun() = default;

    const std::vector<GlyphInfo>& TextRun::getGlyphInfo() const
    {
        return glyph_info_;
    }

    GlyphBitmapInfo TextRun::queryBitmapInfo(GlyphID glyph_id)
    {
        auto typeface = lockTypeface();

        if (!typeface)
            return {};

        return typeface->getGlyphBitmapInfo(glyph_id, font_size_);
    }

}  // namespace skity
