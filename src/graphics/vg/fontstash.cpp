#include <cstdint>
#include <cstdio>

#include "graphics/stb/stb_truetype.hpp"
#include "graphics/vg/fontstash.hpp"
#include "nanovg.hpp"
#include "utils/conversions.hpp"

namespace rl::nvg {
    using namespace rl::stb;

    constexpr i32 FONS_UTF8_ACCEPT{ 0 };

    namespace {
        u32 fons_hashint(u32 a)
        {
            a += ~(a << 15);
            a ^= (a >> 10);
            a += (a << 3);
            a ^= (a >> 6);
            a += ~(a << 11);
            a ^= (a >> 16);
            return a;
        }

        i32 fons_mini(const i32 a, const i32 b)
        {
            return a < b ? a : b;
        }

        i32 fons_maxi(const i32 a, const i32 b)
        {
            return a > b ? a : b;
        }

        i32 fons_tt_init(FONScontext*)
        {
            return 1;
        }

        i32 fons_tt_done(FONScontext*)
        {
            return 1;
        }

        i32 fons_tt_load_font(FONScontext* context, FONSttFontImpl* font, const u8* data,
                              i32 /*data_size*/, const i32 font_index)
        {
            i32 stb_error;
            font->font.userdata = context;
            const i32 offset = stbtt_get_font_offset_for_index(data, font_index);

            if (offset == -1)
                stb_error = 0;
            else
                stb_error = stbtt_init_font(&font->font, data, offset);

            return stb_error;
        }

        void fons_tt_get_font_v_metrics(const FONSttFontImpl* font, i32* ascent, i32* descent,
                                        i32* line_gap)
        {
            stbtt_GetFontVMetrics(&font->font, ascent, descent, line_gap);
        }

        f32 fons_tt_get_pixel_height_scale(const FONSttFontImpl* font, const f32 size)
        {
            return stbtt_ScaleForMappingEmToPixels(&font->font, size);
        }

        i32 fons_tt_get_glyph_index(const FONSttFontImpl* font, const i32 codepoint)
        {
            return stbtt_find_glyph_index(&font->font, codepoint);
        }

        i32 fons_tt_build_glyph_bitmap(const FONSttFontImpl* font, const i32 glyph, f32 /*size*/,
                                       const f32 scale, i32* advance, i32* lsb, i32* x0, i32* y0,
                                       i32* x1, i32* y1)
        {
            stbtt_GetGlyphHMetrics(&font->font, glyph, advance, lsb);
            stbtt_GetGlyphBitmapBox(&font->font, glyph, scale, scale, x0, y0, x1, y1);
            return 1;
        }

        void fons_tt_render_glyph_bitmap(
            const FONSttFontImpl* font, u8* output, const i32 out_width, const i32 out_height,
            const i32 out_stride, const f32 scale_x, const f32 scale_y, const i32 glyph)
        {
            stbtt_MakeGlyphBitmap(&font->font, output, out_width, out_height, out_stride, scale_x,
                                  scale_y, glyph);
        }

        i32 fons_tt_get_glyph_kern_advance(const FONSttFontImpl* font, const i32 glyph1,
                                           const i32 glyph2)
        {
            return stbtt_GetGlyphKernAdvance(&font->font, glyph1, glyph2);
        }

