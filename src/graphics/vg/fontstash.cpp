#include <cstdint>
#include <cstdio>

#include "graphics/stb/stb_truetype.hpp"
#include "graphics/vg/fontstash.hpp"

namespace rl::nvg {
    using namespace rl::stb;

    constexpr int32_t FONS_UTF8_ACCEPT{ 0 };

    namespace {
        uint32_t fons_hashint(uint32_t a)
        {
            a += ~(a << 15);
            a ^= (a >> 10);
            a += (a << 3);
            a ^= (a >> 6);
            a += ~(a << 11);
            a ^= (a >> 16);
            return a;
        }

        int32_t fons_mini(const int32_t a, const int32_t b)
        {
            return a < b ? a : b;
        }

        int32_t fons_maxi(const int32_t a, const int32_t b)
        {
            return a > b ? a : b;
        }

        int32_t fons_tt_init(FONScontext* context)
        {
            return 1;
        }

        int32_t fons_tt_done(FONScontext* context)
        {
            return 1;
        }

        int32_t fons_tt_load_font(FONScontext* context, FONSttFontImpl* font, const uint8_t* data,
                                  int32_t data_size, const int32_t font_index)
        {
            int32_t stb_error;
            font->font.userdata = context;
            const int32_t offset = stbtt_GetFontOffsetForIndex(data, font_index);
            if (offset == -1)
                stb_error = 0;
            else
                stb_error = stbtt_InitFont(&font->font, data, offset);
            return stb_error;
        }

        void fons_tt_get_font_v_metrics(const FONSttFontImpl* font, int32_t* ascent,
                                        int32_t* descent, int32_t* lineGap)
        {
            stbtt_GetFontVMetrics(&font->font, ascent, descent, lineGap);
        }

        float fons_tt_get_pixel_height_scale(const FONSttFontImpl* font, const float size)
        {
            return stbtt_ScaleForMappingEmToPixels(&font->font, size);
        }

        int32_t fons_tt_get_glyph_index(const FONSttFontImpl* font, const int32_t codepoint)
        {
            return stbtt_FindGlyphIndex(&font->font, codepoint);
        }

        int32_t fons_tt_build_glyph_bitmap(
            const FONSttFontImpl* font, const int32_t glyph, float size, const float scale,
            int32_t* advance, int32_t* lsb, int32_t* x0, int32_t* y0, int32_t* x1, int32_t* y1)
        {
            stbtt_GetGlyphHMetrics(&font->font, glyph, advance, lsb);
            stbtt_GetGlyphBitmapBox(&font->font, glyph, scale, scale, x0, y0, x1, y1);
            return 1;
        }

        void fons_tt_render_glyph_bitmap(const FONSttFontImpl* font, uint8_t* output,
                                         const int32_t out_width, const int32_t out_height,
                                         const int32_t outStride, const float scaleX,
                                         const float scale_y, const int32_t glyph)
        {
            stbtt_MakeGlyphBitmap(&font->font, output, out_width, out_height, outStride, scaleX,
                                  scale_y, glyph);
        }

        int32_t fons_tt_get_glyph_kern_advance(const FONSttFontImpl* font, const int32_t glyph1,
                                               const int32_t glyph2)
        {
            return stbtt_GetGlyphKernAdvance(&font->font, glyph1, glyph2);
        }

        uint32_t fons_decutf8(uint32_t* state, uint32_t* codep, const uint32_t byte)
        {
            constexpr static uint8_t utf8d[] = {
                // The first part of the table maps bytes to character classes that
                // to reduce the size of the transition table and create bitmasks.
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 7, 7, 7,
                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 2, 2,
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                10, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 11, 6, 6, 6, 5, 8, 8, 8, 8, 8, 8,
                8, 8, 8, 8, 8,

                // The second part is a transition table that maps a combination
                // of a state of the automaton and a character class to a state.
                0, 12, 24, 36, 60, 96, 84, 12, 12, 12, 48, 72, 12, 12, 12, 12, 12, 12, 12, 12, 12,
                12, 12, 12, 12, 0, 12, 12, 12, 12, 12, 0, 12, 0, 12, 12, 12, 24, 12, 12, 12, 12, 12,
                24, 12, 24, 12, 12, 12, 12, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 24, 12, 12,
                12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12, 12, 12, 12, 36, 12, 36, 12, 12, 12,
                36, 12, 12, 12, 12, 12, 36, 12, 36, 12, 12, 12, 36, 12, 12, 12, 12, 12, 12, 12, 12,
                12, 12
            };

            const uint32_t type = utf8d[byte];

            *codep = (*state != FONS_UTF8_ACCEPT) ? (byte & 0x3fu) | (*codep << 6)
                                                  : (0xff >> type) & (byte);

            *state = utf8d[256 + *state + type];
            return *state;
        }

        // Atlas based on Skyline Bin Packer by Jukka Jyl�nki
        void fons_delete_atlas(FONSatlas* atlas)
        {
            if (atlas == nullptr)
                return;
            if (atlas->nodes != nullptr)
                free(atlas->nodes);
            free(atlas);
        }

        FONSatlas* fons_alloc_atlas(const int32_t w, const int32_t h, const int32_t nnodes)
        {
            // Allocate memory for the font stash.
            const auto atlas = static_cast<FONSatlas*>(std::malloc(sizeof(FONSatlas)));
            if (atlas != nullptr)
            {
                memset(atlas, 0, sizeof(FONSatlas));

                atlas->width = w;
                atlas->height = h;

                // Allocate space for skyline nodes
                atlas->nodes = static_cast<FONSatlasNode*>(
                    std::malloc(sizeof(FONSatlasNode) * nnodes));
                if (atlas->nodes != nullptr)
                {
                    memset(atlas->nodes, 0, sizeof(FONSatlasNode) * nnodes);
                    atlas->nnodes = 0;
                    atlas->cnodes = nnodes;

                    // Init root node.
                    atlas->nodes[0].x = 0;
                    atlas->nodes[0].y = 0;
                    atlas->nodes[0].width = static_cast<int16_t>(w);
                    atlas->nnodes++;

                    return atlas;
                }
            }

            // error
            if (atlas != nullptr)
                fons_delete_atlas(atlas);

            return nullptr;
        }

