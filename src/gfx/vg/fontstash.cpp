#include <array>
#include <cstdint>
#include <cstdio>
#include <utility>
#include <vector>

#include "gfx/stb/stb_truetype.hpp"
#include "gfx/vg/fontstash.hpp"
#include "nanovg.hpp"
#include "utils/conversions.hpp"

namespace rl::nvg::font {
    using namespace stb;

    constexpr static i32 UTF8_ACCEPT{ 0 };
    constexpr static i32 APREC{ 16 };
    constexpr static i32 ZPREC{ 7 };

    namespace {
        u32 hashint(u32 a)
        {
            a += ~(a << 15);
            a ^= (a >> 10);
            a += (a << 3);
            a ^= (a >> 6);
            a += ~(a << 11);
            a ^= (a >> 16);
            return a;
        }

        i32 tt_init(Context*)
        {
            return 1;
        }

        i32 tt_done(Context*)
        {
            return 1;
        }

        i32 tt_load_font(Context* context, STTFontImpl* font, const u8* data, i32 /*data_size*/,
                         const i32 font_index)
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

        void tt_get_font_v_metrics(const STTFontImpl* font, i32* ascent, i32* descent, i32* line_gap)
        {
            stbtt_get_font_v_metrics(&font->font, ascent, descent, line_gap);
        }

        f32 tt_get_pixel_height_scale(const STTFontImpl* font, const f32 size)
        {
            return stbtt_scale_for_mapping_em_to_pixels(&font->font, size);
        }

        i32 tt_get_glyph_index(const STTFontImpl* font, const i32 codepoint)
        {
            return stbtt_find_glyph_index(&font->font, codepoint);
        }

        i32 tt_build_glyph_bitmap(const STTFontImpl* font, const i32 glyph, f32 /*size*/,
                                  const f32 scale, i32* advance, i32* lsb, i32* x0, i32* y0,
                                  i32* x1, i32* y1)
        {
            stbtt_get_glyph_h_metrics(&font->font, glyph, advance, lsb);
            stbtt_get_glyph_bitmap_box(&font->font, glyph, scale, scale, x0, y0, x1, y1);
            return 1;
        }

        void tt_render_glyph_bitmap(const STTFontImpl* font, u8* output, const i32 out_width,
                                    const i32 out_height, const i32 out_stride, const f32 scale_x,
                                    const f32 scale_y, const i32 glyph)
        {
            stbtt_make_glyph_bitmap(&font->font, output, out_width, out_height, out_stride, scale_x,
                                    scale_y, glyph);
        }

        i32 tt_get_glyph_kern_advance(const STTFontImpl* font, const i32 glyph1, const i32 glyph2)
        {
            return stbtt_get_glyph_kern_advance(&font->font, glyph1, glyph2);
        }

        u32 decutf8(u32* state, u32* codep, const u32 byte)
        {
            constexpr static std::array utf8d = std::to_array<u32>(
                { // The first part of the table maps bytes to character classes that
                  // to reduce the size of the transition table and create bitmasks.
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 7, 7,
                  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                  7, 7, 7, 8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                  2, 2, 2, 2, 2, 2, 2, 2, 10, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 11, 6, 6,
                  6, 5, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,

                  // The second part is a transition table that maps a combination
                  // of a state of the automaton and a character class to a state.
                  0, 12, 24, 36, 60, 96, 84, 12, 12, 12, 48, 72, 12, 12, 12, 12, 12, 12, 12, 12, 12,
                  12, 12, 12, 12, 0, 12, 12, 12, 12, 12, 0, 12, 0, 12, 12, 12, 24, 12, 12, 12, 12,
                  12, 24, 12, 24, 12, 12, 12, 12, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 24,
                  12, 12, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12, 12, 12, 12, 36, 12, 36,
                  12, 12, 12, 36, 12, 12, 12, 12, 12, 36, 12, 36, 12, 12, 12, 36, 12, 12, 12, 12,
                  12, 12, 12, 12, 12, 12 });

            const u32 type = utf8d[byte];

            *codep = (*state != UTF8_ACCEPT) ? (byte & 0x3fu) | (*codep << 6)
                                             : (0xff >> type) & (byte);

            *state = utf8d[256 + *state + type];
            return *state;
        }

        // Atlas based on Skyline Bin Packer by Jukka Jyl�nki
        void delete_atlas(Atlas* atlas)
        {
            if (atlas == nullptr)
                return;
            if (atlas->nodes != nullptr)
                free(atlas->nodes);
            free(atlas);
        }

        Atlas* alloc_atlas(const i32 w, const i32 h, const i32 nnodes)
        {
            // Allocate memory for the font font_ctx.
            const auto atlas = static_cast<Atlas*>(std::malloc(sizeof(Atlas)));
            if (atlas != nullptr) {
                memset(atlas, 0, sizeof(Atlas));

                atlas->width = w;
                atlas->height = h;

                // Allocate space for skyline nodes
                atlas->nodes = static_cast<AtlasNode*>(std::malloc(sizeof(AtlasNode) * nnodes));
                if (atlas->nodes != nullptr) {
                    memset(atlas->nodes, 0, sizeof(AtlasNode) * nnodes);
                    atlas->nnodes = 0;
                    atlas->cnodes = nnodes;

                    // Init root node.
                    atlas->nodes[0].x = 0;
                    atlas->nodes[0].y = 0;
                    atlas->nodes[0].width = static_cast<i16>(w);
                    atlas->nnodes++;

                    return atlas;
                }
            }

            // error
            if (atlas != nullptr)
                delete_atlas(atlas);

            return nullptr;
        }

        i32 atlas_insert_node(Atlas* atlas, const i32 idx, const i32 x, const i32 y, const i32 w)
        {
            // Insert node
            if (atlas->nnodes + 1 > atlas->cnodes) {
                atlas->cnodes = atlas->cnodes == 0 ? 8 : atlas->cnodes * 2;
                atlas->nodes = static_cast<AtlasNode*>(
                    std::realloc(  // NOLINT(bugprone-suspicious-realloc-usage)
                        atlas->nodes, sizeof(AtlasNode) * static_cast<u64>(atlas->cnodes)));

                if (atlas->nodes == nullptr)
                    return 0;
            }

            for (i32 i = atlas->nnodes; i > idx; i--)
                atlas->nodes[i] = atlas->nodes[i - 1];

            atlas->nodes[idx].x = static_cast<i16>(x);
            atlas->nodes[idx].y = static_cast<i16>(y);
            atlas->nodes[idx].width = static_cast<i16>(w);
            atlas->nnodes++;

            return 1;
        }