        u32 fons_decutf8(u32* state, u32* codep, const u32 byte)
        {
            constexpr static u8 utf8d[] = {
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

            const u32 type = utf8d[byte];

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

        FONSatlas* fons_alloc_atlas(const i32 w, const i32 h, const i32 nnodes)
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

        i32 fons_atlas_insert_node(FONSatlas* atlas, const i32 idx, const i32 x, const i32 y,
                                   const i32 w)
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

            for (i32 i = atlas->nnodes; i > idx; i--)
                atlas->nodes[i] = atlas->nodes[i - 1];

            atlas->nodes[idx].x = static_cast<int16_t>(x);
            atlas->nodes[idx].y = static_cast<int16_t>(y);
            atlas->nodes[idx].width = static_cast<int16_t>(w);
            atlas->nnodes++;

            return 1;
        }

        void fons_atlas_remove_node(FONSatlas* atlas, const i32 idx)
        {
            if (atlas->nnodes == 0)
                return;

            for (i32 i = idx; i < atlas->nnodes - 1; i++)
                atlas->nodes[i] = atlas->nodes[i + 1];

            atlas->nnodes--;
        }

        void fons_atlas_expand(FONSatlas* atlas, const i32 w, const i32 h)
        {
            // Insert node for empty space
            if (w > atlas->width)
                fons_atlas_insert_node(atlas, atlas->nnodes, atlas->width, 0, w - atlas->width);

            atlas->width = w;
            atlas->height = h;
        }

        void fons_atlas_reset(FONSatlas* atlas, const i32 w, const i32 h)
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

        i32 fons_atlas_add_skyline_level(FONSatlas* atlas, const i32 idx, const i32 x, const i32 y,
                                         const i32 w, const i32 h)
        {
            i32 i;

            // Insert new node
            if (fons_atlas_insert_node(atlas, idx, x, y + h, w) == 0)
                return 0;

            // Delete skyline segments that fall under the shadow of the new segment.
            for (i = idx + 1; i < atlas->nnodes; i++)
            {
                if (atlas->nodes[i].x < atlas->nodes[i - 1].x + atlas->nodes[i - 1].width)
                {
                    const i32 shrink = atlas->nodes[i - 1].x + atlas->nodes[i - 1].width -
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

        i32 fons_atlas_rect_fits(const FONSatlas* atlas, i32 i, const i32 w, const i32 h)
        {
            // Checks if there is enough space at the location of skyline span 'i',
            // and return the max height of all skyline spans under that at that location,
            // (think tetris block being dropped at that position). Or -1 if no space found.
            const i32 x = atlas->nodes[i].x;
            i32 y = atlas->nodes[i].y;
            if (x + w > atlas->width)
                return -1;

            i32 spaceLeft = w;
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

        i32 fons_atlas_add_rect(FONSatlas* atlas, const i32 rw, const i32 rh, i32* rx, i32* ry)
        {
            i32 besth = atlas->height, bestw = atlas->width, besti = -1;
            i32 bestx = -1, besty = -1;

            // Bottom left fit heuristic.
            for (i32 i = 0; i < atlas->nnodes; i++)
            {
                const i32 y = fons_atlas_rect_fits(atlas, i, rw, rh);
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

        void fons_add_white_rect(FONScontext* stash, const i32 w, const i32 h)
        {
            i32 gx, gy;
            if (fons_atlas_add_rect(stash->atlas, w, h, &gx, &gy) == 0)
                return;

            // Rasterize
            u8* dst = &stash->tex_data[gx + gy * stash->params.width];
            for (i32 y = 0; y < h; y++)
            {
                for (i32 x = 0; x < w; x++)
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

        void fons_blur_cols(u8* dst, const i32 w, const i32 h, const i32 dstStride, const i32 alpha)
        {
            i32 x;
            for (i32 y = 0; y < h; y++)
            {
                i32 z = 0;  // force zero border
                for (x = 1; x < w; x++)
                {
                    z += (alpha * ((static_cast<i32>(dst[x]) << ZPREC) - z)) >> APREC;
                    dst[x] = static_cast<u8>(z >> ZPREC);
                }
                dst[w - 1] = 0;  // force zero border
                z = 0;
                for (x = w - 2; x >= 0; x--)
                {
                    z += (alpha * ((static_cast<i32>(dst[x]) << ZPREC) - z)) >> APREC;
                    dst[x] = static_cast<u8>(z >> ZPREC);
                }
                dst[0] = 0;  // force zero border
                dst += dstStride;
            }
        }

        void fons_blur_rows(u8* dst, const i32 w, const i32 h, const i32 dstStride, const i32 alpha)
        {
            i32 y;
            for (i32 x = 0; x < w; x++)
            {
                i32 z = 0;  // force zero border
                for (y = dstStride; y < h * dstStride; y += dstStride)
                {
                    z += (alpha * ((static_cast<i32>(dst[y]) << ZPREC) - z)) >> APREC;
                    dst[y] = static_cast<u8>(z >> ZPREC);
                }
                dst[(h - 1) * dstStride] = 0;  // force zero border
                z = 0;
                for (y = (h - 2) * dstStride; y >= 0; y -= dstStride)
                {
                    z += (alpha * ((static_cast<i32>(dst[y]) << ZPREC) - z)) >> APREC;
                    dst[y] = static_cast<u8>(z >> ZPREC);
                }
                dst[0] = 0;  // force zero border
                dst++;
            }
        }

        void fons_blur(const FONScontext* stash, u8* dst, const i32 w, const i32 h,
                       const i32 dst_stride, const i32 blur)
        {
            (void)stash;

            if (blur < 1)
                return;
            // Calculate the alpha such that 90% of the kernel is within the radius. (Kernel extends
            // to infinity)
            const f32 sigma = static_cast<f32>(blur) * 0.57735f;  // 1 / sqrt(3)
            const i32 alpha = static_cast<i32>((1 << APREC) * (1.0f - expf(-2.3f / (sigma + 1.0f))));
            fons_blur_rows(dst, w, h, dst_stride, alpha);
            fons_blur_cols(dst, w, h, dst_stride, alpha);
            fons_blur_rows(dst, w, h, dst_stride, alpha);
            fons_blur_cols(dst, w, h, dst_stride, alpha);
            //	fons__blurrows(dst, w, h, dstStride, alpha);
            //	fons__blurcols(dst, w, h, dstStride, alpha);
        }

        FONSglyph* fons_alloc_glyph(FONSfont* font)
        {
            if (font->nglyphs + 1 > font->cglyphs)
            {
                font->cglyphs = font->cglyphs == 0 ? 8 : font->cglyphs * 2;
                font->glyphs = static_cast<FONSglyph*>(
                    std::realloc(font->glyphs, sizeof(FONSglyph) * font->cglyphs));
                if (font->glyphs == nullptr)
                    return nullptr;
            }
            font->nglyphs++;
            return &font->glyphs[font->nglyphs - 1];
        }

        FONSglyph* fons_get_glyph(FONScontext* stash, FONSfont* font, const u32 codepoint,
                                  const int16_t isize, int16_t iblur, const i32 bitmapOption)
        {
            i32 advance;
            i32 lsb;
            i32 x0;
            i32 y0;
            i32 x1;
            i32 y1;
            i32 gx;
            i32 gy;
            FONSglyph* glyph = nullptr;
            const f32 size = static_cast<f32>(isize / 10.0f);
            const FONSfont* render_font = font;

            if (isize < 2)
                return nullptr;
            if (iblur > 20)
                iblur = 20;
            const i32 pad = iblur + 2;

            // Reset allocator.
            stash->nscratch = 0;

            // Find code point and size.
            const u32 h = fons_hashint(codepoint) & (FONS_HASH_LUT_SIZE - 1);
            i32 i = font->lut[h];
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
            i32 g = fons_tt_get_glyph_index(&font->font, static_cast<i32>(codepoint));
            // Try to find the glyph in fallback fonts.
            if (g == 0)
            {
                for (i = 0; i < font->nfallbacks; ++i)
                {
                    const FONSfont* fallbackFont = stash->fonts[font->fallbacks[i]];
                    const i32 fallback_index = fons_tt_get_glyph_index(&fallbackFont->font,
                                                                       static_cast<i32>(codepoint));
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
            const f32 scale = fons_tt_get_pixel_height_scale(&render_font->font, size);
            fons_tt_build_glyph_bitmap(&render_font->font, g, size, scale, &advance, &lsb, &x0, &y0,
                                       &x1, &y1);
            const i32 gw = x1 - x0 + pad * 2;
            const i32 gh = y1 - y0 + pad * 2;

            // Determines the spot to draw glyph in the atlas.
            if (bitmapOption == FonsGlyphBitmapRequired)
            {
                // Find free spot for the rect in the atlas
                i32 added = fons_atlas_add_rect(stash->atlas, gw, gh, &gx, &gy);
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
            u8* dst = &stash->tex_data[(glyph->x0 + pad) + (glyph->y0 + pad) * stash->params.width];
            fons_tt_render_glyph_bitmap(&render_font->font, dst, gw - pad * 2, gh - pad * 2,
                                        stash->params.width, scale, scale, g);

            // Make sure there is one pixel empty border.
            dst = &stash->tex_data[glyph->x0 + glyph->y0 * stash->params.width];
            for (i32 y = 0; y < gh; y++)
            {
                dst[y * stash->params.width] = 0;
                dst[gw - 1 + y * stash->params.width] = 0;
            }
            for (i32 x = 0; x < gw; x++)
            {
                dst[x] = 0;
                dst[x + (gh - 1) * stash->params.width] = 0;
            }

            // Debug code to color the glyph background
            /*	u8* fdst = &stash->texData[glyph->x0 + glyph->y0 * stash->params.width];
                for (y = 0; y < gh; y++) {
                    for (x = 0; x < gw; x++) {
                        i32 a = (i32)fdst[x+y*stash->params.width] + 20;
                        if (a > 255) a = 255;
                        fdst[x+y*stash->params.width] = a;
                    }
                }*/

            // Blur
            if (iblur > 0)
            {
                stash->nscratch = 0;
                u8* bdst = &stash->tex_data[glyph->x0 + glyph->y0 * stash->params.width];
                fons_blur(stash, bdst, gw, gh, stash->params.width, iblur);
            }

            stash->dirty_rect[0] = fons_mini(stash->dirty_rect[0], glyph->x0);
            stash->dirty_rect[1] = fons_mini(stash->dirty_rect[1], glyph->y0);
            stash->dirty_rect[2] = fons_maxi(stash->dirty_rect[2], glyph->x1);
            stash->dirty_rect[3] = fons_maxi(stash->dirty_rect[3], glyph->y1);

            return glyph;
        }

        void fons_getQuad(const FONScontext* stash, FONSfont* font, const i32 prevGlyphIndex,
                          const FONSglyph* glyph, const f32 scale, const f32 spacing, f32* x,
                          const f32* y, FONSquad* q)
        {
            f32 rx, ry;

            if (prevGlyphIndex != -1)
            {
                const f32 adv = fons_tt_get_glyph_kern_advance(&font->font, prevGlyphIndex,
                                                               glyph->index) *
                                scale;
                *x += static_cast<i32>(adv + spacing + 0.5f);
            }

            // Each glyph has 2px border to allow good interpolation,
            // one pixel to prevent leaking, and one to allow good interpolation for rendering.
            // Inset the texture region by one pixel for correct interpolation.
            const f32 xoff = static_cast<int16_t>(glyph->xoff + 1);
            const f32 yoff = static_cast<int16_t>(glyph->yoff + 1);
            const f32 x0 = static_cast<f32>(glyph->x0 + 1);
            const f32 y0 = static_cast<f32>(glyph->y0 + 1);
            const f32 x1 = static_cast<f32>(glyph->x1 - 1);
            const f32 y1 = static_cast<f32>(glyph->y1 - 1);

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

            *x += static_cast<i32>(glyph->xadv / 10.0f + 0.5f);
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

        __inline void fons_vertex(FONScontext* stash, const f32 x, const f32 y, const f32 s,
                                  const f32 t, const u32 c)
        {
            stash->verts[stash->nverts * 2 + 0] = x;
            stash->verts[stash->nverts * 2 + 1] = y;
            stash->tcoords[stash->nverts * 2 + 0] = s;
            stash->tcoords[stash->nverts * 2 + 1] = t;
            stash->colors[stash->nverts] = c;
            stash->nverts++;
        }

        f32 fons_get_vert_align(const FONScontext* stash, const FONSfont* font, const Align align,
                                const int16_t isize)
        {
            if (stash->params.flags & FonsZeroTopleft)
            {
                if ((align & Align::VTop) != 0)
                    return font->ascender * static_cast<f32>(isize) / 10.0f;
                if ((align & Align::VMiddle) != 0)
                    return (font->ascender + font->descender) / 2 * static_cast<f32>(isize) / 10.0f;
                if ((align & Align::VBaseline) != 0)
                    return 0.0f;
                if ((align & Align::VBottom) != 0)
                    return font->descender * static_cast<f32>(isize) / 10.0f;
            }
            else
            {
                if ((align & Align::VTop) != 0)
                    return -font->ascender * static_cast<f32>(isize) / 10.0f;
                if ((align & Align::VMiddle) != 0)
                    return -(font->ascender + font->descender) / 2 * static_cast<f32>(isize) / 10.f;
                if ((align & Align::VBaseline) != 0)
                    return 0.0f;
                if ((align & Align::VBottom) != 0)
                    return -font->descender * static_cast<f32>(isize) / 10.0f;
            }
            return 0.0;
        }

        void fons_free_font(FONSfont* font)
        {
            if (font == nullptr)
                return;

            if (font->glyphs)
                std::free(font->glyphs);
            if (font->free_data && font->data)
                std::free(font->data);

            std::free(font);
        }

        i32 fons_alloc_font(FONScontext* stash)
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
                std::memset(font, 0, sizeof(FONSfont));

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
            std::memset(stash, 0, sizeof(FONScontext));

            stash->params = *params;

            // Allocate scratch buffer.
            stash->scratch = static_cast<u8*>(std::malloc(FONS_SCRATCH_BUF_SIZE));
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
                            stash->itw = 1.0f / static_cast<f32>(stash->params.width);
                            stash->ith = 1.0f / static_cast<f32>(stash->params.height);
                            stash->tex_data = static_cast<u8*>(
                                std::malloc(stash->params.width * stash->params.height));

                            if (stash->tex_data != nullptr)
                            {
                                std::memset(stash->tex_data, 0,
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

    i32 fons_add_fallback_font(const FONScontext* stash, const i32 base, const i32 fallback)
    {
        FONSfont* base_font = stash->fonts[base];
        if (base_font->nfallbacks < FONS_MAX_FALLBACKS)
        {
            base_font->fallbacks[base_font->nfallbacks++] = fallback;
            return 1;
        }
        return 0;
    }

    void fons_reset_fallback_font(const FONScontext* stash, const i32 base)
    {
        FONSfont* base_font = stash->fonts[base];
        base_font->nfallbacks = 0;
        base_font->nglyphs = 0;
        for (i32& i : base_font->lut)
            i = -1;
    }

    void fons_set_size(FONScontext* stash, const f32 size)
    {
        fons_get_state(stash)->size = size;
    }

    void fons_set_color(FONScontext* stash, const u32 color)
    {
        fons_get_state(stash)->color = color;
    }

    void fons_set_spacing(FONScontext* stash, const f32 spacing)
    {
        fons_get_state(stash)->spacing = spacing;
    }

    void fons_set_blur(FONScontext* stash, const f32 blur)
    {
        fons_get_state(stash)->blur = blur;
    }

    void fons_set_align(FONScontext* stash, const Align align)
    {
        fons_get_state(stash)->align = align;
    }

    void fons_set_font(FONScontext* stash, const i32 font)
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
        state->align = Align::HLeft | Align::VBaseline;
    }

    i32 fons_add_font(FONScontext* stash, const char* name, const char* path, const i32 font_index)
    {
        u8* data = nullptr;
        i32 status = -1;

        // Read in the font data.
        FILE* fp = std::fopen(path, "rb");
        if (fp != nullptr)
        {
            status = std::fseek(fp, 0, SEEK_END);
            i32 data_size = static_cast<i32>(std::ftell(fp));
            status = std::fseek(fp, 0, SEEK_SET);
            data = static_cast<u8*>(std::malloc(data_size));
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

    i32 fons_add_font_mem(FONScontext* stash, const char* name, u8* data, const i32 data_size,
                          const i32 free_data, const i32 font_index)
    {
        i32 ascent, descent, line_gap;

        const i32 idx = fons_alloc_font(stash);
        if (idx == FONS_INVALID)
            return FONS_INVALID;

        FONSfont* font = stash->fonts[idx];

        std::strncpy(font->name, name, sizeof(font->name));
        font->name[sizeof(font->name) - 1] = '\0';

        // Init hash lookup.
        for (i32& i : font->lut)
            i = -1;

        // Read in the font data.
        font->data_size = data_size;
        font->data = data;
        font->free_data = static_cast<u8>(free_data);

        // Init font
        stash->nscratch = 0;
        if (fons_tt_load_font(stash, &font->font, data, data_size, font_index))
        {
            // Store normalized line height. The real line height is got
            // by multiplying the lineh by font size.
            fons_tt_get_font_v_metrics(&font->font, &ascent, &descent, &line_gap);
            ascent += line_gap;
            const i32 fh = ascent - descent;
            font->ascender = static_cast<f32>(ascent) / static_cast<f32>(fh);
            font->descender = static_cast<f32>(descent) / static_cast<f32>(fh);
            font->lineh = font->ascender - font->descender;

            return idx;
        }

        fons_free_font(font);
        stash->nfonts--;
        return FONS_INVALID;
    }

    i32 fons_get_font_by_name(const FONScontext* s, const char* name)
    {
        for (i32 i = 0; i < s->nfonts; i++)
            if (std::strcmp(s->fonts[i]->name, name) == 0)
                return i;

        return FONS_INVALID;
    }

    f32 fons_draw_text(FONScontext* stash, f32 x, f32 y, const char* str, const char* end)
    {
        const FONSstate* state = fons_get_state(stash);
        u32 codepoint;
        u32 utf8_state = 0;
        FONSquad q;
        i32 prevGlyphIndex = -1;
        const int16_t isize = static_cast<int16_t>(state->size * 10.0f);
        const int16_t iblur = static_cast<int16_t>(state->blur);
        f32 width;

        if (stash == nullptr)
            return x;
        if (state->font < 0 || state->font >= stash->nfonts)
            return x;
        FONSfont* font = stash->fonts[state->font];
        if (font->data == nullptr)
            return x;

        const f32 scale = fons_tt_get_pixel_height_scale(&font->font,
                                                         static_cast<f32>(isize) / 10.0f);

        if (end == nullptr)
            end = str + std::strlen(str);

        // Align horizontally
        if ((state->align & Align::HLeft) != 0)
        {
            // empty
        }
        else if ((state->align & Align::HRight) != 0)
        {
            width = fons_text_bounds(stash, x, y, str, end, nullptr);
            x -= width;
        }
        else if ((state->align & Align::HCenter) != 0)
        {
            width = fons_text_bounds(stash, x, y, str, end, nullptr);
            x -= width * 0.5f;
        }

        // Align vertically.
        y += fons_get_vert_align(stash, font, state->align, isize);

        for (; str != end; ++str)
        {
            if (fons_decutf8(&utf8_state, &codepoint, *reinterpret_cast<const u8*>(str)))
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

    i32 fons_text_iter_init(FONScontext* stash, FONStextIter* iter, f32 x, f32 y, const char* str,
                            const char* end, const i32 bitmap_option)
    {
        const FONSstate* state = fons_get_state(stash);
        f32 width;

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
                                                     static_cast<f32>(iter->isize) / 10.0f);

        // Align horizontally
        if ((state->align & Align::HLeft) != 0)
        {
            // empty
        }
        else if ((state->align & Align::HRight) != 0)
        {
            width = fons_text_bounds(stash, x, y, str, end, nullptr);
            x -= width;
        }
        else if ((state->align & Align::HCenter) != 0)
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

    i32 fons_text_iter_next(FONScontext* stash, FONStextIter* iter, FONSquad* quad)
    {
        const char* str = iter->next;
        iter->str = iter->next;

        if (str == iter->end)
            return 0;

        for (; str != iter->end; str++)
        {
            if (fons_decutf8(&iter->utf8_state, &iter->codepoint, *reinterpret_cast<const u8*>(str)))
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

    void fons_draw_debug(FONScontext* stash, const f32 x, const f32 y)
    {
        const f32 w{ static_cast<f32>(stash->params.width) };
        const f32 h{ static_cast<f32>(stash->params.height) };
        const f32 u{ math::equal(w, 0.0f) ? 0 : (1.0f / w) };
        const f32 v{ math::equal(h, 0.0f) ? 0 : (1.0f / h) };

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
        for (i32 i = 0; i < stash->atlas->nnodes; i++)
        {
            const FONSatlasNode* n = &stash->atlas->nodes[i];

            if (stash->nverts + 6 > FONS_VERTEX_COUNT)
                fons_flush(stash);

            const f32 nx = static_cast<f32>(n->x);
            const f32 ny = static_cast<f32>(n->y);
            const f32 nw = static_cast<f32>(n->width);

            fons_vertex(stash, x + nx + 0, y + ny + 0, u, v, 0xc00000ff);
            fons_vertex(stash, x + nx + nw, y + ny + 1, u, v, 0xc00000ff);
            fons_vertex(stash, x + nx + nw, y + ny + 0, u, v, 0xc00000ff);

            fons_vertex(stash, x + nx + 0, y + ny + 0, u, v, 0xc00000ff);
            fons_vertex(stash, x + nx + 0, y + ny + 1, u, v, 0xc00000ff);
            fons_vertex(stash, x + nx + nw, y + ny + 1, u, v, 0xc00000ff);
        }

        fons_flush(stash);
    }

    f32 fons_text_bounds(FONScontext* stash, f32 x, f32 y, const char* str, const char* end,
                         f32* bounds)
    {
        const FONSstate* state = fons_get_state(stash);
        u32 codepoint;
        u32 utf8_state = 0;
        FONSquad q;
        i32 prev_glyph_index = -1;
        const int16_t isize = static_cast<int16_t>(state->size * 10.0f);
        const int16_t iblur = static_cast<int16_t>(state->blur);
        f32 maxx, maxy;

        if (stash == nullptr)
            return 0;
        if (state->font < 0 || state->font >= stash->nfonts)
            return 0;
        FONSfont* font = stash->fonts[state->font];
        if (font->data == nullptr)
            return 0;

        const f32 scale = fons_tt_get_pixel_height_scale(&font->font,
                                                         static_cast<f32>(isize) / 10.0f);

        // Align vertically.
        y += fons_get_vert_align(stash, font, state->align, isize);

        f32 minx = maxx = x;
        f32 miny = maxy = y;
        const f32 startx = x;

        if (end == nullptr)
            end = str + strlen(str);

        for (; str != end; ++str)
        {
            if (fons_decutf8(&utf8_state, &codepoint, *reinterpret_cast<const u8*>(str)))
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

        const f32 advance = x - startx;

        // Align horizontally
        if ((state->align & Align::HLeft) != 0)
        {
            // empty
        }
        else if ((state->align & Align::HRight) != 0)
        {
            minx -= advance;
            maxx -= advance;
        }
        else if ((state->align & Align::HCenter) != 0)
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

    void fons_vert_metrics(FONScontext* stash, f32* ascender, f32* descender, f32* lineh)
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

    void fons_line_bounds(FONScontext* stash, f32 y, f32* miny, f32* maxy)
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
            *miny = y - font->ascender * static_cast<f32>(isize) / 10.0f;
            *maxy = *miny + font->lineh * static_cast<f32>(isize) / 10.0f;
        }
        else
        {
            *maxy = y + font->descender * static_cast<f32>(isize) / 10.0f;
            *miny = *maxy - font->lineh * static_cast<f32>(isize) / 10.0f;
        }
    }

    const u8* fons_get_texture_data(const FONScontext* stash, i32* width, i32* height)
    {
        if (width != nullptr)
            *width = stash->params.width;
        if (height != nullptr)
            *height = stash->params.height;

        return stash->tex_data;
    }

    i32 fons_validate_texture(FONScontext* stash, i32* dirty)
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

        for (i32 i = 0; i < stash->nfonts; ++i)
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

        std::free(stash);
    }

    void fons_set_error_callback(FONScontext* stash,
                                 void (*callback)(void* uptr, i32 error, i32 val), void* uptr)
    {
        if (stash == nullptr)
            return;
        stash->handle_error = callback;
        stash->error_uptr = uptr;
    }

    void fons_get_atlas_size(const FONScontext* stash, i32* width, i32* height)
    {
        if (stash == nullptr)
            return;

        *width = stash->params.width;
        *height = stash->params.height;
    }

    i32 fons_expand_atlas(FONScontext* stash, i32 width, i32 height)
    {
        i32 i;
        i32 maxy = 0;
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
        const auto data = static_cast<u8*>(std::malloc(width * height));
        if (data == nullptr)
            return 0;

        for (i = 0; i < stash->params.height; i++)
        {
            u8* dst = &data[i * width];
            const u8* src = &stash->tex_data[i * stash->params.width];
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
        stash->itw = 1.0f / static_cast<f32>(stash->params.width);
        stash->ith = 1.0f / static_cast<f32>(stash->params.height);

        return 1;
    }

    i32 fons_reset_atlas(FONScontext* stash, const i32 width, const i32 height)
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
        stash->tex_data = static_cast<u8*>(realloc(stash->tex_data, width * height));
        if (stash->tex_data == nullptr)
            return 0;
        memset(stash->tex_data, 0, width * height);

        // Reset dirty rect
        stash->dirty_rect[0] = width;
        stash->dirty_rect[1] = height;
        stash->dirty_rect[2] = 0;
        stash->dirty_rect[3] = 0;

        // Reset cached glyphs
        for (i32 i = 0; i < stash->nfonts; i++)
        {
            FONSfont* font = stash->fonts[i];
            font->nglyphs = 0;
            for (i32& j : font->lut)
                j = -1;
        }

        stash->params.width = width;
        stash->params.height = height;
        stash->itw = 1.0f / static_cast<f32>(stash->params.width);
        stash->ith = 1.0f / static_cast<f32>(stash->params.height);

        // Add white rect at 0,0 for debug drawing.
        fons_add_white_rect(stash, 2, 2);

        return 1;
    }
}