        int32_t fons_atlas_insert_node(FONSatlas* atlas, const int32_t idx, const int32_t x,
                                       const int32_t y, const int32_t w)
        {
            // Insert node
            if (atlas->nnodes + 1 > atlas->cnodes)
            {
                atlas->cnodes = atlas->cnodes == 0 ? 8 : atlas->cnodes * 2;
                atlas->nodes = static_cast<FONSatlasNode*>(
                    realloc(atlas->nodes, sizeof(FONSatlasNode) * atlas->cnodes));

                if (atlas->nodes == nullptr)
                    return 0;
            }

            for (int32_t i = atlas->nnodes; i > idx; i--)
                atlas->nodes[i] = atlas->nodes[i - 1];

            atlas->nodes[idx].x = static_cast<int16_t>(x);
            atlas->nodes[idx].y = static_cast<int16_t>(y);
            atlas->nodes[idx].width = static_cast<int16_t>(w);
            atlas->nnodes++;

            return 1;
        }

        void fons_atlas_remove_node(FONSatlas* atlas, const int32_t idx)
        {
            if (atlas->nnodes == 0)
                return;

            for (int32_t i = idx; i < atlas->nnodes - 1; i++)
                atlas->nodes[i] = atlas->nodes[i + 1];

            atlas->nnodes--;
        }

        void fons_atlas_expand(FONSatlas* atlas, const int32_t w, const int32_t h)
        {
            // Insert node for empty space
            if (w > atlas->width)
                fons_atlas_insert_node(atlas, atlas->nnodes, atlas->width, 0, w - atlas->width);

            atlas->width = w;
            atlas->height = h;
        }

        void fons_atlas_reset(FONSatlas* atlas, const int32_t w, const int32_t h)
        {
            atlas->width = w;
            atlas->height = h;
            atlas->nnodes = 0;

            // Init root node.
            atlas->nodes[0].x = 0;
            atlas->nodes[0].y = 0;
            atlas->nodes[0].width = static_cast<int16_t>(w);
            atlas->nnodes++;
        }

        int32_t fons_atlas_add_skyline_level(FONSatlas* atlas, const int32_t idx, const int32_t x,
                                             const int32_t y, const int32_t w, const int32_t h)
        {
            int32_t i;

            // Insert new node
            if (fons_atlas_insert_node(atlas, idx, x, y + h, w) == 0)
                return 0;

            // Delete skyline segments that fall under the shadow of the new segment.
            for (i = idx + 1; i < atlas->nnodes; i++)
            {
                if (atlas->nodes[i].x < atlas->nodes[i - 1].x + atlas->nodes[i - 1].width)
                {
                    const int32_t shrink = atlas->nodes[i - 1].x + atlas->nodes[i - 1].width -
                                           atlas->nodes[i].x;
                    atlas->nodes[i].x += static_cast<int16_t>(shrink);
                    atlas->nodes[i].width -= static_cast<int16_t>(shrink);
                    if (atlas->nodes[i].width > 0)
                        break;
                    else
                    {
                        fons_atlas_remove_node(atlas, i);
                        i--;
                    }
                }
                else
                {
                    break;
                }
            }

            // Merge same height skyline segments that are next to each other.
            for (i = 0; i < atlas->nnodes - 1; i++)
            {
                if (atlas->nodes[i].y == atlas->nodes[i + 1].y)
                {
                    atlas->nodes[i].width += atlas->nodes[i + 1].width;
                    fons_atlas_remove_node(atlas, i + 1);
                    i--;
                }
            }

            return 1;
        }

        int32_t fons_atlas_rect_fits(const FONSatlas* atlas, int32_t i, const int32_t w,
                                     const int32_t h)
        {
            // Checks if there is enough space at the location of skyline span 'i',
            // and return the max height of all skyline spans under that at that location,
            // (think tetris block being dropped at that position). Or -1 if no space found.
            const int32_t x = atlas->nodes[i].x;
            int32_t y = atlas->nodes[i].y;
            if (x + w > atlas->width)
                return -1;

            int32_t spaceLeft = w;
            while (spaceLeft > 0)
            {
                if (i == atlas->nnodes)
                    return -1;

                y = fons_maxi(y, atlas->nodes[i].y);
                if (y + h > atlas->height)
                    return -1;

                spaceLeft -= atlas->nodes[i].width;
                ++i;
            }

            return y;
        }

        int32_t fons_atlas_add_rect(FONSatlas* atlas, const int32_t rw, const int32_t rh,
                                    int32_t* rx, int32_t* ry)
        {
            int32_t besth = atlas->height, bestw = atlas->width, besti = -1;
            int32_t bestx = -1, besty = -1;

            // Bottom left fit heuristic.
            for (int32_t i = 0; i < atlas->nnodes; i++)
            {
                const int32_t y = fons_atlas_rect_fits(atlas, i, rw, rh);
                if (y != -1)
                {
                    if (y + rh < besth || (y + rh == besth && atlas->nodes[i].width < bestw))
                    {
                        besti = i;
                        bestw = atlas->nodes[i].width;
                        besth = y + rh;
                        bestx = atlas->nodes[i].x;
                        besty = y;
                    }
                }
            }

            if (besti == -1)
                return 0;

            // Perform the actual packing.
            if (fons_atlas_add_skyline_level(atlas, besti, bestx, besty, rw, rh) == 0)
                return 0;

            *rx = bestx;
            *ry = besty;

            return 1;
        }

        void fons_add_white_rect(FONScontext* stash, const int32_t w, const int32_t h)
        {
            int32_t gx, gy;
            if (fons_atlas_add_rect(stash->atlas, w, h, &gx, &gy) == 0)
                return;

            // Rasterize
            uint8_t* dst = &stash->tex_data[gx + gy * stash->params.width];
            for (int32_t y = 0; y < h; y++)
            {
                for (int32_t x = 0; x < w; x++)
                    dst[x] = 0xff;
                dst += stash->params.width;
            }

            stash->dirty_rect[0] = fons_mini(stash->dirty_rect[0], gx);
            stash->dirty_rect[1] = fons_mini(stash->dirty_rect[1], gy);
            stash->dirty_rect[2] = fons_maxi(stash->dirty_rect[2], gx + w);
            stash->dirty_rect[3] = fons_maxi(stash->dirty_rect[3], gy + h);
        }

        FONSstate* fons_get_state(FONScontext* stash)
        {
            return &stash->states[stash->nstates - 1];
        }

#define APREC 16
#define ZPREC 7

        void fons_blur_cols(uint8_t* dst, const int32_t w, const int32_t h, const int32_t dstStride,
                            const int32_t alpha)
        {
            int32_t x;
            for (int32_t y = 0; y < h; y++)
            {
                int32_t z = 0;  // force zero border
                for (x = 1; x < w; x++)
                {
                    z += (alpha * ((static_cast<int32_t>(dst[x]) << ZPREC) - z)) >> APREC;
                    dst[x] = static_cast<uint8_t>(z >> ZPREC);
                }
                dst[w - 1] = 0;  // force zero border
                z = 0;
                for (x = w - 2; x >= 0; x--)
                {
                    z += (alpha * ((static_cast<int32_t>(dst[x]) << ZPREC) - z)) >> APREC;
                    dst[x] = static_cast<uint8_t>(z >> ZPREC);
                }
                dst[0] = 0;  // force zero border
                dst += dstStride;
            }
        }