        void atlas_remove_node(Atlas* atlas, const i32 idx)
        {
            if (atlas->nnodes == 0)
                return;

            for (i32 i = idx; i < atlas->nnodes - 1; i++)
                atlas->nodes[i] = atlas->nodes[i + 1];

            atlas->nnodes--;
        }

        void atlas_expand(Atlas* atlas, const i32 w, const i32 h)
        {
            // Insert node for empty space
            if (w > atlas->width)
                atlas_insert_node(atlas, atlas->nnodes, atlas->width, 0, w - atlas->width);

            atlas->width = w;
            atlas->height = h;
        }

        void atlas_reset(Atlas* atlas, const i32 w, const i32 h)
        {
            atlas->width = w;
            atlas->height = h;
            atlas->nnodes = 0;

            // Init root node.
            atlas->nodes[0].x = 0;
            atlas->nodes[0].y = 0;
            atlas->nodes[0].width = static_cast<i16>(w);
            atlas->nnodes++;
        }

        i32 atlas_add_skyline_level(Atlas* atlas, const i32 idx, const i32 x, const i32 y,
                                    const i32 w, const i32 h)
        {
            // Insert new node
            if (atlas_insert_node(atlas, idx, x, y + h, w) == 0)
                return 0;

            // Delete skyline segments that fall under the shadow of the new segment.
            for (i32 i = idx + 1; i < atlas->nnodes; ++i) {
                if (atlas->nodes[i].x >= atlas->nodes[i - 1].x + atlas->nodes[i - 1].width)
                    break;

                const i32 shrink{ atlas->nodes[i - 1].x + atlas->nodes[i - 1].width -
                                  atlas->nodes[i].x };

                atlas->nodes[i].x += shrink;
                atlas->nodes[i].width -= shrink;
                if (atlas->nodes[i].width > 0)
                    break;

                atlas_remove_node(atlas, i);
                --i;
            }

            // Merge same height skyline segments that are next to each other.
            for (i32 i = 0; i < atlas->nnodes - 1; ++i) {
                if (atlas->nodes[i].y == atlas->nodes[i + 1].y) {
                    atlas->nodes[i].width += atlas->nodes[i + 1].width;
                    atlas_remove_node(atlas, i + 1);
                    --i;
                }
            }

            return 1;
        }

        i32 atlas_rect_fits(const Atlas* atlas, i32 i, const i32 w, const i32 h)
        {
            // Checks if there is enough space at the location of skyline span 'i',
            // and return the max height of all skyline spans under that at that location,
            // (think tetris block being dropped at that position). Or -1 if no space found.
            const i32 x = atlas->nodes[i].x;
            i32 y = atlas->nodes[i].y;
            if (x + w > atlas->width)
                return -1;

            i32 space_left = w;
            while (space_left > 0) {
                if (i == atlas->nnodes)
                    return -1;

                y = math::max(y, atlas->nodes[i].y);
                if (y + h > atlas->height)
                    return -1;

                space_left -= atlas->nodes[i].width;
                ++i;
            }

            return y;
        }

