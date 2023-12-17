#ifndef SKITY_SRC_RENDER_TEXT_FONT_TEXTURE_HPP
#define SKITY_SRC_RENDER_TEXT_FONT_TEXTURE_HPP

#include <map>
#include <memory>

#include <glm/glm.hpp>

#include "gl/skity/render/texture_atlas.hpp"
#include "gl/skity/text/typeface.hpp"

namespace skity {

    class FontTexture : public TextureAtlas
    {
        enum {
            DEFAULT_SIZE = 512,
        };

        struct GlyphKey
        {
            GlyphID id{};
            float font_size{};

            // constexpr GlyphKey() = default;
            // constexpr ~GlyphKey() = default;

            // constexpr bool operator==(const GlyphKey& other) const;
        };

        struct GlyphKeyCompare
        {
            bool operator()(const GlyphKey& lhs, const GlyphKey& rhs) const
            {
                if (lhs.id < rhs.id)
                    return true;

                if (lhs.id == rhs.id)
                    return lhs.font_size < rhs.font_size;

                return false;
            }
        };

    public:
        FontTexture(Typeface* typeface);
        ~FontTexture() override = default;

        glm::ivec4 GetGlyphRegion(GlyphID glyph_id, float font_size);

    private:
        glm::ivec4 GenerateGlyphRegion(const GlyphKey& key);

    private:
        Typeface* typeface_;
        std::map<GlyphKey, glm::ivec4, GlyphKeyCompare> glyph_regions_ = {};
    };

}  // namespace skity

#endif  // SKITY_SRC_RENDER_TEXT_FONT_TEXTURE_HPP