        void fons_blur_rows(uint8_t* dst, const int32_t w, const int32_t h, const int32_t dstStride,
                            const int32_t alpha)
        {
            int32_t y;
            for (int32_t x = 0; x < w; x++)
            {
                int32_t z = 0;  // force zero border
                for (y = dstStride; y < h * dstStride; y += dstStride)
                {
                    z += (alpha * ((static_cast<int32_t>(dst[y]) << ZPREC) - z)) >> APREC;
                    dst[y] = static_cast<uint8_t>(z >> ZPREC);
                }
                dst[(h - 1) * dstStride] = 0;  // force zero border
                z = 0;
                for (y = (h - 2) * dstStride; y >= 0; y -= dstStride)
                {
                    z += (alpha * ((static_cast<int32_t>(dst[y]) << ZPREC) - z)) >> APREC;
                    dst[y] = static_cast<uint8_t>(z >> ZPREC);
                }
                dst[0] = 0;  // force zero border
                dst++;
            }
        }

        void fons_blur(const FONScontext* stash, uint8_t* dst, const int32_t w, const int32_t h,
                       const int32_t dstStride, const int32_t blur)
        {
            (void)stash;

            if (blur < 1)
                return;
            // Calculate the alpha such that 90% of the kernel is within the radius. (Kernel extends
            // to infinity)
            const float sigma = static_cast<float>(blur) * 0.57735f;  // 1 / sqrt(3)
            const int32_t alpha = static_cast<int32_t>(
                (1 << APREC) * (1.0f - expf(-2.3f / (sigma + 1.0f))));
            fons_blur_rows(dst, w, h, dstStride, alpha);
            fons_blur_cols(dst, w, h, dstStride, alpha);
            fons_blur_rows(dst, w, h, dstStride, alpha);
            fons_blur_cols(dst, w, h, dstStride, alpha);
            //	fons__blurrows(dst, w, h, dstStride, alpha);
            //	fons__blurcols(dst, w, h, dstStride, alpha);
        }

        FONSglyph* fons_alloc_glyph(FONSfont* font)
        {
            if (font->nglyphs + 1 > font->cglyphs)
            {
                font->cglyphs = font->cglyphs == 0 ? 8 : font->cglyphs * 2;
                font->glyphs = static_cast<FONSglyph*>(
                    realloc(font->glyphs, sizeof(FONSglyph) * font->cglyphs));
                if (font->glyphs == nullptr)
                    return nullptr;
            }
            font->nglyphs++;
            return &font->glyphs[font->nglyphs - 1];
        }

        FONSglyph* fons_get_glyph(FONScontext* stash, FONSfont* font, const uint32_t codepoint,
                                  const int16_t isize, int16_t iblur, const int32_t bitmapOption)
        {
            int32_t advance;
            int32_t lsb;
            int32_t x0;
            int32_t y0;
            int32_t x1;
            int32_t y1;
            int32_t gx;
            int32_t gy;
            FONSglyph* glyph = nullptr;
            const float size = isize / 10.0f;
            const FONSfont* render_font = font;

            if (isize < 2)
                return nullptr;
            if (iblur > 20)
                iblur = 20;
            const int32_t pad = iblur + 2;

            // Reset allocator.
            stash->nscratch = 0;

            // Find code point and size.
            const uint32_t h = fons_hashint(codepoint) & (FONS_HASH_LUT_SIZE - 1);
            int32_t i = font->lut[h];
            while (i != -1)
            {
                if (font->glyphs[i].codepoint == codepoint && font->glyphs[i].size == isize &&
                    font->glyphs[i].blur == iblur)
                {
                    glyph = &font->glyphs[i];
                    if (bitmapOption == FonsGlyphBitmapOptional ||
                        (glyph->x0 >= 0 && glyph->y0 >= 0))
                        return glyph;
                    // At this point, glyph exists but the bitmap data is not yet created.
                    break;
                }
                i = font->glyphs[i].next;
            }

            // Create a new glyph or rasterize bitmap data for a cached glyph.
            int32_t g = fons_tt_get_glyph_index(&font->font, codepoint);
            // Try to find the glyph in fallback fonts.
            if (g == 0)
            {
                for (i = 0; i < font->nfallbacks; ++i)
                {
                    const FONSfont* fallbackFont = stash->fonts[font->fallbacks[i]];
                    const int32_t fallback_index = fons_tt_get_glyph_index(&fallbackFont->font,
                                                                           codepoint);
                    if (fallback_index != 0)
                    {
                        g = fallback_index;
                        render_font = fallbackFont;
                        break;
                    }
                }
                // It is possible that we did not find a fallback glyph.
                // In that case the glyph index 'g' is 0, and we'll proceed below and cache empty
                // glyph.
            }
            const float scale = fons_tt_get_pixel_height_scale(&render_font->font, size);
            fons_tt_build_glyph_bitmap(&render_font->font, g, size, scale, &advance, &lsb, &x0, &y0,
                                       &x1, &y1);
            const int32_t gw = x1 - x0 + pad * 2;
            const int32_t gh = y1 - y0 + pad * 2;

            // Determines the spot to draw glyph in the atlas.
            if (bitmapOption == FonsGlyphBitmapRequired)
            {
                // Find free spot for the rect in the atlas
                int32_t added = fons_atlas_add_rect(stash->atlas, gw, gh, &gx, &gy);
                if (added == 0 && stash->handle_error != nullptr)
                {
                    // Atlas is full, let the user to resize the atlas (or not), and try again.
                    stash->handle_error(stash->error_uptr, FonsAtlasFull, 0);
                    added = fons_atlas_add_rect(stash->atlas, gw, gh, &gx, &gy);
                }
                if (added == 0)
                    return nullptr;
            }
            else
            {
                // Negative coordinate indicates there is no bitmap data created.
                gx = -1;
                gy = -1;
            }

            // Init glyph.
            if (glyph == nullptr)
            {
                glyph = fons_alloc_glyph(font);
                glyph->codepoint = codepoint;
                glyph->size = isize;
                glyph->blur = iblur;
                glyph->next = 0;

                // Insert char to hash lookup.
                glyph->next = font->lut[h];
                font->lut[h] = font->nglyphs - 1;
            }
            glyph->index = g;
            glyph->x0 = static_cast<int16_t>(gx);
            glyph->y0 = static_cast<int16_t>(gy);
            glyph->x1 = static_cast<int16_t>(glyph->x0 + gw);
            glyph->y1 = static_cast<int16_t>(glyph->y0 + gh);
            glyph->xadv = static_cast<int16_t>(scale * advance * 10.0f);
            glyph->xoff = static_cast<int16_t>(x0 - pad);
            glyph->yoff = static_cast<int16_t>(y0 - pad);

            if (bitmapOption == FonsGlyphBitmapOptional)
                return glyph;

            // Rasterize
            uint8_t*
                dst = &stash->tex_data[(glyph->x0 + pad) + (glyph->y0 + pad) * stash->params.width];
            fons_tt_render_glyph_bitmap(&render_font->font, dst, gw - pad * 2, gh - pad * 2,
                                        stash->params.width, scale, scale, g);

            // Make sure there is one pixel empty border.
            dst = &stash->tex_data[glyph->x0 + glyph->y0 * stash->params.width];
            for (int32_t y = 0; y < gh; y++)
            {
                dst[y * stash->params.width] = 0;
                dst[gw - 1 + y * stash->params.width] = 0;
            }
            for (int32_t x = 0; x < gw; x++)
            {
                dst[x] = 0;
                dst[x + (gh - 1) * stash->params.width] = 0;
            }

            // Debug code to color the glyph background
            /*	uint8_t* fdst = &stash->texData[glyph->x0 + glyph->y0 * stash->params.width];
                for (y = 0; y < gh; y++) {
                    for (x = 0; x < gw; x++) {
                        int32_t a = (int32_t)fdst[x+y*stash->params.width] + 20;
                        if (a > 255) a = 255;
                        fdst[x+y*stash->params.width] = a;
                    }
                }*/

            // Blur
            if (iblur > 0)
            {
                stash->nscratch = 0;
                uint8_t* bdst = &stash->tex_data[glyph->x0 + glyph->y0 * stash->params.width];
                fons_blur(stash, bdst, gw, gh, stash->params.width, iblur);
            }

            stash->dirty_rect[0] = fons_mini(stash->dirty_rect[0], glyph->x0);
            stash->dirty_rect[1] = fons_mini(stash->dirty_rect[1], glyph->y0);
            stash->dirty_rect[2] = fons_maxi(stash->dirty_rect[2], glyph->x1);
            stash->dirty_rect[3] = fons_maxi(stash->dirty_rect[3], glyph->y1);

            return glyph;
        }