        i32 atlas_add_rect(Atlas* atlas, const i32 rw, const i32 rh, i32* rx, i32* ry)
        {
            i32 besth = atlas->height, bestw = atlas->width, besti = -1;
            i32 bestx = -1, besty = -1;

            // Bottom left fit heuristic.
            for (i32 i = 0; i < atlas->nnodes; i++) {
                const i32 y = atlas_rect_fits(atlas, i, rw, rh);
                if (y != -1) {
                    if (y + rh < besth || (y + rh == besth && atlas->nodes[i].width < bestw)) {
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
            if (atlas_add_skyline_level(atlas, besti, bestx, besty, rw, rh) == 0)
                return 0;

            *rx = bestx;
            *ry = besty;

            return 1;
        }

        void add_white_rect(Context* font_ctx, const i32 w, const i32 h)
        {
            i32 gx, gy;
            if (atlas_add_rect(font_ctx->atlas, w, h, &gx, &gy) == 0)
                return;

            // Rasterize
            u8* dst = &font_ctx->tex_data[gx + gy * font_ctx->params.width];
            for (i32 y = 0; y < h; y++) {
                for (i32 x = 0; x < w; x++)
                    dst[x] = 0xff;
                dst += font_ctx->params.width;
            }

            font_ctx->dirty_rect[0] = std::min(font_ctx->dirty_rect[0], gx);
            font_ctx->dirty_rect[1] = std::min(font_ctx->dirty_rect[1], gy);
            font_ctx->dirty_rect[2] = math::max(font_ctx->dirty_rect[2], gx + w);
            font_ctx->dirty_rect[3] = math::max(font_ctx->dirty_rect[3], gy + h);
        }

        font::State* get_state(Context* font_ctx)
        {
            return &font_ctx->states[font_ctx->nstates - 1];
        }

        void blur_cols(u8* dst, const i32 w, const i32 h, const i32 dst_stride, const i32 alpha)
        {
            i32 x;
            for (i32 y = 0; y < h; y++) {
                i32 z = 0;  // force zero border
                for (x = 1; x < w; x++) {
                    z += (alpha * ((static_cast<i32>(dst[x]) << ZPREC) - z)) >> APREC;
                    dst[x] = static_cast<u8>(z >> ZPREC);
                }
                dst[w - 1] = 0;  // force zero border
                z = 0;
                for (x = w - 2; x >= 0; x--) {
                    z += (alpha * ((static_cast<i32>(dst[x]) << ZPREC) - z)) >> APREC;
                    dst[x] = static_cast<u8>(z >> ZPREC);
                }
                dst[0] = 0;  // force zero border
                dst += dst_stride;
            }
        }

        void blur_rows(u8* dst, const i32 w, const i32 h, const i32 dst_stride, const i32 alpha)
        {
            i32 y;
            for (i32 x = 0; x < w; x++) {
                i32 z = 0;  // force zero border
                for (y = dst_stride; y < h * dst_stride; y += dst_stride) {
                    z += (alpha * ((static_cast<i32>(dst[y]) << ZPREC) - z)) >> APREC;
                    dst[y] = static_cast<u8>(z >> ZPREC);
                }
                dst[(h - 1) * dst_stride] = 0;  // force zero border
                z = 0;
                for (y = (h - 2) * dst_stride; y >= 0; y -= dst_stride) {
                    z += (alpha * ((static_cast<i32>(dst[y]) << ZPREC) - z)) >> APREC;
                    dst[y] = static_cast<u8>(z >> ZPREC);
                }
                dst[0] = 0;  // force zero border
                dst++;
            }
        }

        void blur(const Context* font_ctx, u8* dst, const i32 w, const i32 h, const i32 dst_stride,
                  const i32 blur)
        {
            (void)font_ctx;

            if (blur < 1)
                return;
            // Calculate the alpha such that 90% of the kernel is within the radius. (Kernel extends
            // to infinity)
            const f32 sigma = static_cast<f32>(blur) * 0.57735f;  // 1 / sqrt(3)
            const i32 alpha = static_cast<i32>((1 << APREC) * (1.0f - expf(-2.3f / (sigma + 1.0f))));
            blur_rows(dst, w, h, dst_stride, alpha);
            blur_cols(dst, w, h, dst_stride, alpha);
            blur_rows(dst, w, h, dst_stride, alpha);
            blur_cols(dst, w, h, dst_stride, alpha);
            //  _blurrows(dst, w, h, dstStride, alpha);
            //  _blurcols(dst, w, h, dstStride, alpha);
        }

        Glyph* alloc_glyph(Font* font)
        {
            if (font->nglyphs + 1 > font->cglyphs) {
                font->cglyphs = font->cglyphs == 0 ? 8 : font->cglyphs * 2;
                font->glyphs = static_cast<Glyph*>(std::realloc(
                    font->glyphs,
                    sizeof(Glyph) * font->cglyphs));  // NOLINT(bugprone-suspicious-realloc-usage)

                if (font->glyphs == nullptr)
                    return nullptr;
            }

            font->nglyphs++;

            return &font->glyphs[font->nglyphs - 1];
        }

        Glyph* get_glyph(Context* font_ctx, Font* font, const u32 codepoint, const i16 isize,
                         i16 iblur, const i32 bitmap_option)
        {
            i32 advance;
            i32 lsb;
            i32 x0;
            i32 y0;
            i32 x1;
            i32 y1;
            i32 gx;
            i32 gy;
            Glyph* glyph = nullptr;
            const f32 size = static_cast<f32>(isize) / 10.0f;
            const Font* render_font = font;

            if (isize < 2)
                return nullptr;
            if (iblur > 20)
                iblur = 20;
            const i32 pad = iblur + 2;

            // Reset allocator.
            font_ctx->nscratch = 0;

            // Find code point and size.
            const u32 h = hashint(codepoint) & (HASH_LUT_SIZE - 1);
            i32 i = font->lut[h];
            while (i != -1) {
                if (font->glyphs[i].codepoint == codepoint && font->glyphs[i].size == isize &&
                    font->glyphs[i].blur == iblur) {
                    glyph = &font->glyphs[i];
                    if (bitmap_option == FonsGlyphBitmapOptional ||
                        (glyph->x0 >= 0 && glyph->y0 >= 0))
                        return glyph;
                    // At this point, glyph exists but the bitmap data is not yet created.
                    break;
                }
                i = font->glyphs[i].next;
            }

            // Create a new glyph or rasterize bitmap data for a cached glyph.
            i32 g = tt_get_glyph_index(&font->font, static_cast<i32>(codepoint));
            // Try to find the glyph in fallback fonts.
            if (g == 0) {
                for (i = 0; i < font->nfallbacks; ++i) {
                    const Font* fallbackFont = font_ctx->fonts[font->fallbacks[i]];
                    const i32 fallback_index = tt_get_glyph_index(&fallbackFont->font,
                                                                  static_cast<i32>(codepoint));
                    if (fallback_index != 0) {
                        g = fallback_index;
                        render_font = fallbackFont;
                        break;
                    }
                }
                // It is possible that we did not find a fallback glyph.
                // In that case the glyph index 'g' is 0, and we'll proceed below and cache empty
                // glyph.
            }
            const f32 scale = tt_get_pixel_height_scale(&render_font->font, size);
            tt_build_glyph_bitmap(&render_font->font, g, size, scale, &advance, &lsb, &x0, &y0, &x1,
                                  &y1);
            const i32 gw = x1 - x0 + pad * 2;
            const i32 gh = y1 - y0 + pad * 2;

            // Determines the spot to draw glyph in the atlas.
            if (bitmap_option == FonsGlyphBitmapRequired) {
                // Find free spot for the rect in the atlas
                i32 added = atlas_add_rect(font_ctx->atlas, gw, gh, &gx, &gy);
                if (added == 0 && font_ctx->handle_error != nullptr) {
                    // Atlas is full, let the user to resize the atlas (or not), and try again.
                    font_ctx->handle_error(font_ctx->error_uptr, ErrorCode::FonsAtlasFull, 0);
                    added = atlas_add_rect(font_ctx->atlas, gw, gh, &gx, &gy);
                }
                if (added == 0)
                    return nullptr;
            }
            else {
                // Negative coordinate indicates there is no bitmap data created.
                gx = -1;
                gy = -1;
            }

            // Init glyph.
            if (glyph == nullptr) {
                glyph = alloc_glyph(font);
                glyph->codepoint = codepoint;
                glyph->size = isize;
                glyph->blur = iblur;
                glyph->next = 0;

                // Insert char to hash lookup.
                glyph->next = font->lut[h];
                font->lut[h] = font->nglyphs - 1;
            }
            glyph->index = g;
            glyph->x0 = static_cast<i16>(gx);
            glyph->y0 = static_cast<i16>(gy);
            glyph->x1 = static_cast<i16>(glyph->x0 + gw);
            glyph->y1 = static_cast<i16>(glyph->y0 + gh);
            glyph->x_adv = static_cast<i16>(scale * static_cast<f32>(advance) * 10.0f);
            glyph->x_off = static_cast<i16>(x0 - pad);
            glyph->y_off = static_cast<i16>(y0 - pad);

            if (bitmap_option == FonsGlyphBitmapOptional)
                return glyph;

            // Rasterize
            u8* dst = &font_ctx->tex_data[(glyph->x0 + pad) +
                                          (glyph->y0 + pad) * font_ctx->params.width];
            tt_render_glyph_bitmap(&render_font->font, dst, gw - pad * 2, gh - pad * 2,
                                   font_ctx->params.width, scale, scale, g);

            // Make sure there is one pixel empty border.
            dst = &font_ctx->tex_data[glyph->x0 + glyph->y0 * font_ctx->params.width];
            for (i32 y = 0; y < gh; y++) {
                dst[y * font_ctx->params.width] = 0;
                dst[gw - 1 + y * font_ctx->params.width] = 0;
            }
            for (i32 x = 0; x < gw; x++) {
                dst[x] = 0;
                dst[x + (gh - 1) * font_ctx->params.width] = 0;
            }

            // Debug code to color the glyph background
            /*
            u8* fdst = &font_ctx->texData[glyph->x0 + glyph->y0 * font_ctx->params.width];
            for (y = 0; y < gh; y++) {
                for (x = 0; x < gw; x++) {
                    i32 a = (i32)fdst[x+y*font_ctx->params.width] + 20;
                    if (a > 255) a = 255;
                    fdst[x+y*font_ctx->params.width] = a;
                }
            }
            */

            // Blur
            if (iblur > 0) {
                font_ctx->nscratch = 0;
                u8* bdst = &font_ctx->tex_data[glyph->x0 + glyph->y0 * font_ctx->params.width];
                blur(font_ctx, bdst, gw, gh, font_ctx->params.width, iblur);
            }

            font_ctx->dirty_rect[0] = math::min(font_ctx->dirty_rect[0], glyph->x0);
            font_ctx->dirty_rect[1] = math::min(font_ctx->dirty_rect[1], glyph->y0);
            font_ctx->dirty_rect[2] = math::max(font_ctx->dirty_rect[2], glyph->x1);
            font_ctx->dirty_rect[3] = math::max(font_ctx->dirty_rect[3], glyph->y1);

            return glyph;
        }

        void get_quad(const Context* font_ctx, const Font* font, const i32 prev_glyph_index,
                      const Glyph* glyph, const f32 scale, const f32 spacing, f32* x, const f32* y,
                      FontQuad* q)
        {
            if (prev_glyph_index != -1) {
                const f32 adv{ scale * static_cast<f32>(tt_get_glyph_kern_advance(
                                           &font->font, prev_glyph_index, glyph->index)) };
                *x += std::round(adv + spacing);
            }

            // Each glyph has 2px border to allow good interpolation,
            // one pixel to prevent leaking, and one to allow good interpolation for rendering.
            // Inset the texture region by one pixel for correct interpolation.
            const f32 xoff = static_cast<i16>(glyph->x_off + 1);
            const f32 yoff = static_cast<i16>(glyph->y_off + 1);
            const f32 x0 = static_cast<f32>(glyph->x0 + 1);
            const f32 y0 = static_cast<f32>(glyph->y0 + 1);
            const f32 x1 = static_cast<f32>(glyph->x1 - 1);
            const f32 y1 = static_cast<f32>(glyph->y1 - 1);

            if (font_ctx->params.flags & FonsZeroTopleft) {
                f32 rx = std::floor(*x + xoff);
                f32 ry = std::floor(*y + yoff);

                q->x0 = rx;
                q->y0 = ry;
                q->x1 = rx + x1 - x0;
                q->y1 = ry + y1 - y0;

                q->s0 = x0 * font_ctx->itw;
                q->t0 = y0 * font_ctx->ith;
                q->s1 = x1 * font_ctx->itw;
                q->t1 = y1 * font_ctx->ith;
            }
            else {
                f32 rx = std::floor(*x + xoff);
                f32 ry = std::floor(*y - yoff);

                q->x0 = rx;
                q->y0 = ry;
                q->x1 = rx + x1 - x0;
                q->y1 = ry - y1 + y0;

                q->s0 = x0 * font_ctx->itw;
                q->t0 = y0 * font_ctx->ith;
                q->s1 = x1 * font_ctx->itw;
                q->t1 = y1 * static_cast<f32>(font_ctx->ith);
            }

            *x += std::round(static_cast<f32>(glyph->x_adv) / 10.0f);
        }

        void flush(Context* font_ctx)
        {
            // Flush texture
            if (font_ctx->dirty_rect[0] < font_ctx->dirty_rect[2] &&
                font_ctx->dirty_rect[1] < font_ctx->dirty_rect[3]) {
                if (font_ctx->params.render_update != nullptr)
                    font_ctx->params.render_update(font_ctx->params.user_ptr, font_ctx->dirty_rect,
                                                   font_ctx->tex_data);
                // Reset dirty rect
                font_ctx->dirty_rect[0] = font_ctx->params.width;
                font_ctx->dirty_rect[1] = font_ctx->params.height;
                font_ctx->dirty_rect[2] = 0;
                font_ctx->dirty_rect[3] = 0;
            }

            // Flush triangles
            if (font_ctx->nverts > 0) {
                if (font_ctx->params.render_draw != nullptr)
                    font_ctx->params.render_draw(font_ctx->params.user_ptr, font_ctx->verts,
                                                 font_ctx->tcoords, font_ctx->colors,
                                                 font_ctx->nverts);
                font_ctx->nverts = 0;
            }
        }

        void vertex(Context* font_ctx, const f32 x, const f32 y, const f32 s, const f32 t,
                    const u32 c)
        {
            font_ctx->verts[font_ctx->nverts * 2 + 0] = x;
            font_ctx->verts[font_ctx->nverts * 2 + 1] = y;
            font_ctx->tcoords[font_ctx->nverts * 2 + 0] = s;
            font_ctx->tcoords[font_ctx->nverts * 2 + 1] = t;
            font_ctx->colors[font_ctx->nverts] = c;
            font_ctx->nverts++;
        }

        f32 get_vert_align(const Context* font_ctx, const Font* font, const Align align,
                           const i16 isize)
        {
            if (font_ctx->params.flags & FonsZeroTopleft) {
                if ((align & Align::VTop) != 0)
                    return font->ascender * static_cast<f32>(isize) / 10.0f;
                if ((align & Align::VMiddle) != 0)
                    return (font->ascender + font->descender) / 2 * static_cast<f32>(isize) / 10.0f;
                if ((align & Align::VBaseline) != 0)
                    return 0.0f;
                if ((align & Align::VBottom) != 0)
                    return font->descender * static_cast<f32>(isize) / 10.0f;
            }
            else {
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

        void free_font(Font* font)
        {
            if (font == nullptr)
                return;

            if (font->glyphs)
                std::free(font->glyphs);
            if (font->free_data && font->data)
                std::free(font->data);

            std::free(font);
        }

        i32 alloc_font(Context* font_ctx)
        {
            if (font_ctx->nfonts + 1 > font_ctx->cfonts) {
                font_ctx->cfonts = font_ctx->cfonts == 0 ? 8 : font_ctx->cfonts * 2;
                font_ctx->fonts = static_cast<Font**>(
                    std::realloc(font_ctx->fonts, sizeof(Font*) * font_ctx->cfonts));

                if (font_ctx->fonts == nullptr)
                    return -1;
            }

            const auto font = static_cast<Font*>(std::malloc(sizeof(Font)));
            if (font != nullptr) {
                std::memset(font, 0, sizeof(Font));

                font->glyphs = static_cast<Glyph*>(std::malloc(sizeof(Glyph) * INIT_GLYPHS));

                if (font->glyphs != nullptr) {
                    font->cglyphs = INIT_GLYPHS;
                    font->nglyphs = 0;

                    font_ctx->fonts[font_ctx->nfonts++] = font;
                    return font_ctx->nfonts - 1;
                }
            }

            free_font(font);
            return INVALID;
        }
    }

    Context* create_internal(const Params* params)
    {
        // Allocate memory for the font font_ctx.
        const auto font_ctx = static_cast<Context*>(std::malloc(sizeof(Context)));
        if (font_ctx != nullptr) {
            std::memset(font_ctx, 0, sizeof(Context));

            font_ctx->params = *params;

            // Allocate scratch buffer.
            font_ctx->scratch = static_cast<u8*>(std::malloc(SCRATCH_BUF_SIZE));
            if (font_ctx->scratch != nullptr) {
                // Initialize implementation library
                if (tt_init(font_ctx) || font_ctx->params.render_create != nullptr ||
                    font_ctx->params.render_create(font_ctx->params.user_ptr, font_ctx->params.width,
                                                   font_ctx->params.height) != 0) {
                    font_ctx->atlas = alloc_atlas(font_ctx->params.width, font_ctx->params.height,
                                                  INIT_ATLAS_NODES);
                    if (font_ctx->atlas != nullptr) {
                        // Allocate space for fonts.
                        font_ctx->fonts = static_cast<Font**>(
                            std::malloc(sizeof(Font*) * INIT_FONTS));

                        if (font_ctx->fonts != nullptr) {
                            memset(font_ctx->fonts, 0, sizeof(Font*) * INIT_FONTS);
                            font_ctx->cfonts = INIT_FONTS;
                            font_ctx->nfonts = 0;

                            // Create texture for the cache.
                            font_ctx->itw = 1.0f / static_cast<f32>(font_ctx->params.width);
                            font_ctx->ith = 1.0f / static_cast<f32>(font_ctx->params.height);
                            font_ctx->tex_data = static_cast<u8*>(
                                std::malloc(font_ctx->params.width * font_ctx->params.height));

                            if (font_ctx->tex_data != nullptr) {
                                std::memset(font_ctx->tex_data, 0,
                                            font_ctx->params.width * font_ctx->params.height);

                                font_ctx->dirty_rect[0] = font_ctx->params.width;
                                font_ctx->dirty_rect[1] = font_ctx->params.height;
                                font_ctx->dirty_rect[2] = 0;
                                font_ctx->dirty_rect[3] = 0;

                                // Add white rect at 0,0 for debug drawing.
                                add_white_rect(font_ctx, 2, 2);
                                push_state(font_ctx);
                                clear_state(font_ctx);
                                return font_ctx;
                            }
                        }
                    }
                }
            }
        }

        delete_internal(font_ctx);
        return nullptr;
    }

    i32 add_fallback_font(const Context* font_ctx, const i32 base, const i32 fallback)
    {
        Font* base_font = font_ctx->fonts[base];
        if (base_font->nfallbacks < MAX_FALLBACKS) {
            base_font->fallbacks[base_font->nfallbacks++] = fallback;
            return 1;
        }
        return 0;
    }

    void reset_fallback_font(const Context* font_ctx, const i32 base)
    {
        Font* base_font = font_ctx->fonts[base];
        base_font->nfallbacks = 0;
        base_font->nglyphs = 0;
        for (i32& i : base_font->lut)
            i = -1;
    }

    void set_size(Context* font_ctx, const f32 size)
    {
        get_state(font_ctx)->size = size;
    }

    void set_color(Context* font_ctx, const u32 color)
    {
        get_state(font_ctx)->color = color;
    }

    void set_spacing(Context* font_ctx, const f32 spacing)
    {
        get_state(font_ctx)->spacing = spacing;
    }

    void set_blur(Context* font_ctx, const f32 blur)
    {
        get_state(font_ctx)->blur = blur;
    }

    void set_align(Context* font_ctx, const Align align)
    {
        get_state(font_ctx)->align = align;
    }

    void set_font(Context* font_ctx, const i32 font)
    {
        get_state(font_ctx)->font = font;
    }

    void push_state(Context* font_ctx)
    {
        if (font_ctx->nstates >= MAX_STATES) {
            if (font_ctx->handle_error)
                font_ctx->handle_error(font_ctx->error_uptr, ErrorCode::FonsStatesOverflow, 0);

            return;
        }

        if (font_ctx->nstates > 0) {
            std::memcpy(&font_ctx->states[font_ctx->nstates],
                        &font_ctx->states[font_ctx->nstates - 1], sizeof(font::State));
        }

        font_ctx->nstates++;
    }

    void pop_state(Context* font_ctx)
    {
        if (font_ctx->nstates <= 1) {
            if (font_ctx->handle_error)
                font_ctx->handle_error(font_ctx->error_uptr, ErrorCode::FonsStatesUnderflow, 0);
            return;
        }

        font_ctx->nstates--;
    }

    void clear_state(Context* font_ctx)
    {
        font::State* state = get_state(font_ctx);
        state->size = 12.0f;
        state->color = 0xffffffff;
        state->font = 0;
        state->blur = 0;
        state->spacing = 0;
        state->align = Align::HLeft | Align::VBaseline;
    }

    i32 add_font(Context* font_ctx, const char* name, const char* path, const i32 font_index)
    {
        u8* data = nullptr;
        i32 status = -1;

        // Read in the font data.
        FILE* fp = std::fopen(path, "rb");
        if (fp != nullptr) {
            status = std::fseek(fp, 0, SEEK_END);
            debug_assert(status == 0, "fseek failed");
            i32 data_size = static_cast<i32>(std::ftell(fp));
            status = std::fseek(fp, 0, SEEK_SET);
            debug_assert(status == 0, "fseek failed");
            data = static_cast<u8*>(std::malloc(data_size));
            if (data != nullptr) {
                const size_t readed = std::fread(data, 1, data_size, fp);
                status = std::fclose(fp);
                debug_assert(status == 0, "fseek failed");

                fp = nullptr;
                if (readed == static_cast<size_t>(data_size))
                    return add_font_mem(font_ctx, name, data, data_size, 1, font_index);
            }
        }

        if (data != nullptr)
            std::free(data);
        if (fp != nullptr) {
            status = std::fclose(fp);
            debug_assert(status == 0, "fseek failed");
        }

        return font::INVALID;
    }

    i32 add_font_mem(Context* font_ctx, const char* name, u8* data, const i32 data_size,
                     const i32 free_data, const i32 font_index)
    {
        i32 ascent, descent, line_gap;

        const i32 idx = alloc_font(font_ctx);
        if (idx == INVALID)
            return INVALID;

        Font* font = font_ctx->fonts[idx];

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
        font_ctx->nscratch = 0;
        if (tt_load_font(font_ctx, &font->font, data, data_size, font_index)) {
            // Store normalized line height. The real line height is got
            // by multiplying the lineh by font size.
            tt_get_font_v_metrics(&font->font, &ascent, &descent, &line_gap);
            ascent += line_gap;
            const i32 fh = ascent - descent;
            font->ascender = static_cast<f32>(ascent) / static_cast<f32>(fh);
            font->descender = static_cast<f32>(descent) / static_cast<f32>(fh);
            font->lineh = font->ascender - font->descender;

            return idx;
        }

        free_font(font);
        font_ctx->nfonts--;
        return INVALID;
    }

    i32 get_font_by_name(const Context* font_ctx, const char* name)
    {
        for (i32 i = 0; i < font_ctx->nfonts; i++)
            if (std::strcmp(font_ctx->fonts[i]->name, name) == 0)
                return i;

        return INVALID;
    }

    f32 draw_text(Context* font_ctx, f32 x, f32 y, const char* str, const char* end)
    {
        const font::State* state = get_state(font_ctx);
        u32 codepoint;
        u32 utf8_state = 0;
        FontQuad q;
        i32 prev_glyph_index = -1;
        const i16 isize = static_cast<i16>(state->size * 10.0f);
        const i16 iblur = static_cast<i16>(state->blur);
        f32 width;

        if (font_ctx == nullptr)
            return x;

        if (state->font < 0 || state->font >= font_ctx->nfonts)
            return x;

        Font* font = font_ctx->fonts[state->font];
        if (font->data == nullptr)
            return x;

        const f32 scale = tt_get_pixel_height_scale(&font->font, static_cast<f32>(isize) / 10.0f);

        if (end == nullptr)
            end = str + std::strlen(str);

        // Align horizontally
        if ((state->align & Align::HLeft) != 0) {
            // empty
        }
        else if ((state->align & Align::HRight) != 0) {
            width = text_bounds(font_ctx, { x, y }, str);
            x -= width;
        }
        else if ((state->align & Align::HCenter) != 0) {
            width = text_bounds(font_ctx, { x, y }, str);
            x -= width * 0.5f;
        }

        // Align vertically.
        y += get_vert_align(font_ctx, font, state->align, isize);

        for (; str != end; ++str) {
            if (decutf8(&utf8_state, &codepoint, *reinterpret_cast<const u8*>(str)))
                continue;

            const Glyph* glyph = get_glyph(font_ctx, font, codepoint, isize, iblur,
                                           FonsGlyphBitmapRequired);
            if (glyph != nullptr) {
                get_quad(font_ctx, font, prev_glyph_index, glyph, scale, state->spacing, &x, &y, &q);
                if (font_ctx->nverts + 6 > VERTEX_COUNT)
                    flush(font_ctx);

                vertex(font_ctx, q.x0, q.y0, q.s0, q.t0, state->color);
                vertex(font_ctx, q.x1, q.y1, q.s1, q.t1, state->color);
                vertex(font_ctx, q.x1, q.y0, q.s1, q.t0, state->color);

                vertex(font_ctx, q.x0, q.y0, q.s0, q.t0, state->color);
                vertex(font_ctx, q.x0, q.y1, q.s0, q.t1, state->color);
                vertex(font_ctx, q.x1, q.y1, q.s1, q.t1, state->color);
            }

            prev_glyph_index = glyph != nullptr ? glyph->index : -1;
        }

        flush(font_ctx);
        return x;
    }

    i32 text_iter_init(Context* font_ctx, TextIter* iter, ds::point<f32> pos, const char* str,
                       const char* end, const i32 bitmap_option)
    {
        const nvg::font::State* state{ nvg::font::get_state(font_ctx) };

        std::memset(iter, 0, sizeof(*iter));
        if (font_ctx == nullptr)
            return 0;
        if (state->font < 0 || state->font >= font_ctx->nfonts)
            return 0;

        iter->font = font_ctx->fonts[state->font];
        if (iter->font->data == nullptr)
            return 0;

        iter->isize = static_cast<i16>(state->size * 10.0f);
        iter->iblur = static_cast<i16>(state->blur);
        iter->scale = tt_get_pixel_height_scale(&iter->font->font,
                                                static_cast<f32>(iter->isize) / 10.0f);
        // horizontal
        f32 width{ 0.0f };

        if ((state->align & Align::HLeft) != 0)
            ;  // intentionally empty
        else if ((state->align & Align::HRight) != 0) {
            width = text_bounds(font_ctx, pos, str);
            pos.x -= width;
        }
        else if ((state->align & Align::HCenter) != 0) {
            width = text_bounds(font_ctx, pos, str);
            pos.x -= width * 0.5f;
        }

        // vertical
        pos.y += get_vert_align(font_ctx, iter->font, state->align, iter->isize);
        if (end == nullptr)
            end = str + std::strlen(str);

        iter->x = pos.x;
        iter->y = pos.y;
        iter->nextx = pos.x;
        iter->nexty = pos.y;
        iter->spacing = state->spacing;
        iter->str = str;
        iter->next = str;
        iter->end = end;
        iter->codepoint = 0;
        iter->prev_glyph_index = -1;
        iter->bitmap_option = bitmap_option;
        return 1;
    }

    i32 text_iter_next(Context* font_ctx, TextIter* iter, FontQuad* quad)
    {
        const char* str = iter->next;
        iter->str = iter->next;

        if (str == iter->end)
            return 0;

        for (; str != iter->end; str++) {
            if (decutf8(&iter->utf8_state, &iter->codepoint, *reinterpret_cast<const u8*>(str)))
                continue;

            str++;
            // Get glyph and quad
            iter->x = iter->nextx;
            iter->y = iter->nexty;
            const Glyph* glyph = get_glyph(font_ctx, iter->font, iter->codepoint, iter->isize,
                                           iter->iblur, iter->bitmap_option);
            // If the iterator was initialized with GLYPH_BITMAP_OPTIONAL, then the UV
            // coordinates of the quad will be invalid.
            if (glyph != nullptr)
                get_quad(font_ctx, iter->font, iter->prev_glyph_index, glyph, iter->scale,
                         iter->spacing, &iter->nextx, &iter->nexty, quad);
            iter->prev_glyph_index = glyph != nullptr ? glyph->index : -1;
            break;
        }
        iter->next = str;

        return 1;
    }

    void draw_debug(Context* font_ctx, const f32 x, const f32 y)
    {
        const f32 w{ static_cast<f32>(font_ctx->params.width) };
        const f32 h{ static_cast<f32>(font_ctx->params.height) };
        const f32 u{ math::equal(w, 0.0f) ? 0 : (1.0f / w) };
        const f32 v{ math::equal(h, 0.0f) ? 0 : (1.0f / h) };

        if (font_ctx->nverts + 6 + 6 > VERTEX_COUNT)
            flush(font_ctx);

        // Draw background
        vertex(font_ctx, x + 0, y + 0, u, v, 0x0fffffff);
        vertex(font_ctx, x + w, y + h, u, v, 0x0fffffff);
        vertex(font_ctx, x + w, y + 0, u, v, 0x0fffffff);

        vertex(font_ctx, x + 0, y + 0, u, v, 0x0fffffff);
        vertex(font_ctx, x + 0, y + h, u, v, 0x0fffffff);
        vertex(font_ctx, x + w, y + h, u, v, 0x0fffffff);

        // Draw texture
        vertex(font_ctx, x + 0, y + 0, 0, 0, 0xffffffff);
        vertex(font_ctx, x + w, y + h, 1, 1, 0xffffffff);
        vertex(font_ctx, x + w, y + 0, 1, 0, 0xffffffff);

        vertex(font_ctx, x + 0, y + 0, 0, 0, 0xffffffff);
        vertex(font_ctx, x + 0, y + h, 0, 1, 0xffffffff);
        vertex(font_ctx, x + w, y + h, 1, 1, 0xffffffff);

        // Drawbug draw atlas
        for (i32 i = 0; i < font_ctx->atlas->nnodes; i++) {
            const AtlasNode* n = &font_ctx->atlas->nodes[i];

            if (font_ctx->nverts + 6 > VERTEX_COUNT)
                flush(font_ctx);

            const f32 nx = static_cast<f32>(n->x);
            const f32 ny = static_cast<f32>(n->y);
            const f32 nw = static_cast<f32>(n->width);

            vertex(font_ctx, x + nx + 0, y + ny + 0, u, v, 0xc00000ff);
            vertex(font_ctx, x + nx + nw, y + ny + 1, u, v, 0xc00000ff);
            vertex(font_ctx, x + nx + nw, y + ny + 0, u, v, 0xc00000ff);

            vertex(font_ctx, x + nx + 0, y + ny + 0, u, v, 0xc00000ff);
            vertex(font_ctx, x + nx + 0, y + ny + 1, u, v, 0xc00000ff);
            vertex(font_ctx, x + nx + nw, y + ny + 1, u, v, 0xc00000ff);
        }

        flush(font_ctx);
    }

    f32 text_bounds(Context* font_ctx, const ds::point<f32> pos, const char* str, const char* end)
    {
        ds::rect<f32> bounds{ ds::rect<f32>::null() };
        return text_bounds(font_ctx, pos, str, end, bounds);
    }

    f32 text_bounds(Context* font_ctx, ds::point<f32> pos, const char* str, const char* end,
                    ds::rect<f32>& bounds)
    {
        f32 text_width{ 0.0f };

        const font::State* state = get_state(font_ctx);
        u32 codepoint;
        u32 utf8_state = 0;
        FontQuad q;
        i32 prev_glyph_index = -1;
        const i16 isize = static_cast<i16>(state->size * 10.0f);
        const i16 iblur = static_cast<i16>(state->blur);
        f32 maxx, maxy;

        if (font_ctx == nullptr)
            return text_width;

        if (state->font < 0 || state->font >= font_ctx->nfonts)
            return text_width;

        Font* font = font_ctx->fonts[state->font];
        if (font->data == nullptr)
            return text_width;

        const f32 scale = tt_get_pixel_height_scale(&font->font, static_cast<f32>(isize) / 10.0f);

        // Align vertically.
        pos.y += get_vert_align(font_ctx, font, state->align, isize);

        f32 minx = maxx = pos.x;
        f32 miny = maxy = pos.y;

        const f32 startx{ pos.x };
        if (end == nullptr)
            end = str + std::strlen(str);

        for (; str < end; ++str) {
            if (decutf8(&utf8_state, &codepoint, *reinterpret_cast<const u8*>(str)))
                continue;

            const Glyph* glyph = get_glyph(font_ctx, font, codepoint, isize, iblur,
                                           FonsGlyphBitmapOptional);
            if (glyph != nullptr) {
                get_quad(font_ctx, font, prev_glyph_index, glyph, scale, state->spacing, &pos.x,
                         &pos.y, &q);

                if (q.x0 < minx)
                    minx = q.x0;
                if (q.x1 > maxx)
                    maxx = q.x1;

                if (font_ctx->params.flags & FonsZeroTopleft) {
                    if (q.y0 < miny)
                        miny = q.y0;
                    if (q.y1 > maxy)
                        maxy = q.y1;
                }
                else {
                    if (q.y1 < miny)
                        miny = q.y1;
                    if (q.y0 > maxy)
                        maxy = q.y0;
                }
            }

            prev_glyph_index = glyph != nullptr ? glyph->index : -1;
        }

        text_width = pos.x - startx;

        // Align horizontally
        if ((state->align & Align::HLeft) != 0) {
            // empty
        }
        else if ((state->align & Align::HRight) != 0) {
            minx -= text_width;
            maxx -= text_width;
        }
        else if ((state->align & Align::HCenter) != 0) {
            minx -= text_width * 0.5f;
            maxx -= text_width * 0.5f;
        }

        if (!bounds.is_null()) {
            bounds = ds::rect<f32>{
                ds::point<f32>{
                    minx,
                    miny,
                },
                ds::dims<f32>{
                    maxx - minx,
                    maxy - miny,
                },
            };
        }

        return text_width;
    }

    void vert_metrics(Context* font_ctx, f32* ascender, f32* descender, f32* lineh)
    {
        if (font_ctx == nullptr)
            return;

        const font::State* state = get_state(font_ctx);
        if (state->font < 0 || state->font >= font_ctx->nfonts)
            return;

        const Font* font{ font_ctx->fonts[state->font] };
        if (font->data == nullptr)
            return;

        const f32 isize{ std::round(state->size * 10.0f) };
        if (ascender != nullptr)
            *ascender = font->ascender * isize / 10.0f;
        if (descender != nullptr)
            *descender = font->descender * isize / 10.0f;
        if (lineh != nullptr)
            *lineh = font->lineh * isize / 10.0f;
    }

    void line_bounds(Context* font_ctx, f32 y, f32* miny, f32* maxy)
    {
        if (font_ctx == nullptr)
            return;

        const font::State* state = get_state(font_ctx);
        if (state->font < 0 || state->font >= font_ctx->nfonts)
            return;

        const Font* font = font_ctx->fonts[state->font];
        if (font->data == nullptr)
            return;

        const i16 isize = static_cast<i16>(state->size * 10.0f);
        y += get_vert_align(font_ctx, font, state->align, isize);

        if (font_ctx->params.flags & FonsZeroTopleft) {
            *miny = y - font->ascender * static_cast<f32>(isize) / 10.0f;
            *maxy = *miny + font->lineh * static_cast<f32>(isize) / 10.0f;
        }
        else {
            *maxy = y + font->descender * static_cast<f32>(isize) / 10.0f;
            *miny = *maxy - font->lineh * static_cast<f32>(isize) / 10.0f;
        }
    }

    const u8* get_texture_data(const Context* font_ctx, i32* width, i32* height)
    {
        if (width != nullptr)
            *width = font_ctx->params.width;
        if (height != nullptr)
            *height = font_ctx->params.height;

        return font_ctx->tex_data;
    }

    i32 validate_texture(Context* font_ctx, i32* dirty)
    {
        if (font_ctx->dirty_rect[0] < font_ctx->dirty_rect[2] &&
            font_ctx->dirty_rect[1] < font_ctx->dirty_rect[3]) {
            dirty[0] = font_ctx->dirty_rect[0];
            dirty[1] = font_ctx->dirty_rect[1];
            dirty[2] = font_ctx->dirty_rect[2];
            dirty[3] = font_ctx->dirty_rect[3];
            // Reset dirty rect
            font_ctx->dirty_rect[0] = font_ctx->params.width;
            font_ctx->dirty_rect[1] = font_ctx->params.height;
            font_ctx->dirty_rect[2] = 0;
            font_ctx->dirty_rect[3] = 0;
            return 1;
        }
        return 0;
    }

    void delete_internal(Context* font_ctx)
    {
        if (font_ctx == nullptr)
            return;

        if (font_ctx->params.render_delete)
            font_ctx->params.render_delete(font_ctx->params.user_ptr);

        for (i32 i = 0; i < font_ctx->nfonts; ++i)
            free_font(font_ctx->fonts[i]);

        if (font_ctx->atlas)
            delete_atlas(font_ctx->atlas);
        if (font_ctx->fonts)
            free(font_ctx->fonts);
        if (font_ctx->tex_data)
            free(font_ctx->tex_data);
        if (font_ctx->scratch)
            free(font_ctx->scratch);

        tt_done(font_ctx);

        std::free(font_ctx);
    }

    void set_error_callback(Context* font_ctx,
                            void (*callback)(void* uptr, ErrorCode error, i32 val), void* uptr)
    {
        if (font_ctx == nullptr)
            return;

        font_ctx->handle_error = callback;
        font_ctx->error_uptr = uptr;
    }

    void get_atlas_size(const Context* font_ctx, i32* width, i32* height)
    {
        if (font_ctx == nullptr)
            return;

        *width = font_ctx->params.width;
        *height = font_ctx->params.height;
    }

    i32 expand_atlas(Context* font_ctx, i32 width, i32 height)
    {
        i32 maxy = 0;
        if (font_ctx == nullptr)
            return 0;

        width = math::max(width, font_ctx->params.width);
        height = math::max(height, font_ctx->params.height);

        if (width == font_ctx->params.width && height == font_ctx->params.height)
            return 1;

        // Flush pending glyphs.
        flush(font_ctx);

        // Create new texture
        if (font_ctx->params.render_resize != nullptr) {
            if (font_ctx->params.render_resize(font_ctx->params.user_ptr, width, height) == 0)
                return 0;
        }

        // Copy old texture data over.
        const auto data = static_cast<u8*>(std::malloc(width * height));
        if (data == nullptr)
            return 0;

        for (i32 i = 0; i < font_ctx->params.height; i++) {
            u8* dst = &data[i * width];
            const u8* src = &font_ctx->tex_data[i * font_ctx->params.width];
            std::memcpy(dst, src, font_ctx->params.width);
            if (width > font_ctx->params.width)
                std::memset(dst + font_ctx->params.width, 0, width - font_ctx->params.width);
        }
        if (height > font_ctx->params.height)
            std::memset(&data[font_ctx->params.height * width], 0,
                        (height - font_ctx->params.height) * width);

        std::free(font_ctx->tex_data);
        font_ctx->tex_data = data;

        // Increase atlas size
        atlas_expand(font_ctx->atlas, width, height);

        // Add existing data as dirty.
        for (i32 i = 0; i < font_ctx->atlas->nnodes; i++)
            maxy = math::max(maxy, font_ctx->atlas->nodes[i].y);

        font_ctx->dirty_rect[0] = 0;
        font_ctx->dirty_rect[1] = 0;
        font_ctx->dirty_rect[2] = font_ctx->params.width;
        font_ctx->dirty_rect[3] = maxy;

        font_ctx->params.width = width;
        font_ctx->params.height = height;
        font_ctx->itw = 1.0f / static_cast<f32>(font_ctx->params.width);
        font_ctx->ith = 1.0f / static_cast<f32>(font_ctx->params.height);

        return 1;
    }

    i32 reset_atlas(Context* font_ctx, const i32 width, const i32 height)
    {
        if (font_ctx == nullptr)
            return 0;

        // Flush pending glyphs.
        flush(font_ctx);

        // Create new texture
        if (font_ctx->params.render_resize != nullptr) {
            if (font_ctx->params.render_resize(font_ctx->params.user_ptr, width, height) == 0)
                return 0;
        }

        // Reset atlas
        atlas_reset(font_ctx->atlas, width, height);

        // Clear texture data.
        font_ctx->tex_data = static_cast<u8*>(std::realloc(font_ctx->tex_data, width * height));
        if (font_ctx->tex_data == nullptr)
            return 0;

        std::memset(font_ctx->tex_data, 0, width * height);

        // Reset dirty rect
        font_ctx->dirty_rect[0] = width;
        font_ctx->dirty_rect[1] = height;
        font_ctx->dirty_rect[2] = 0;
        font_ctx->dirty_rect[3] = 0;

        // Reset cached glyphs
        for (i32 i = 0; i < font_ctx->nfonts; i++) {
            Font* font = font_ctx->fonts[i];
            font->nglyphs = 0;
            for (i32& j : font->lut)
                j = -1;
        }

        font_ctx->params.width = width;
        font_ctx->params.height = height;
        font_ctx->itw = 1.0f / static_cast<f32>(font_ctx->params.width);
        font_ctx->ith = 1.0f / static_cast<f32>(font_ctx->params.height);

        // Add white rect at 0,0 for debug drawing.
        add_white_rect(font_ctx, 2, 2);

        return 1;
    }
}
