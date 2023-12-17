#ifndef SKITY_TEXT_TEXT_RUN_HPP
#define SKITY_TEXT_TEXT_RUN_HPP

#include <memory>
#include <vector>

#include "gl/skity/graphic/path.hpp"

namespace skity {

    using GlyphID = uint32_t;

    class Typeface;

    struct GlyphInfo
    {
        GlyphID id;
        Path path;
        float path_font_size;
        float advance_x;
        float advance_y;
        float ascent;
        float descent;
        float width;
        float height;
        float font_size;
        float bearing_x;
    };

    struct GlyphBitmapInfo
    {
        float width = {};
        float height = {};
        uint8_t* buffer = {};
    };

    /**
     * Simple class for represents a sequence of characters that share a single
     * property set.
     * Like `Typeface`, `FontSize` ...
     *
     */
    class TextRun final
    {
    public:
        TextRun(const std::shared_ptr<Typeface>& typeface, std::vector<GlyphInfo> info,
                float font_size);
        ~TextRun();

        const std::vector<GlyphInfo>& getGlyphInfo() const;

        GlyphBitmapInfo queryBitmapInfo(GlyphID glyph_id);

        std::shared_ptr<Typeface> lockTypeface() const
        {
            return typeface_.lock();
        }

    private:
        std::weak_ptr<Typeface> typeface_ = {};
        std::vector<GlyphInfo> glyph_info_;
        float font_size_ = 0.f;
    };

}  // namespace skity

#endif  // SKITY_TEXT_TEXT_RUN_HPP