        void fons_getQuad(const FONScontext* stash, FONSfont* font, const int32_t prevGlyphIndex,
                          const FONSglyph* glyph, const float scale, const float spacing, float* x,
                          const float* y, FONSquad* q)
        {
            float rx, ry;

            if (prevGlyphIndex != -1)
            {
                const float adv = fons_tt_get_glyph_kern_advance(&font->font, prevGlyphIndex,
                                                                 glyph->index) *
                                  scale;
                *x += static_cast<int32_t>(adv + spacing + 0.5f);
            }

            // Each glyph has 2px border to allow good interpolation,
            // one pixel to prevent leaking, and one to allow good interpolation for rendering.
            // Inset the texture region by one pixel for correct interpolation.
            const float xoff = static_cast<int16_t>(glyph->xoff + 1);
            const float yoff = static_cast<int16_t>(glyph->yoff + 1);
            const float x0 = static_cast<float>(glyph->x0 + 1);
            const float y0 = static_cast<float>(glyph->y0 + 1);
            const float x1 = static_cast<float>(glyph->x1 - 1);
            const float y1 = static_cast<float>(glyph->y1 - 1);

            if (stash->params.flags & FonsZeroTopleft)
            {
                rx = floorf(*x + xoff);
                ry = floorf(*y + yoff);

                q->x0 = rx;
                q->y0 = ry;
                q->x1 = rx + x1 - x0;
                q->y1 = ry + y1 - y0;

                q->s0 = x0 * stash->itw;
                q->t0 = y0 * stash->ith;
                q->s1 = x1 * stash->itw;
                q->t1 = y1 * stash->ith;
            }
            else
            {
                rx = floorf(*x + xoff);
                ry = floorf(*y - yoff);

                q->x0 = rx;
                q->y0 = ry;
                q->x1 = rx + x1 - x0;
                q->y1 = ry - y1 + y0;

                q->s0 = x0 * stash->itw;
                q->t0 = y0 * stash->ith;
                q->s1 = x1 * stash->itw;
                q->t1 = y1 * stash->ith;
            }

            *x += static_cast<int32_t>(glyph->xadv / 10.0f + 0.5f);
        }

        void fons_flush(FONScontext* stash)
        {
            // Flush texture
            if (stash->dirty_rect[0] < stash->dirty_rect[2] &&
                stash->dirty_rect[1] < stash->dirty_rect[3])
            {
                if (stash->params.render_update != nullptr)
                    stash->params.render_update(stash->params.user_ptr, stash->dirty_rect,
                                                stash->tex_data);
                // Reset dirty rect
                stash->dirty_rect[0] = stash->params.width;
                stash->dirty_rect[1] = stash->params.height;
                stash->dirty_rect[2] = 0;
                stash->dirty_rect[3] = 0;
            }

            // Flush triangles
            if (stash->nverts > 0)
            {
                if (stash->params.render_draw != nullptr)
                    stash->params.render_draw(stash->params.user_ptr, stash->verts, stash->tcoords,
                                              stash->colors, stash->nverts);
                stash->nverts = 0;
            }
        }

        __inline void fons_vertex(FONScontext* stash, const float x, const float y, const float s,
                                  const float t, const uint32_t c)
        {
            stash->verts[stash->nverts * 2 + 0] = x;
            stash->verts[stash->nverts * 2 + 1] = y;
            stash->tcoords[stash->nverts * 2 + 0] = s;
            stash->tcoords[stash->nverts * 2 + 1] = t;
            stash->colors[stash->nverts] = c;
            stash->nverts++;
        }

        float fons_get_vert_align(const FONScontext* stash, const FONSfont* font,
                                  const int32_t align, const int16_t isize)
        {
            if (stash->params.flags & FonsZeroTopleft)
            {
                if (align & FonsAlignTop)
                    return font->ascender * static_cast<float>(isize) / 10.0f;
                if (align & FonsAlignMiddle)
                    return (font->ascender + font->descender) / 2.0f * static_cast<float>(isize) /
                           10.0f;
                if (align & FonsAlignBaseline)
                    return 0.0f;
                if (align & FonsAlignBottom)
                    return font->descender * static_cast<float>(isize) / 10.0f;
            }
            else
            {
                if (align & FonsAlignTop)
                    return -font->ascender * static_cast<float>(isize) / 10.0f;
                if (align & FonsAlignMiddle)
                    return -(font->ascender + font->descender) / 2.0f * static_cast<float>(isize) /
                           10.0f;
                if (align & FonsAlignBaseline)
                    return 0.0f;
                if (align & FonsAlignBottom)
                    return -font->descender * static_cast<float>(isize) / 10.0f;
            }
            return 0.0;
        }

        void fons_free_font(FONSfont* font)
        {
            if (font == nullptr)
                return;
            if (font->glyphs)
                free(font->glyphs);
            if (font->free_data && font->data)
                free(font->data);
            free(font);
        }

        int32_t fons_alloc_font(FONScontext* stash)
        {
            if (stash->nfonts + 1 > stash->cfonts)
            {
                stash->cfonts = stash->cfonts == 0 ? 8 : stash->cfonts * 2;
                stash->fonts = static_cast<FONSfont**>(
                    std::realloc(stash->fonts, sizeof(FONSfont*) * stash->cfonts));

                if (stash->fonts == nullptr)
                    return -1;
            }

            const auto font = static_cast<FONSfont*>(std::malloc(sizeof(FONSfont)));
            if (font != nullptr)
            {
                memset(font, 0, sizeof(FONSfont));

                font->glyphs = static_cast<FONSglyph*>(
                    std::malloc(sizeof(FONSglyph) * FONS_INIT_GLYPHS));
                if (font->glyphs != nullptr)
                {
                    font->cglyphs = FONS_INIT_GLYPHS;
                    font->nglyphs = 0;

                    stash->fonts[stash->nfonts++] = font;
                    return stash->nfonts - 1;
                }
            }

            fons_free_font(font);
            return FONS_INVALID;
        }
    }

    FONScontext* fons_create_internal(const FONSparams* params)
    {
        // Allocate memory for the font stash.
        const auto stash = static_cast<FONScontext*>(std::malloc(sizeof(FONScontext)));
        if (stash != nullptr)
        {
            memset(stash, 0, sizeof(FONScontext));

            stash->params = *params;

            // Allocate scratch buffer.
            stash->scratch = static_cast<uint8_t*>(std::malloc(FONS_SCRATCH_BUF_SIZE));
            if (stash->scratch != nullptr)
            {
                // Initialize implementation library
                if (fons_tt_init(stash) || stash->params.render_create != nullptr ||
                    stash->params.render_create(stash->params.user_ptr, stash->params.width,
                                                stash->params.height) != 0)
                {
                    stash->atlas = fons_alloc_atlas(stash->params.width, stash->params.height,
                                                    FONS_INIT_ATLAS_NODES);
                    if (stash->atlas != nullptr)
                    {
                        // Allocate space for fonts.
                        stash->fonts = static_cast<FONSfont**>(
                            std::malloc(sizeof(FONSfont*) * FONS_INIT_FONTS));
                        if (stash->fonts != nullptr)
                        {
                            memset(stash->fonts, 0, sizeof(FONSfont*) * FONS_INIT_FONTS);
                            stash->cfonts = FONS_INIT_FONTS;
                            stash->nfonts = 0;

                            // Create texture for the cache.
                            stash->itw = 1.0f / static_cast<float>(stash->params.width);
                            stash->ith = 1.0f / static_cast<float>(stash->params.height);
                            stash->tex_data = static_cast<uint8_t*>(
                                std::malloc(stash->params.width * stash->params.height));
                            if (stash->tex_data != nullptr)
                            {
                                memset(stash->tex_data, 0,
                                       stash->params.width * stash->params.height);

                                stash->dirty_rect[0] = stash->params.width;
                                stash->dirty_rect[1] = stash->params.height;
                                stash->dirty_rect[2] = 0;
                                stash->dirty_rect[3] = 0;

                                // Add white rect at 0,0 for debug drawing.
                                fons_add_white_rect(stash, 2, 2);

                                fons_push_state(stash);
                                fons_clear_state(stash);

                                return stash;
                            }
                        }
                    }
                }
            }
        }
        fons_delete_internal(stash);
        return nullptr;
    }

    int32_t fons_add_fallback_font(const FONScontext* stash, const int32_t base,
                                   const int32_t fallback)
    {
        FONSfont* baseFont = stash->fonts[base];
        if (baseFont->nfallbacks < FONS_MAX_FALLBACKS)
        {
            baseFont->fallbacks[baseFont->nfallbacks++] = fallback;
            return 1;
        }
        return 0;
    }

    void fons_reset_fallback_font(const FONScontext* stash, const int32_t base)
    {
        FONSfont* base_font = stash->fonts[base];
        base_font->nfallbacks = 0;
        base_font->nglyphs = 0;
        for (int32_t& i : base_font->lut)
            i = -1;
    }

    void fons_set_size(FONScontext* stash, const float size)
    {
        fons_get_state(stash)->size = size;
    }

    void fons_set_color(FONScontext* stash, const uint32_t color)
    {
        fons_get_state(stash)->color = color;
    }

    void fons_set_spacing(FONScontext* stash, const float spacing)
    {
        fons_get_state(stash)->spacing = spacing;
    }

    void fons_set_blur(FONScontext* stash, const float blur)
    {
        fons_get_state(stash)->blur = blur;
    }

    void fons_set_align(FONScontext* stash, const int32_t align)
    {
        fons_get_state(stash)->align = align;
    }

    void fons_set_font(FONScontext* stash, const int32_t font)
    {
        fons_get_state(stash)->font = font;
    }

    void fons_push_state(FONScontext* stash)
    {
        if (stash->nstates >= FONS_MAX_STATES)
        {
            if (stash->handle_error)
                stash->handle_error(stash->error_uptr, FonsStatesOverflow, 0);
            return;
        }
        if (stash->nstates > 0)
            memcpy(&stash->states[stash->nstates], &stash->states[stash->nstates - 1],
                   sizeof(FONSstate));
        stash->nstates++;
    }

    void fons_pop_state(FONScontext* stash)
    {
        if (stash->nstates <= 1)
        {
            if (stash->handle_error)
                stash->handle_error(stash->error_uptr, FonsStatesUnderflow, 0);
            return;
        }
        stash->nstates--;
    }

    void fons_clear_state(FONScontext* stash)
    {
        FONSstate* state = fons_get_state(stash);
        state->size = 12.0f;
        state->color = 0xffffffff;
        state->font = 0;
        state->blur = 0;
        state->spacing = 0;
        state->align = FonsAlignLeft | FonsAlignBaseline;
    }

    int32_t fons_add_font(FONScontext* stash, const char* name, const char* path,
                          const int32_t font_index)
    {
        uint8_t* data = nullptr;
        int32_t status = -1;

        // Read in the font data.
        FILE* fp = std::fopen(path, "rb");
        if (fp != nullptr)
        {
            status = std::fseek(fp, 0, SEEK_END);
            const int32_t data_size = static_cast<int32_t>(ftell(fp));
            status = std::fseek(fp, 0, SEEK_SET);
            data = static_cast<uint8_t*>(std::malloc(data_size));
            if (data != nullptr)
            {
                const size_t readed = std::fread(data, 1, data_size, fp);
                status = std::fclose(fp);
                fp = nullptr;
                if (readed == static_cast<size_t>(data_size))
                    return fons_add_font_mem(stash, name, data, data_size, 1, font_index);
            }
        }

        if (data != nullptr)
            std::free(data);
        if (fp != nullptr)
            status = std::fclose(fp);

        return FONS_INVALID;
    }

    int32_t fons_add_font_mem(FONScontext* stash, const char* name, uint8_t* data,
                              const int32_t data_size, const int32_t free_data,
                              const int32_t font_index)
    {
        int32_t ascent, descent, line_gap;

        const int32_t idx = fons_alloc_font(stash);
        if (idx == FONS_INVALID)
            return FONS_INVALID;

        FONSfont* font = stash->fonts[idx];

        std::strncpy(font->name, name, sizeof(font->name));
        font->name[sizeof(font->name) - 1] = '\0';

        // Init hash lookup.
        for (int32_t& i : font->lut)
            i = -1;

        // Read in the font data.
        font->data_size = data_size;
        font->data = data;
        font->free_data = static_cast<uint8_t>(free_data);

        // Init font
        stash->nscratch = 0;
        if (fons_tt_load_font(stash, &font->font, data, data_size, font_index))
        {
            // Store normalized line height. The real line height is got
            // by multiplying the lineh by font size.
            fons_tt_get_font_v_metrics(&font->font, &ascent, &descent, &line_gap);
            ascent += line_gap;
            const int32_t fh = ascent - descent;
            font->ascender = static_cast<float>(ascent) / static_cast<float>(fh);
            font->descender = static_cast<float>(descent) / static_cast<float>(fh);
            font->lineh = font->ascender - font->descender;

            return idx;
        }

        fons_free_font(font);
        stash->nfonts--;
        return FONS_INVALID;
    }

    int32_t fons_get_font_by_name(const FONScontext* s, const char* name)
    {
        for (int32_t i = 0; i < s->nfonts; i++)
            if (std::strcmp(s->fonts[i]->name, name) == 0)
                return i;

        return FONS_INVALID;
    }

    float fons_draw_text(FONScontext* stash, float x, float y, const char* str, const char* end)
    {
        const FONSstate* state = fons_get_state(stash);
        uint32_t codepoint;
        uint32_t utf8_state = 0;
        FONSquad q;
        int32_t prevGlyphIndex = -1;
        const int16_t isize = static_cast<int16_t>(state->size * 10.0f);
        const int16_t iblur = static_cast<int16_t>(state->blur);
        float width;

        if (stash == nullptr)
            return x;
        if (state->font < 0 || state->font >= stash->nfonts)
            return x;
        FONSfont* font = stash->fonts[state->font];
        if (font->data == nullptr)
            return x;

        const float scale = fons_tt_get_pixel_height_scale(&font->font,
                                                           static_cast<float>(isize) / 10.0f);

        if (end == nullptr)
            end = str + std::strlen(str);

        // Align horizontally
        if (state->align & FonsAlignLeft)
        {
            // empty
        }
        else if (state->align & FonsAlignRight)
        {
            width = fons_text_bounds(stash, x, y, str, end, nullptr);
            x -= width;
        }
        else if (state->align & FonsAlignCenter)
        {
            width = fons_text_bounds(stash, x, y, str, end, nullptr);
            x -= width * 0.5f;
        }
        // Align vertically.
        y += fons_get_vert_align(stash, font, state->align, isize);

        for (; str != end; ++str)
        {
            if (fons_decutf8(&utf8_state, &codepoint, *reinterpret_cast<const uint8_t*>(str)))
                continue;

            const FONSglyph* glyph = fons_get_glyph(stash, font, codepoint, isize, iblur,
                                                    FonsGlyphBitmapRequired);
            if (glyph != nullptr)
            {
                fons_getQuad(stash, font, prevGlyphIndex, glyph, scale, state->spacing, &x, &y, &q);

                if (stash->nverts + 6 > FONS_VERTEX_COUNT)
                    fons_flush(stash);

                fons_vertex(stash, q.x0, q.y0, q.s0, q.t0, state->color);
                fons_vertex(stash, q.x1, q.y1, q.s1, q.t1, state->color);
                fons_vertex(stash, q.x1, q.y0, q.s1, q.t0, state->color);

                fons_vertex(stash, q.x0, q.y0, q.s0, q.t0, state->color);
                fons_vertex(stash, q.x0, q.y1, q.s0, q.t1, state->color);
                fons_vertex(stash, q.x1, q.y1, q.s1, q.t1, state->color);
            }

            prevGlyphIndex = glyph != nullptr ? glyph->index : -1;
        }

        fons_flush(stash);
        return x;
    }

    int32_t fons_text_iter_init(FONScontext* stash, FONStextIter* iter, float x, float y,
                                const char* str, const char* end, const int32_t bitmap_option)
    {
        const FONSstate* state = fons_get_state(stash);
        float width;

        memset(iter, 0, sizeof(*iter));

        if (stash == nullptr)
            return 0;
        if (state->font < 0 || state->font >= stash->nfonts)
            return 0;
        iter->font = stash->fonts[state->font];
        if (iter->font->data == nullptr)
            return 0;

        iter->isize = static_cast<int16_t>(state->size * 10.0f);
        iter->iblur = static_cast<int16_t>(state->blur);
        iter->scale = fons_tt_get_pixel_height_scale(&iter->font->font,
                                                     static_cast<float>(iter->isize) / 10.0f);

        // Align horizontally
        if (state->align & FonsAlignLeft)
        {
            // empty
        }
        else if (state->align & FonsAlignRight)
        {
            width = fons_text_bounds(stash, x, y, str, end, nullptr);
            x -= width;
        }
        else if (state->align & FonsAlignCenter)
        {
            width = fons_text_bounds(stash, x, y, str, end, nullptr);
            x -= width * 0.5f;
        }
        // Align vertically.
        y += fons_get_vert_align(stash, iter->font, state->align, iter->isize);

        if (end == nullptr)
            end = str + strlen(str);

        iter->x = iter->nextx = x;
        iter->y = iter->nexty = y;
        iter->spacing = state->spacing;
        iter->str = str;
        iter->next = str;
        iter->end = end;
        iter->codepoint = 0;
        iter->prev_glyph_index = -1;
        iter->bitmap_option = bitmap_option;

        return 1;
    }

    int32_t fons_text_iter_next(FONScontext* stash, FONStextIter* iter, FONSquad* quad)
    {
        const char* str = iter->next;
        iter->str = iter->next;

        if (str == iter->end)
            return 0;

        for (; str != iter->end; str++)
        {
            if (fons_decutf8(&iter->utf8_state, &iter->codepoint, *(const uint8_t*)str))
                continue;
            str++;
            // Get glyph and quad
            iter->x = iter->nextx;
            iter->y = iter->nexty;
            const FONSglyph* glyph = fons_get_glyph(stash, iter->font, iter->codepoint, iter->isize,
                                                    iter->iblur, iter->bitmap_option);
            // If the iterator was initialized with FONS_GLYPH_BITMAP_OPTIONAL, then the UV
            // coordinates of the quad will be invalid.
            if (glyph != nullptr)
                fons_getQuad(stash, iter->font, iter->prev_glyph_index, glyph, iter->scale,
                             iter->spacing, &iter->nextx, &iter->nexty, quad);
            iter->prev_glyph_index = glyph != nullptr ? glyph->index : -1;
            break;
        }
        iter->next = str;

        return 1;
    }

    void fons_draw_debug(FONScontext* stash, const float x, const float y)
    {
        const float w = static_cast<float>(stash->params.width);
        const float h = static_cast<float>(stash->params.height);
        const float u = w == 0 ? 0 : (1.0f / w);
        const float v = h == 0 ? 0 : (1.0f / h);

        if (stash->nverts + 6 + 6 > FONS_VERTEX_COUNT)
            fons_flush(stash);

        // Draw background
        fons_vertex(stash, x + 0, y + 0, u, v, 0x0fffffff);
        fons_vertex(stash, x + w, y + h, u, v, 0x0fffffff);
        fons_vertex(stash, x + w, y + 0, u, v, 0x0fffffff);

        fons_vertex(stash, x + 0, y + 0, u, v, 0x0fffffff);
        fons_vertex(stash, x + 0, y + h, u, v, 0x0fffffff);
        fons_vertex(stash, x + w, y + h, u, v, 0x0fffffff);

        // Draw texture
        fons_vertex(stash, x + 0, y + 0, 0, 0, 0xffffffff);
        fons_vertex(stash, x + w, y + h, 1, 1, 0xffffffff);
        fons_vertex(stash, x + w, y + 0, 1, 0, 0xffffffff);

        fons_vertex(stash, x + 0, y + 0, 0, 0, 0xffffffff);
        fons_vertex(stash, x + 0, y + h, 0, 1, 0xffffffff);
        fons_vertex(stash, x + w, y + h, 1, 1, 0xffffffff);

        // Drawbug draw atlas
        for (int32_t i = 0; i < stash->atlas->nnodes; i++)
        {
            const FONSatlasNode* n = &stash->atlas->nodes[i];

            if (stash->nverts + 6 > FONS_VERTEX_COUNT)
                fons_flush(stash);

            const float nx = static_cast<float>(n->x);
            const float ny = static_cast<float>(n->y);
            const float nw = static_cast<float>(n->width);

            fons_vertex(stash, x + nx + 0, y + ny + 0, u, v, 0xc00000ff);
            fons_vertex(stash, x + nx + nw, y + ny + 1, u, v, 0xc00000ff);
            fons_vertex(stash, x + nx + nw, y + ny + 0, u, v, 0xc00000ff);

            fons_vertex(stash, x + nx + 0, y + ny + 0, u, v, 0xc00000ff);
            fons_vertex(stash, x + nx + 0, y + ny + 1, u, v, 0xc00000ff);
            fons_vertex(stash, x + nx + nw, y + ny + 1, u, v, 0xc00000ff);
        }

        fons_flush(stash);
    }

    float fons_text_bounds(FONScontext* stash, float x, float y, const char* str, const char* end,
                           float* bounds)
    {
        const FONSstate* state = fons_get_state(stash);
        uint32_t codepoint;
        uint32_t utf8_state = 0;
        FONSquad q;
        int32_t prev_glyph_index = -1;
        const int16_t isize = static_cast<int16_t>(state->size * 10.0f);
        const int16_t iblur = static_cast<int16_t>(state->blur);
        float maxx, maxy;

        if (stash == nullptr)
            return 0;
        if (state->font < 0 || state->font >= stash->nfonts)
            return 0;
        FONSfont* font = stash->fonts[state->font];
        if (font->data == nullptr)
            return 0;

        const float scale = fons_tt_get_pixel_height_scale(&font->font,
                                                           static_cast<float>(isize) / 10.0f);

        // Align vertically.
        y += fons_get_vert_align(stash, font, state->align, isize);

        float minx = maxx = x;
        float miny = maxy = y;
        const float startx = x;

        if (end == nullptr)
            end = str + strlen(str);

        for (; str != end; ++str)
        {
            if (fons_decutf8(&utf8_state, &codepoint, *reinterpret_cast<const uint8_t*>(str)))
                continue;

            const FONSglyph* glyph = fons_get_glyph(stash, font, codepoint, isize, iblur,
                                                    FonsGlyphBitmapOptional);
            if (glyph != nullptr)
            {
                fons_getQuad(stash, font, prev_glyph_index, glyph, scale, state->spacing, &x, &y,
                             &q);

                if (q.x0 < minx)
                    minx = q.x0;
                if (q.x1 > maxx)
                    maxx = q.x1;

                if (stash->params.flags & FonsZeroTopleft)
                {
                    if (q.y0 < miny)
                        miny = q.y0;
                    if (q.y1 > maxy)
                        maxy = q.y1;
                }
                else
                {
                    if (q.y1 < miny)
                        miny = q.y1;
                    if (q.y0 > maxy)
                        maxy = q.y0;
                }
            }

            prev_glyph_index = glyph != nullptr ? glyph->index : -1;
        }

        const float advance = x - startx;

        // Align horizontally
        if (state->align & FonsAlignLeft)
        {
            // empty
        }
        else if (state->align & FonsAlignRight)
        {
            minx -= advance;
            maxx -= advance;
        }
        else if (state->align & FonsAlignCenter)
        {
            minx -= advance * 0.5f;
            maxx -= advance * 0.5f;
        }

        if (bounds)
        {
            bounds[0] = minx;
            bounds[1] = miny;
            bounds[2] = maxx;
            bounds[3] = maxy;
        }

        return advance;
    }

    void fons_vert_metrics(FONScontext* stash, float* ascender, float* descender, float* lineh)
    {
        const FONSstate* state = fons_get_state(stash);

        if (stash == nullptr)
            return;
        if (state->font < 0 || state->font >= stash->nfonts)
            return;
        const FONSfont* font = stash->fonts[state->font];
        const int16_t isize = static_cast<int16_t>(state->size * 10.0f);
        if (font->data == nullptr)
            return;

        if (ascender)
            *ascender = font->ascender * isize / 10.0f;
        if (descender)
            *descender = font->descender * isize / 10.0f;
        if (lineh)
            *lineh = font->lineh * isize / 10.0f;
    }

    void fons_line_bounds(FONScontext* stash, float y, float* miny, float* maxy)
    {
        const FONSstate* state = fons_get_state(stash);

        if (stash == nullptr)
            return;
        if (state->font < 0 || state->font >= stash->nfonts)
            return;

        const FONSfont* font = stash->fonts[state->font];
        const int16_t isize = static_cast<int16_t>(state->size * 10.0f);
        if (font->data == nullptr)
            return;

        y += fons_get_vert_align(stash, font, state->align, isize);

        if (stash->params.flags & FonsZeroTopleft)
        {
            *miny = y - font->ascender * static_cast<float>(isize) / 10.0f;
            *maxy = *miny + font->lineh * static_cast<float>(isize) / 10.0f;
        }
        else
        {
            *maxy = y + font->descender * static_cast<float>(isize) / 10.0f;
            *miny = *maxy - font->lineh * static_cast<float>(isize) / 10.0f;
        }
    }

    const uint8_t* fons_get_texture_data(const FONScontext* stash, int32_t* width, int32_t* height)
    {
        if (width != nullptr)
            *width = stash->params.width;
        if (height != nullptr)
            *height = stash->params.height;

        return stash->tex_data;
    }

    int32_t fons_validate_texture(FONScontext* stash, int32_t* dirty)
    {
        if (stash->dirty_rect[0] < stash->dirty_rect[2] &&
            stash->dirty_rect[1] < stash->dirty_rect[3])
        {
            dirty[0] = stash->dirty_rect[0];
            dirty[1] = stash->dirty_rect[1];
            dirty[2] = stash->dirty_rect[2];
            dirty[3] = stash->dirty_rect[3];
            // Reset dirty rect
            stash->dirty_rect[0] = stash->params.width;
            stash->dirty_rect[1] = stash->params.height;
            stash->dirty_rect[2] = 0;
            stash->dirty_rect[3] = 0;
            return 1;
        }
        return 0;
    }

    void fons_delete_internal(FONScontext* stash)
    {
        if (stash == nullptr)
            return;

        if (stash->params.render_delete)
            stash->params.render_delete(stash->params.user_ptr);

        for (int32_t i = 0; i < stash->nfonts; ++i)
            fons_free_font(stash->fonts[i]);

        if (stash->atlas)
            fons_delete_atlas(stash->atlas);
        if (stash->fonts)
            free(stash->fonts);
        if (stash->tex_data)
            free(stash->tex_data);
        if (stash->scratch)
            free(stash->scratch);

        fons_tt_done(stash);
        free(stash);
    }

    void fons_set_error_callback(
        FONScontext* stash, void (*callback)(void* uptr, int32_t error, int32_t val), void* uptr)
    {
        if (stash == nullptr)
            return;
        stash->handle_error = callback;
        stash->error_uptr = uptr;
    }

    void fons_get_atlas_size(const FONScontext* stash, int32_t* width, int32_t* height)
    {
        if (stash == nullptr)
            return;

        *width = stash->params.width;
        *height = stash->params.height;
    }

    int32_t fons_expand_atlas(FONScontext* stash, int32_t width, int32_t height)
    {
        int32_t i;
        int32_t maxy = 0;
        if (stash == nullptr)
            return 0;

        width = fons_maxi(width, stash->params.width);
        height = fons_maxi(height, stash->params.height);

        if (width == stash->params.width && height == stash->params.height)
            return 1;

        // Flush pending glyphs.
        fons_flush(stash);

        // Create new texture
        if (stash->params.render_resize != nullptr)
        {
            if (stash->params.render_resize(stash->params.user_ptr, width, height) == 0)
                return 0;
        }
        // Copy old texture data over.
        const auto data = static_cast<uint8_t*>(std::malloc(width * height));
        if (data == nullptr)
            return 0;

        for (i = 0; i < stash->params.height; i++)
        {
            uint8_t* dst = &data[i * width];
            const uint8_t* src = &stash->tex_data[i * stash->params.width];
            memcpy(dst, src, stash->params.width);
            if (width > stash->params.width)
                memset(dst + stash->params.width, 0, width - stash->params.width);
        }
        if (height > stash->params.height)
            memset(&data[stash->params.height * width], 0, (height - stash->params.height) * width);

        free(stash->tex_data);
        stash->tex_data = data;

        // Increase atlas size
        fons_atlas_expand(stash->atlas, width, height);

        // Add existing data as dirty.
        for (i = 0; i < stash->atlas->nnodes; i++)
            maxy = fons_maxi(maxy, stash->atlas->nodes[i].y);
        stash->dirty_rect[0] = 0;
        stash->dirty_rect[1] = 0;
        stash->dirty_rect[2] = stash->params.width;
        stash->dirty_rect[3] = maxy;

        stash->params.width = width;
        stash->params.height = height;
        stash->itw = 1.0f / static_cast<float>(stash->params.width);
        stash->ith = 1.0f / static_cast<float>(stash->params.height);

        return 1;
    }

    int32_t fons_reset_atlas(FONScontext* stash, const int32_t width, const int32_t height)
    {
        if (stash == nullptr)
            return 0;

        // Flush pending glyphs.
        fons_flush(stash);

        // Create new texture
        if (stash->params.render_resize != nullptr)
        {
            if (stash->params.render_resize(stash->params.user_ptr, width, height) == 0)
                return 0;
        }

        // Reset atlas
        fons_atlas_reset(stash->atlas, width, height);

        // Clear texture data.
        stash->tex_data = static_cast<uint8_t*>(realloc(stash->tex_data, width * height));
        if (stash->tex_data == nullptr)
            return 0;
        memset(stash->tex_data, 0, width * height);

        // Reset dirty rect
        stash->dirty_rect[0] = width;
        stash->dirty_rect[1] = height;
        stash->dirty_rect[2] = 0;
        stash->dirty_rect[3] = 0;

        // Reset cached glyphs
        for (int32_t i = 0; i < stash->nfonts; i++)
        {
            FONSfont* font = stash->fonts[i];
            font->nglyphs = 0;
            for (int32_t& j : font->lut)
                j = -1;
        }

        stash->params.width = width;
        stash->params.height = height;
        stash->itw = 1.0f / static_cast<float>(stash->params.width);
        stash->ith = 1.0f / static_cast<float>(stash->params.height);

        // Add white rect at 0,0 for debug drawing.
        fons_add_white_rect(stash, 2, 2);

        return 1;
    }
}
