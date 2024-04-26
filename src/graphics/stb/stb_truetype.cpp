#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <utility>

#include "core/assert.hpp"
#include "graphics/stb/stb_truetype.hpp"
#include "utils/conversions.hpp"
#include "utils/numeric.hpp"

#define STBTT_RASTERIZER_VERSION 2

#define stbtt_tag4(p, c0, c1, c2, c3) ((p)[0] == (c0) && (p)[1] == (c1) && (p)[2] == (c2) && (p)[3] == (c3))
#define stbtt_tag(p, str)             stbtt_tag4(p, (str)[0], (str)[1], (str)[2], (str)[3])

#define STBTT_CSCTX_INIT(bounds)                   \
    {                                              \
        bounds, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0 \
    }

namespace rl::stb {
    constexpr static int MaxOversample{ 8 };
    constexpr static int OversampleMask{ MaxOversample - 1 };
    static_assert(MaxOversample <= 255, "STBTT_MAX_OVERSAMPLE cannot be > 255");
    using stbtt_test_oversample_pow2 = i32[(MaxOversample & OversampleMask) == 0 ? 1 : -1];

    namespace {
        // stbtt_buf helpers to parse data from file

        u8 stbtt_buf_peek8(const stbtt_buf* b)
        {
            if (b->cursor >= b->size)
                return 0;
            return b->data[b->cursor];
        }

        void stbtt_buf_seek(stbtt_buf* b, const i32 o)
        {
            debug_assert(!(o > b->size || o < 0));
            b->cursor = (o > b->size || o < 0) ? b->size : o;
        }

        void stbtt_buf_skip(stbtt_buf* b, const i32 o)
        {
            stbtt_buf_seek(b, b->cursor + o);
        }

        constexpr u8 stbtt_buf_get8(stbtt_buf* b)
        {
            if (b->cursor >= b->size)
                return 0;
            return b->data[b->cursor++];
        }

        constexpr u32 stbtt_buf_get(stbtt_buf* b, const i32 n)
        {
            u32 v{ 0 };
            if (!std::is_constant_evaluated()) {
                debug_assert(n >= 1 && n <= 4);
            }
            for (i32 i = 0; i < n; i++)
                v = (v << 8) | stbtt_buf_get8(b);
            return v;
        }

        constexpr u32 stbtt_buf_get16(stbtt_buf* b)
        {
            return stbtt_buf_get(b, 2);
        }

        constexpr u32 stbtt_buf_get32(stbtt_buf* b)
        {
            return stbtt_buf_get(b, 4);
        }

        stbtt_buf stbtt_new_buf(void* p, const size_t size)
        {
            stbtt_buf r;
            debug_assert(size < 0x40000000);
            r.data = static_cast<u8*>(p);
            r.size = static_cast<i32>(size);
            r.cursor = 0;
            return r;
        }

        stbtt_buf stbtt_buf_range(const stbtt_buf* b, const i32 o, const i32 s)
        {
            stbtt_buf r = stbtt_new_buf(nullptr, 0);
            if (o < 0 || s < 0 || o > b->size || s > b->size - o)
                return r;
            r.data = b->data + o;
            r.size = s;
            return r;
        }

        stbtt_buf stbtt_cff_get_index(stbtt_buf* b)
        {
            i32 start = b->cursor;
            i32 count = stbtt_buf_get16(b);
            if (count) {
                i32 offsize = stbtt_buf_get8(b);
                debug_assert(offsize >= 1 && offsize <= 4);
                stbtt_buf_skip(b, offsize * count);
                stbtt_buf_skip(b, stbtt_buf_get(b, offsize) - 1);
            }
            return stbtt_buf_range(b, start, b->cursor - start);
        }

        u32 stbtt_cff_int(stbtt_buf* b)
        {
            i32 b0 = stbtt_buf_get8(b);
            if (b0 >= 32 && b0 <= 246)
                return b0 - 139;
            if (b0 >= 247 && b0 <= 250)
                return (b0 - 247) * 256 + stbtt_buf_get8(b) + 108;
            if (b0 >= 251 && b0 <= 254)
                return -(b0 - 251) * 256 - stbtt_buf_get8(b) - 108;
            if (b0 == 28)
                return stbtt_buf_get16(b);
            if (b0 == 29)
                return stbtt_buf_get32(b);

            debug_assert(false);
            return 0;
        }

        void stbtt_cff_skip_operand(stbtt_buf* b)
        {
            i32 b0 = stbtt_buf_peek8(b);
            debug_assert(b0 >= 28);
            if (b0 == 30) {
                stbtt_buf_skip(b, 1);
                while (b->cursor < b->size) {
                    i32 v = stbtt_buf_get8(b);
                    if ((v & 0xF) == 0xF || (v >> 4) == 0xF)
                        break;
                }
            }
            else {
                stbtt_cff_int(b);
            }
        }

        stbtt_buf stbtt_dict_get(stbtt_buf* b, const i32 key)
        {
            stbtt_buf_seek(b, 0);
            while (b->cursor < b->size) {
                i32 start = b->cursor;
                while (stbtt_buf_peek8(b) >= 28)
                    stbtt_cff_skip_operand(b);
                i32 end = b->cursor;
                i32 op = stbtt_buf_get8(b);
                if (op == 12)
                    op = stbtt_buf_get8(b) | 0x100;
                if (op == key)
                    return stbtt_buf_range(b, start, end - start);
            }
            return stbtt_buf_range(b, 0, 0);
        }

        void stbtt_dict_get_ints(stbtt_buf* b, const i32 key, const i32 outcount, u32* out)
        {
            stbtt_buf operands = stbtt_dict_get(b, key);
            for (i32 i = 0; i < outcount && operands.cursor < operands.size; i++)
                out[i] = stbtt_cff_int(&operands);
        }

        i32 stbtt_cff_index_count(stbtt_buf* b)
        {
            stbtt_buf_seek(b, 0);
            return stbtt_buf_get16(b);
        }

        stbtt_buf stbtt_cff_index_get(stbtt_buf b, const i32 i)
        {
            stbtt_buf_seek(&b, 0);
            i32 count = stbtt_buf_get16(&b);
            i32 offsize = stbtt_buf_get8(&b);
            debug_assert(i >= 0 && i < count);
            debug_assert(offsize >= 1 && offsize <= 4);
            stbtt_buf_skip(&b, i * offsize);
            i32 start = stbtt_buf_get(&b, offsize);
            i32 end = stbtt_buf_get(&b, offsize);
            return stbtt_buf_range(&b, 2 + (count + 1) * offsize + start, end - start);
        }

        //////////////////////////////////////////////////////////////////////////
        //
        // accessors to parse data from file
        //

        // on platforms that don't allow misaligned reads, if we want to allow
        // truetype fonts that aren't padded to alignment, define ALLOW_UNALIGNED_TRUETYPE

        u16 tt_aligned_u16(const u8* p)
        {
            return p[0] * 256 + p[1];
        }

        i16 tt_aligned_i16(const u8* p)
        {
            return p[0] * 256 + p[1];
        }

        u32 tt_aligned_u32(const u8* p)
        {
            return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
        }

        i32 tt_aligned_i32(const u8* p)
        {
            return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
        }

        i32 stbtt_isfont(const u8* font)
        {
            // check the version number
            if (stbtt_tag4(font, '1', 0, 0, 0))
                return 1;  // TrueType 1
            if (stbtt_tag(font, "typ1"))
                return 1;  // TrueType with type 1 font -- we don't support this!
            if (stbtt_tag(font, "OTTO"))
                return 1;  // OpenType with CFF
            if (stbtt_tag4(font, 0, 1, 0, 0))
                return 1;  // OpenType 1.0
            if (stbtt_tag(font, "true"))
                return 1;  // Apple specification for TrueType fonts
            return 0;
        }

        // @OPTIMIZE: binary search
        u32 stbtt_find_table(const u8* data, const u32 fontstart, const char* tag)
        {
            i32 num_tables = tt_aligned_u16(data + fontstart + 4);
            u32 tabledir = fontstart + 12;
            for (i32 i = 0; i < num_tables; ++i) {
                u32 loc = tabledir + 16 * i;
                if (stbtt_tag(data + loc + 0, tag))
                    return tt_aligned_u32(data + loc + 8);
            }
            return 0;
        }

        i32 stbtt_GetFontOffsetForIndex_internal(const u8* font_collection, const i32 index)
        {
            // if it's just a font, there's only one valid index
            if (stbtt_isfont(font_collection))
                return index == 0 ? 0 : -1;

            // check if it's a TTC
            if (stbtt_tag(font_collection, "ttcf")) {
                // version 1?
                if (tt_aligned_u32(font_collection + 4) == 0x00010000 ||
                    tt_aligned_u32(font_collection + 4) == 0x00020000) {
                    i32 n = tt_aligned_i32(font_collection + 8);
                    if (index >= n)
                        return -1;

                    return tt_aligned_u32(font_collection + 12 + index * 4);
                }
            }

            return -1;
        }

        i32 stbtt_GetNumberOfFonts_internal(const u8* font_collection)
        {
            // if it's just a font, there's only one valid font
            if (stbtt_isfont(font_collection))
                return 1;

            // check if it's a TTC
            if (stbtt_tag(font_collection, "ttcf")) {
                // version 1?
                if (tt_aligned_u32(font_collection + 4) == 0x00010000 ||
                    tt_aligned_u32(font_collection + 4) == 0x00020000) {
                    return tt_aligned_i32(font_collection + 8);
                }
            }
            return 0;
        }

        stbtt_buf stbtt_get_subrs(stbtt_buf cff, stbtt_buf fontdict)
        {
            u32 subrsoff = 0, private_loc[2] = { 0, 0 };
            stbtt_buf pdict;
            stbtt_dict_get_ints(&fontdict, 18, 2, private_loc);
            if (!private_loc[1] || !private_loc[0])
                return stbtt_new_buf(nullptr, 0);
            pdict = stbtt_buf_range(&cff, private_loc[1], private_loc[0]);
            stbtt_dict_get_ints(&pdict, 19, 1, &subrsoff);
            if (!subrsoff)
                return stbtt_new_buf(nullptr, 0);
            stbtt_buf_seek(&cff, private_loc[1] + subrsoff);
            return stbtt_cff_get_index(&cff);
        }

        // since most people won't use this, find this table the first time it's needed
        i32 stbtt_get_svg(stbtt_fontinfo* info)
        {
            if (info->svg < 0) {
                u32 t = stbtt_find_table(info->data, info->fontstart, "SVG ");
                if (t) {
                    u32 offset = tt_aligned_u32(info->data + t + 2);
                    info->svg = t + offset;
                }
                else {
                    info->svg = 0;
                }
            }
            return info->svg;
        }

        i32 stbtt_InitFont_internal(stbtt_fontinfo* info, u8* data, const i32 fontstart)
        {
            info->data = data;
            info->fontstart = fontstart;
            info->cff = stbtt_new_buf(nullptr, 0);

            u32 cmap = stbtt_find_table(data, fontstart, "cmap");    // required
            info->loca = stbtt_find_table(data, fontstart, "loca");  // required
            info->head = stbtt_find_table(data, fontstart, "head");  // required
            info->glyf = stbtt_find_table(data, fontstart, "glyf");  // required
            info->hhea = stbtt_find_table(data, fontstart, "hhea");  // required
            info->hmtx = stbtt_find_table(data, fontstart, "hmtx");  // required
            info->kern = stbtt_find_table(data, fontstart, "kern");  // not required
            info->gpos = stbtt_find_table(data, fontstart, "GPOS");  // not required

            if (!cmap || !info->head || !info->hhea || !info->hmtx)
                return 0;
            if (info->glyf) {
                // required for truetype
                if (!info->loca)
                    return 0;
            }
            else {
                stbtt_buf b;
                stbtt_buf topdict;
                u32 cstype = 2;
                u32 charstrings = 0;
                u32 fdarrayoff = 0;
                u32 fdselectoff = 0;

                const u32 cff = stbtt_find_table(data, fontstart, "CFF ");
                if (!cff)
                    return 0;

                info->fontdicts = stbtt_new_buf(nullptr, 0);
                info->fdselect = stbtt_new_buf(nullptr, 0);

                // @TODO this should use size from table (not 512MB)
                info->cff = stbtt_new_buf(data + cff, 512 * 1024 * 1024);
                b = info->cff;

                // read the header
                stbtt_buf_skip(&b, 2);
                stbtt_buf_seek(&b, stbtt_buf_get8(&b));  // hdrsize

                // @TODO the name INDEX could list multiple fonts,
                // but we just use the first one.
                stbtt_cff_get_index(&b);  // name INDEX
                stbtt_buf topdictidx = stbtt_cff_get_index(&b);
                topdict = stbtt_cff_index_get(topdictidx, 0);
                stbtt_cff_get_index(&b);  // string INDEX
                info->gsubrs = stbtt_cff_get_index(&b);

                stbtt_dict_get_ints(&topdict, 17, 1, &charstrings);
                stbtt_dict_get_ints(&topdict, 0x100 | 6, 1, &cstype);
                stbtt_dict_get_ints(&topdict, 0x100 | 36, 1, &fdarrayoff);
                stbtt_dict_get_ints(&topdict, 0x100 | 37, 1, &fdselectoff);
                info->subrs = stbtt_get_subrs(b, topdict);

                // we only support Type 2 charstrings
                if (cstype != 2)
                    return 0;
                if (charstrings == 0)
                    return 0;

                if (fdarrayoff) {
                    // looks like a CID font
                    if (!fdselectoff)
                        return 0;
                    stbtt_buf_seek(&b, fdarrayoff);
                    info->fontdicts = stbtt_cff_get_index(&b);
                    info->fdselect = stbtt_buf_range(&b, fdselectoff, b.size - fdselectoff);
                }

                stbtt_buf_seek(&b, charstrings);
                info->charstrings = stbtt_cff_get_index(&b);
            }

            u32 t = stbtt_find_table(data, fontstart, "maxp");
            if (t)
                info->num_glyphs = tt_aligned_u16(data + t + 4);
            else
                info->num_glyphs = 0xffff;

            info->svg = -1;

            // find a cmap encoding table we understand *now* to avoid searching
            // later. (todo: could make this installable)
            // the same regardless of glyph.
            i32 num_tables = tt_aligned_u16(data + cmap + 2);
            info->index_map = 0;
            for (i32 i = 0; i < num_tables; ++i) {
                u32 encoding_record = cmap + 4 + 8 * i;
                // find an encoding we understand:
                switch (tt_aligned_u16(data + encoding_record)) {
                    case STBTT_PLATFORM_ID_MICROSOFT:
                        switch (tt_aligned_u16(data + encoding_record + 2)) {
                            case STBTT_MS_EID_UNICODE_BMP:
                            case STBTT_MS_EID_UNICODE_FULL:
                                // MS/Unicode
                                info->index_map = cmap + tt_aligned_u32(data + encoding_record + 4);
                                break;
                        }
                        break;
                    case STBTT_PLATFORM_ID_UNICODE:
                        // Mac/iOS has these
                        // all the encodingIDs are unicode, so we don't bother to check it
                        info->index_map = cmap + tt_aligned_u32(data + encoding_record + 4);
                        break;
                }
            }
            if (info->index_map == 0)
                return 0;

            info->index_to_loc_format = tt_aligned_u16(data + info->head + 50);
            return 1;
        }
    }

    i32 stbtt_find_glyph_index(const stbtt_fontinfo* info, const i32 unicode_codepoint)
    {
        u8* data = info->data;
        u32 index_map = info->index_map;

        u16 format = tt_aligned_u16(data + index_map + 0);
        if (format == 0) {  // apple byte encoding
            i32 bytes = tt_aligned_u16(data + index_map + 2);
            if (unicode_codepoint < bytes - 6)
                return (*(data + index_map + 6 + unicode_codepoint));
            return 0;
        }
        if (format == 6) {
            u32 first = tt_aligned_u16(data + index_map + 6);
            u32 count = tt_aligned_u16(data + index_map + 8);
            if (static_cast<u32>(unicode_codepoint) >= first &&
                static_cast<u32>(unicode_codepoint) < first + count)
                return tt_aligned_u16(data + index_map + 10 + (unicode_codepoint - first) * 2);
            return 0;
        }
        if (format == 2) {
            debug_assert(0);  // @TODO: high-byte mapping for japanese/chinese/korean
            return 0;
        }
        if (format == 4) {  // standard mapping for windows fonts: binary search collection of
                            // ranges
            u16 segcount = tt_aligned_u16(data + index_map + 6) >> 1;
            u16 searchRange = tt_aligned_u16(data + index_map + 8) >> 1;
            u16 entrySelector = tt_aligned_u16(data + index_map + 10);
            u16 rangeShift = tt_aligned_u16(data + index_map + 12) >> 1;

            // do a binary search of the segments
            u32 endCount = index_map + 14;
            u32 search = endCount;

            if (unicode_codepoint > 0xffff)
                return 0;

            // they lie from endCount .. endCount + segCount
            // but searchRange is the nearest power of two, so...
            if (unicode_codepoint >= tt_aligned_u16(data + search + rangeShift * 2))
                search += rangeShift * 2;

            // now decrement to bias correctly to find smallest
            search -= 2;
            while (entrySelector) {
                searchRange >>= 1;
                u16 end = tt_aligned_u16(data + search + searchRange * 2);
                if (unicode_codepoint > end)
                    search += searchRange * 2;
                --entrySelector;
            }
            search += 2;

            {
                u16 item = static_cast<u16>((search - endCount) >> 1);

                u16 start = tt_aligned_u16(data + index_map + 14 + segcount * 2 + 2 + 2 * item);
                u16 last = tt_aligned_u16(data + endCount + 2 * item);
                if (unicode_codepoint < start || unicode_codepoint > last)
                    return 0;

                u16 offset = tt_aligned_u16(data + index_map + 14 + segcount * 6 + 2 + 2 * item);
                if (offset == 0)
                    return static_cast<u16>(
                        unicode_codepoint +
                        tt_aligned_i16(data + index_map + 14 + segcount * 4 + 2 + 2 * item));

                return tt_aligned_u16(data + offset + (unicode_codepoint - start) * 2 + index_map +
                                      14 + segcount * 6 + 2 + 2 * item);
            }
        }
        else if (format == 12 || format == 13) {
            u32 ngroups = tt_aligned_u32(data + index_map + 12);
            i32 low = 0;
            i32 high = static_cast<i32>(ngroups);
            // Binary search the right group.
            while (low < high) {
                i32 mid = low + ((high - low) >> 1);  // rounds down, so low <= mid < high
                u32 start_char = tt_aligned_u32(data + index_map + 16 + mid * 12);
                u32 end_char = tt_aligned_u32(data + index_map + 16 + mid * 12 + 4);
                if (static_cast<u32>(unicode_codepoint) < start_char)
                    high = mid;
                else if (static_cast<u32>(unicode_codepoint) > end_char)
                    low = mid + 1;
                else {
                    u32 start_glyph = tt_aligned_u32(data + index_map + 16 + mid * 12 + 8);
                    if (format == 12)
                        return start_glyph + unicode_codepoint - start_char;
                    else  // format == 13
                        return start_glyph;
                }
            }
            return 0;  // not found
        }
        // @TODO
        debug_assert(0);
        return 0;
    }

    i32 stbtt_get_codepoint_shape(const stbtt_fontinfo* info, const i32 unicode_codepoint,
                                  stbtt_vertex** vertices)
    {
        return stbtt_get_glyph_shape(info, stbtt_find_glyph_index(info, unicode_codepoint),
                                     vertices);
    }

    static void stbtt_setvertex(stbtt_vertex* v, const u8 type, const i32 x, const i32 y,
                                const i32 cx, const i32 cy)
    {
        v->type = type;
        v->x = static_cast<i16>(x);
        v->y = static_cast<i16>(y);
        v->cx = static_cast<i16>(cx);
        v->cy = static_cast<i16>(cy);
    }

    static i32 stbtt_get_glyf_offset(const stbtt_fontinfo* info, const i32 glyph_index)
    {
        i32 g1, g2;

        debug_assert(!info->cff.size);

        if (glyph_index >= info->num_glyphs)
            return -1;  // glyph index out of range
        if (info->index_to_loc_format >= 2)
            return -1;  // unknown index->glyph map format

        if (info->index_to_loc_format == 0) {
            g1 = info->glyf + tt_aligned_u16(info->data + info->loca + glyph_index * 2) * 2;
            g2 = info->glyf + tt_aligned_u16(info->data + info->loca + glyph_index * 2 + 2) * 2;
        }
        else {
            g1 = info->glyf + tt_aligned_u32(info->data + info->loca + glyph_index * 4);
            g2 = info->glyf + tt_aligned_u32(info->data + info->loca + glyph_index * 4 + 4);
        }

        return g1 == g2 ? -1 : g1;  // if length is 0, return -1
    }

    static i32 stbtt_get_glyph_info_t2(const stbtt_fontinfo* info, i32 glyph_index, i32* x0,
                                       i32* y0, i32* x1, i32* y1);

    i32 stbtt_get_glyph_box(const stbtt_fontinfo* info, const i32 glyph_index, i32* x0, i32* y0,
                            i32* x1, i32* y1)
    {
        if (info->cff.size) {
            stbtt_get_glyph_info_t2(info, glyph_index, x0, y0, x1, y1);
        }
        else {
            i32 g = stbtt_get_glyf_offset(info, glyph_index);
            if (g < 0)
                return 0;

            if (x0)
                *x0 = tt_aligned_i16(info->data + g + 2);
            if (y0)
                *y0 = tt_aligned_i16(info->data + g + 4);
            if (x1)
                *x1 = tt_aligned_i16(info->data + g + 6);
            if (y1)
                *y1 = tt_aligned_i16(info->data + g + 8);
        }
        return 1;
    }

    i32 stbtt_get_codepoint_box(const stbtt_fontinfo* info, const i32 codepoint, i32* x0, i32* y0,
                                i32* x1, i32* y1)
    {
        return stbtt_get_glyph_box(info, stbtt_find_glyph_index(info, codepoint), x0, y0, x1, y1);
    }

    i32 stbtt_is_glyph_empty(const stbtt_fontinfo* info, const i32 glyph_index)
    {
        if (info->cff.size)
            return 0 ==
                   stbtt_get_glyph_info_t2(info, glyph_index, nullptr, nullptr, nullptr, nullptr);
        i32 g = stbtt_get_glyf_offset(info, glyph_index);
        if (g < 0)
            return 1;
        i16 numberOfContours = tt_aligned_i16(info->data + g);
        return numberOfContours == 0;
    }

    static i32 stbtt_close_shape(stbtt_vertex* vertices, i32 num_vertices, const i32 was_off,
                                 const i32 start_off, const i32 sx, const i32 sy, const i32 scx,
                                 const i32 scy, const i32 cx, const i32 cy)
    {
        if (start_off) {
            if (was_off)
                stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx + scx) >> 1,
                                (cy + scy) >> 1, cx, cy);
            stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, sx, sy, scx, scy);
        }
        else {
            if (was_off)
                stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, sx, sy, cx, cy);
            else
                stbtt_setvertex(&vertices[num_vertices++], STBTT_vline, sx, sy, 0, 0);
        }
        return num_vertices;
    }

    static i32 stbtt_get_glyph_shape_tt(const stbtt_fontinfo* info, i32 glyph_index,
                                        stbtt_vertex** pvertices)
    {
        i16 number_of_contours;
        u8* end_pts_of_contours;
        u8* data = info->data;
        stbtt_vertex* vertices = nullptr;
        i32 num_vertices = 0;
        i32 g = stbtt_get_glyf_offset(info, glyph_index);

        *pvertices = nullptr;

        if (g < 0)
            return 0;

        number_of_contours = tt_aligned_i16(data + g);

        if (number_of_contours > 0) {
            u8 flags = 0, flagcount;
            i32 ins, i, j = 0, m, n, next_move, was_off = 0, off, start_off = 0;
            i32 x, y, cx, cy, sx, sy, scx, scy;
            u8* points;
            end_pts_of_contours = (data + g + 10);
            ins = tt_aligned_u16(data + g + 10 + number_of_contours * 2);
            points = data + g + 10 + number_of_contours * 2 + 2 + ins;

            n = 1 + tt_aligned_u16(end_pts_of_contours + number_of_contours * 2 - 2);

            m = n + 2 * number_of_contours;  // a loose bound on how many vertices we might need
            vertices = static_cast<stbtt_vertex*>(std::malloc(m * sizeof(vertices[0])));
            if (vertices == nullptr)
                return 0;

            next_move = 0;
            flagcount = 0;

            // in first pass, we load uninterpreted data into the allocated array
            // above, shifted to the end of the array so we won't overwrite it when
            // we create our final data starting from the front

            off = m - n;  // starting offset for uninterpreted data, regardless of how m ends up
                          // being calculated

            // first load flags

            for (i = 0; i < n; ++i) {
                if (flagcount == 0) {
                    flags = *points++;
                    if (flags & 8)
                        flagcount = *points++;
                }
                else
                    --flagcount;
                vertices[off + i].type = flags;
            }

            // now load x coordinates
            x = 0;
            for (i = 0; i < n; ++i) {
                flags = vertices[off + i].type;
                if (flags & 2) {
                    i16 dx = *points++;
                    x += (flags & 16) ? dx : -dx;  // ???
                }
                else if (!(flags & 16)) {
                    x = x + static_cast<i16>(points[0] * 256 + points[1]);
                    points += 2;
                }
                vertices[off + i].x = static_cast<i16>(x);
            }

            // now load y coordinates
            y = 0;
            for (i = 0; i < n; ++i) {
                flags = vertices[off + i].type;
                if (flags & 4) {
                    i16 dy = *points++;
                    y += (flags & 32) ? dy : -dy;  // ???
                }
                else if (!(flags & 32)) {
                    y = y + static_cast<i16>(points[0] * 256 + points[1]);
                    points += 2;
                }
                vertices[off + i].y = static_cast<i16>(y);
            }

            // now convert them to our format
            num_vertices = 0;
            sx = sy = cx = cy = scx = scy = 0;
            for (i = 0; i < n; ++i) {
                flags = vertices[off + i].type;
                x = vertices[off + i].x;
                y = vertices[off + i].y;

                if (next_move == i) {
                    if (i != 0)
                        num_vertices = stbtt_close_shape(vertices, num_vertices, was_off, start_off,
                                                         sx, sy, scx, scy, cx, cy);

                    // now start the new one
                    start_off = !(flags & 1);
                    if (start_off) {
                        // if we start off with an off-curve point, then when we need to find a
                        // point on the curve where we can start, and we need to save some state for
                        // when we wraparound.
                        scx = x;
                        scy = y;
                        if (!(vertices[off + i + 1].type & 1)) {
                            // next point is also a curve point, so interpolate an on-point curve
                            sx = (x + static_cast<i32>(vertices[off + i + 1].x)) >> 1;
                            sy = (y + static_cast<i32>(vertices[off + i + 1].y)) >> 1;
                        }
                        else {
                            // otherwise just use the next point as our start point
                            sx = static_cast<i32>(vertices[off + i + 1].x);
                            sy = static_cast<i32>(vertices[off + i + 1].y);
                            ++i;  // we're using point i+1 as the starting point, so skip it
                        }
                    }
                    else {
                        sx = x;
                        sy = y;
                    }
                    stbtt_setvertex(&vertices[num_vertices++], STBTT_vmove, sx, sy, 0, 0);
                    was_off = 0;
                    next_move = 1 + tt_aligned_u16(end_pts_of_contours + j * 2);
                    ++j;
                }
                else if (!(flags & 1)) {  // if it's a curve
                    if (was_off)          // two off-curve control points in a row means interpolate an
                                          // on-curve midpoint
                        stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx + x) >> 1,
                                        (cy + y) >> 1, cx, cy);
                    cx = x;
                    cy = y;
                    was_off = 1;
                }
                else {
                    if (was_off)
                        stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, x, y, cx, cy);
                    else
                        stbtt_setvertex(&vertices[num_vertices++], STBTT_vline, x, y, 0, 0);
                    was_off = 0;
                }
            }
            num_vertices = stbtt_close_shape(vertices, num_vertices, was_off, start_off, sx, sy,
                                             scx, scy, cx, cy);
        }
        else if (number_of_contours < 0) {
            // Compound shapes.
            i32 more = 1;
            u8* comp = data + g + 10;
            num_vertices = 0;
            vertices = nullptr;
            while (more) {
                u16 flags, gidx;
                i32 comp_num_verts = 0, i;
                stbtt_vertex *comp_verts = nullptr, *tmp = nullptr;
                f32 mtx[6] = { 1, 0, 0, 1, 0, 0 }, m, n;

                flags = tt_aligned_i16(comp);
                comp += 2;
                gidx = tt_aligned_i16(comp);
                comp += 2;

                if (flags & 2) {      // XY values
                    if (flags & 1) {  // shorts
                        mtx[4] = tt_aligned_i16(comp);
                        comp += 2;
                        mtx[5] = tt_aligned_i16(comp);
                        comp += 2;
                    }
                    else {
                        mtx[4] = static_cast<i8>(*comp);
                        comp += 1;
                        mtx[5] = static_cast<i8>(*comp);
                        comp += 1;
                    }
                }
                else {
                    // @TODO handle matching point
                    debug_assert(0);
                }
                if (flags & (1 << 3)) {  // WE_HAVE_A_SCALE
                    mtx[0] = mtx[3] = tt_aligned_i16(comp) / 16384.0f;
                    comp += 2;
                    mtx[1] = mtx[2] = 0;
                }
                else if (flags & (1 << 6)) {  // WE_HAVE_AN_X_AND_YSCALE
                    mtx[0] = tt_aligned_i16(comp) / 16384.0f;
                    comp += 2;
                    mtx[1] = mtx[2] = 0;
                    mtx[3] = tt_aligned_i16(comp) / 16384.0f;
                    comp += 2;
                }
                else if (flags & (1 << 7)) {  // WE_HAVE_A_TWO_BY_TWO
                    mtx[0] = tt_aligned_i16(comp) / 16384.0f;
                    comp += 2;
                    mtx[1] = tt_aligned_i16(comp) / 16384.0f;
                    comp += 2;
                    mtx[2] = tt_aligned_i16(comp) / 16384.0f;
                    comp += 2;
                    mtx[3] = tt_aligned_i16(comp) / 16384.0f;
                    comp += 2;
                }

                // Find transformation scales.
                m = std::sqrt(mtx[0] * mtx[0] + mtx[1] * mtx[1]);
                n = std::sqrt(mtx[2] * mtx[2] + mtx[3] * mtx[3]);

                // Get indexed glyph.
                comp_num_verts = stbtt_get_glyph_shape(info, gidx, &comp_verts);
                if (comp_num_verts > 0) {
                    // Transform vertices.
                    for (i = 0; i < comp_num_verts; ++i) {
                        stbtt_vertex* v = &comp_verts[i];
                        stbtt_vertex_type x, y;
                        x = v->x;
                        y = v->y;
                        v->x = static_cast<short>(m * (mtx[0] * x + mtx[2] * y + mtx[4]));
                        v->y = static_cast<short>(n * (mtx[1] * x + mtx[3] * y + mtx[5]));
                        x = v->cx;
                        y = v->cy;
                        v->cx = static_cast<short>(m * (mtx[0] * x + mtx[2] * y + mtx[4]));
                        v->cy = static_cast<short>(n * (mtx[1] * x + mtx[3] * y + mtx[5]));
                    }
                    // Append vertices.
                    tmp = static_cast<stbtt_vertex*>(
                        std::malloc((num_vertices + comp_num_verts) * sizeof(stbtt_vertex)));
                    if (!tmp) {
                        if (vertices)
                            std::free(vertices);
                        if (comp_verts)
                            std::free(comp_verts);
                        return 0;
                    }
                    if (num_vertices > 0 && vertices)
                        std::memcpy(tmp, vertices, num_vertices * sizeof(stbtt_vertex));
                    std::memcpy(tmp + num_vertices, comp_verts,
                                comp_num_verts * sizeof(stbtt_vertex));
                    if (vertices)
                        std::free(vertices);
                    vertices = tmp;
                    std::free(comp_verts);
                    num_vertices += comp_num_verts;
                }
                // More components ?
                more = flags & (1 << 5);
            }
        }
        else {
            // numberOfCounters == 0, do nothing
        }

        *pvertices = vertices;
        return num_vertices;
    }

    typedef struct
    {
        i32 bounds;
        i32 started;
        f32 first_x, first_y;
        f32 x, y;
        i32 min_x, max_x, min_y, max_y;

        stbtt_vertex* pvertices;
        i32 num_vertices;
    } stbtt_csctx;

    static void stbtt_track_vertex(stbtt_csctx* c, const i32 x, const i32 y)
    {
        if (x > c->max_x || !c->started)
            c->max_x = x;
        if (y > c->max_y || !c->started)
            c->max_y = y;
        if (x < c->min_x || !c->started)
            c->min_x = x;
        if (y < c->min_y || !c->started)
            c->min_y = y;
        c->started = 1;
    }

    static void stbtt_csctx_v(stbtt_csctx* c, const u8 type, const i32 x, const i32 y, const i32 cx,
                              const i32 cy, const i32 cx1, const i32 cy1)
    {
        if (c->bounds) {
            stbtt_track_vertex(c, x, y);
            if (type == STBTT_vcubic) {
                stbtt_track_vertex(c, cx, cy);
                stbtt_track_vertex(c, cx1, cy1);
            }
        }
        else {
            stbtt_setvertex(&c->pvertices[c->num_vertices], type, x, y, cx, cy);
            c->pvertices[c->num_vertices].cx1 = static_cast<i16>(cx1);
            c->pvertices[c->num_vertices].cy1 = static_cast<i16>(cy1);
        }
        c->num_vertices++;
    }

    static void stbtt_csctx_close_shape(stbtt_csctx* ctx)
    {
        if (math::not_equal(ctx->first_x, ctx->x) || math::not_equal(ctx->first_y, ctx->y))
            stbtt_csctx_v(ctx, STBTT_vline, static_cast<i32>(ctx->first_x),
                          static_cast<i32>(ctx->first_y), 0, 0, 0, 0);
    }

    static void stbtt_csctx_rmove_to(stbtt_csctx* ctx, const f32 dx, const f32 dy)
    {
        stbtt_csctx_close_shape(ctx);
        ctx->first_x = ctx->x = ctx->x + dx;
        ctx->first_y = ctx->y = ctx->y + dy;
        stbtt_csctx_v(ctx, STBTT_vmove, static_cast<i32>(ctx->x), static_cast<i32>(ctx->y), 0, 0, 0,
                      0);
    }

    static void stbtt_csctx_rline_to(stbtt_csctx* ctx, const f32 dx, const f32 dy)
    {
        ctx->x += dx;
        ctx->y += dy;
        stbtt_csctx_v(ctx, STBTT_vline, static_cast<i32>(ctx->x), static_cast<i32>(ctx->y), 0, 0, 0,
                      0);
    }

    static void stbtt_csctx_rccurve_to(stbtt_csctx* ctx, const f32 dx1, const f32 dy1,
                                       const f32 dx2, const f32 dy2, const f32 dx3, const f32 dy3)
    {
        f32 cx1 = ctx->x + dx1;
        f32 cy1 = ctx->y + dy1;
        f32 cx2 = cx1 + dx2;
        f32 cy2 = cy1 + dy2;
        ctx->x = cx2 + dx3;
        ctx->y = cy2 + dy3;
        stbtt_csctx_v(ctx, STBTT_vcubic, static_cast<i32>(ctx->x), static_cast<i32>(ctx->y),
                      static_cast<i32>(cx1), static_cast<i32>(cy1), static_cast<i32>(cx2),
                      static_cast<i32>(cy2));
    }

    static stbtt_buf stbtt_get_subr(stbtt_buf idx, i32 n)
    {
        i32 count = stbtt_cff_index_count(&idx);
        i32 bias = 107;
        if (count >= 33900)
            bias = 32768;
        else if (count >= 1240)
            bias = 1131;
        n += bias;
        if (n < 0 || n >= count)
            return stbtt_new_buf(nullptr, 0);
        return stbtt_cff_index_get(idx, n);
    }

    static stbtt_buf stbtt_cid_get_glyph_subrs(const stbtt_fontinfo* info, const i32 glyph_index)
    {
        stbtt_buf fdselect = info->fdselect;
        i32 fdselector = -1;

        stbtt_buf_seek(&fdselect, 0);
        i32 fmt = stbtt_buf_get8(&fdselect);
        if (fmt == 0) {
            // untested
            stbtt_buf_skip(&fdselect, glyph_index);
            fdselector = stbtt_buf_get8(&fdselect);
        }
        else if (fmt == 3) {
            i32 nranges = stbtt_buf_get16(&fdselect);
            i32 start = stbtt_buf_get16(&fdselect);
            for (i32 i = 0; i < nranges; i++) {
                i32 v = stbtt_buf_get8(&fdselect);
                i32 end = stbtt_buf_get16(&fdselect);
                if (glyph_index >= start && glyph_index < end) {
                    fdselector = v;
                    break;
                }
                start = end;
            }
        }
        if (fdselector == -1)
            stbtt_new_buf(nullptr, 0);
        return stbtt_get_subrs(info->cff, stbtt_cff_index_get(info->fontdicts, fdselector));
    }

    static i32 stbtt_run_charstring(const stbtt_fontinfo* info, i32 glyph_index, stbtt_csctx* c)
    {
        i32 in_header = 1, maskbits = 0, subr_stack_height = 0, sp = 0, v, i, b0;
        i32 has_subrs = 0, clear_stack;
        f32 s[48];
        stbtt_buf subr_stack[10], subrs = info->subrs, b;
        f32 f;

#define STBTT_CSERR(s) (0)

        // this currently ignores the initial width value, which isn't needed if we have hmtx
        b = stbtt_cff_index_get(info->charstrings, glyph_index);
        while (b.cursor < b.size) {
            i = 0;
            clear_stack = 1;
            b0 = stbtt_buf_get8(&b);
            switch (b0) {
                // @TODO implement hinting
                case 0x13:  // hintmask
                case 0x14:  // cntrmask
                    if (in_header)
                        maskbits += (sp / 2);  // implicit "vstem"
                    in_header = 0;
                    stbtt_buf_skip(&b, (maskbits + 7) / 8);
                    break;

                case 0x01:  // hstem
                case 0x03:  // vstem
                case 0x12:  // hstemhm
                case 0x17:  // vstemhm
                    maskbits += (sp / 2);
                    break;

                case 0x15:  // rmoveto
                    in_header = 0;
                    if (sp < 2)
                        return STBTT_CSERR("rmoveto stack");
                    stbtt_csctx_rmove_to(c, s[sp - 2], s[sp - 1]);
                    break;
                case 0x04:  // vmoveto
                    in_header = 0;
                    if (sp < 1)
                        return STBTT_CSERR("vmoveto stack");
                    stbtt_csctx_rmove_to(c, 0, s[sp - 1]);
                    break;
                case 0x16:  // hmoveto
                    in_header = 0;
                    if (sp < 1)
                        return STBTT_CSERR("hmoveto stack");
                    stbtt_csctx_rmove_to(c, s[sp - 1], 0);
                    break;

                case 0x05:  // rlineto
                    if (sp < 2)
                        return STBTT_CSERR("rlineto stack");
                    for (; i + 1 < sp; i += 2)
                        stbtt_csctx_rline_to(c, s[i], s[i + 1]);
                    break;

                    // hlineto/vlineto and vhcurveto/hvcurveto alternate horizontal and vertical
                    // starting from a different place.

                case 0x07:  // vlineto
                    if (sp < 1)
                        return STBTT_CSERR("vlineto stack");
                    goto vlineto;
                case 0x06:  // hlineto
                    if (sp < 1)
                        return STBTT_CSERR("hlineto stack");
                    for (;;) {
                        if (i >= sp)
                            break;
                        stbtt_csctx_rline_to(c, s[i], 0);
                        i++;
                    vlineto:
                        if (i >= sp)
                            break;
                        stbtt_csctx_rline_to(c, 0, s[i]);
                        i++;
                    }
                    break;

                case 0x1F:  // hvcurveto
                    if (sp < 4)
                        return STBTT_CSERR("hvcurveto stack");
                    goto hvcurveto;
                case 0x1E:  // vhcurveto
                    if (sp < 4)
                        return STBTT_CSERR("vhcurveto stack");
                    for (;;) {
                        if (i + 3 >= sp)
                            break;
                        stbtt_csctx_rccurve_to(c, 0, s[i], s[i + 1], s[i + 2], s[i + 3],
                                               (sp - i == 5) ? s[i + 4] : 0.0f);
                        i += 4;
                    hvcurveto:
                        if (i + 3 >= sp)
                            break;
                        stbtt_csctx_rccurve_to(c, s[i], 0, s[i + 1], s[i + 2],
                                               (sp - i == 5) ? s[i + 4] : 0.0f, s[i + 3]);
                        i += 4;
                    }
                    break;

                case 0x08:  // rrcurveto
                    if (sp < 6)
                        return STBTT_CSERR("rcurveline stack");
                    for (; i + 5 < sp; i += 6)
                        stbtt_csctx_rccurve_to(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4],
                                               s[i + 5]);
                    break;

                case 0x18:  // rcurveline
                    if (sp < 8)
                        return STBTT_CSERR("rcurveline stack");
                    for (; i + 5 < sp - 2; i += 6)
                        stbtt_csctx_rccurve_to(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4],
                                               s[i + 5]);
                    if (i + 1 >= sp)
                        return STBTT_CSERR("rcurveline stack");
                    stbtt_csctx_rline_to(c, s[i], s[i + 1]);
                    break;

                case 0x19:  // rlinecurve
                    if (sp < 8)
                        return STBTT_CSERR("rlinecurve stack");
                    for (; i + 1 < sp - 6; i += 2)
                        stbtt_csctx_rline_to(c, s[i], s[i + 1]);
                    if (i + 5 >= sp)
                        return STBTT_CSERR("rlinecurve stack");
                    stbtt_csctx_rccurve_to(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4],
                                           s[i + 5]);
                    break;

                case 0x1A:  // vvcurveto
                case 0x1B:  // hhcurveto
                    if (sp < 4)
                        return STBTT_CSERR("(vv|hh)curveto stack");
                    f = 0.0;
                    if (sp & 1) {
                        f = s[i];
                        i++;
                    }
                    for (; i + 3 < sp; i += 4) {
                        if (b0 == 0x1B)
                            stbtt_csctx_rccurve_to(c, s[i], f, s[i + 1], s[i + 2], s[i + 3], 0.0);
                        else
                            stbtt_csctx_rccurve_to(c, f, s[i], s[i + 1], s[i + 2], 0.0, s[i + 3]);
                        f = 0.0;
                    }
                    break;

                case 0x0A:  // callsubr
                    if (!has_subrs) {
                        if (info->fdselect.size)
                            subrs = stbtt_cid_get_glyph_subrs(info, glyph_index);
                        has_subrs = 1;
                    }
                    [[fallthrough]];
                case 0x1D:  // callgsubr
                    if (sp < 1)
                        return STBTT_CSERR("call(g|)subr stack");
                    v = static_cast<i32>(s[--sp]);
                    if (subr_stack_height >= 10)
                        return STBTT_CSERR("recursion limit");
                    subr_stack[subr_stack_height++] = b;
                    b = stbtt_get_subr(b0 == 0x0A ? subrs : info->gsubrs, v);
                    if (b.size == 0)
                        return STBTT_CSERR("subr not found");
                    b.cursor = 0;
                    clear_stack = 0;
                    break;

                case 0x0B:  // return
                    if (subr_stack_height <= 0)
                        return STBTT_CSERR("return outside subr");
                    b = subr_stack[--subr_stack_height];
                    clear_stack = 0;
                    break;

                case 0x0E:  // endchar
                    stbtt_csctx_close_shape(c);
                    return 1;

                case 0x0C:
                {  // two-byte escape
                    f32 dx1, dx2, dx3, dx4, dx5, dx6, dy1, dy2, dy3, dy4, dy5, dy6;
                    f32 dx, dy;
                    i32 b1 = stbtt_buf_get8(&b);
                    switch (b1) {
                        // @TODO These "flex" implementations ignore the flex-depth and resolution,
                        // and always draw beziers.
                        case 0x22:  // hflex
                            if (sp < 7)
                                return STBTT_CSERR("hflex stack");
                            dx1 = s[0];
                            dx2 = s[1];
                            dy2 = s[2];
                            dx3 = s[3];
                            dx4 = s[4];
                            dx5 = s[5];
                            dx6 = s[6];
                            stbtt_csctx_rccurve_to(c, dx1, 0, dx2, dy2, dx3, 0);
                            stbtt_csctx_rccurve_to(c, dx4, 0, dx5, -dy2, dx6, 0);
                            break;

                        case 0x23:  // flex
                            if (sp < 13)
                                return STBTT_CSERR("flex stack");
                            dx1 = s[0];
                            dy1 = s[1];
                            dx2 = s[2];
                            dy2 = s[3];
                            dx3 = s[4];
                            dy3 = s[5];
                            dx4 = s[6];
                            dy4 = s[7];
                            dx5 = s[8];
                            dy5 = s[9];
                            dx6 = s[10];
                            dy6 = s[11];
                            // fd is s[12]
                            stbtt_csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
                            stbtt_csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
                            break;

                        case 0x24:  // hflex1
                            if (sp < 9)
                                return STBTT_CSERR("hflex1 stack");
                            dx1 = s[0];
                            dy1 = s[1];
                            dx2 = s[2];
                            dy2 = s[3];
                            dx3 = s[4];
                            dx4 = s[5];
                            dx5 = s[6];
                            dy5 = s[7];
                            dx6 = s[8];
                            stbtt_csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, 0);
                            stbtt_csctx_rccurve_to(c, dx4, 0, dx5, dy5, dx6, -(dy1 + dy2 + dy5));
                            break;

                        case 0x25:  // flex1
                            if (sp < 11)
                                return STBTT_CSERR("flex1 stack");
                            dx1 = s[0];
                            dy1 = s[1];
                            dx2 = s[2];
                            dy2 = s[3];
                            dx3 = s[4];
                            dy3 = s[5];
                            dx4 = s[6];
                            dy4 = s[7];
                            dx5 = s[8];
                            dy5 = s[9];
                            dx6 = dy6 = s[10];
                            dx = dx1 + dx2 + dx3 + dx4 + dx5;
                            dy = dy1 + dy2 + dy3 + dy4 + dy5;
                            if (std::fabs(dx) > std::fabs(dy))
                                dy6 = -dy;
                            else
                                dx6 = -dx;
                            stbtt_csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
                            stbtt_csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
                            break;

                        default:
                            return STBTT_CSERR("unimplemented");
                    }
                } break;

                default:
                    if (b0 != 255 && b0 != 28 && b0 < 32)
                        return STBTT_CSERR("reserved operator");

                    // push immediate
                    if (b0 == 255) {
                        f = static_cast<f32>(static_cast<i32>(stbtt_buf_get32(&b))) / 0x10000;
                    }
                    else {
                        stbtt_buf_skip(&b, -1);
                        f = static_cast<f32>(static_cast<i16>(stbtt_cff_int(&b)));
                    }
                    if (sp >= 48)
                        return STBTT_CSERR("push stack overflow");
                    s[sp++] = f;
                    clear_stack = 0;
                    break;
            }
            if (clear_stack)
                sp = 0;
        }
        return STBTT_CSERR("no endchar");

#undef STBTT__CSERR
    }

    static i32 stbtt_GetGlyphShapeT2(const stbtt_fontinfo* info, const i32 glyph_index,
                                     stbtt_vertex** pvertices)
    {
        // runs the charstring twice, once to count and once to output (to avoid realloc)
        stbtt_csctx count_ctx = STBTT_CSCTX_INIT(1);
        stbtt_csctx output_ctx = STBTT_CSCTX_INIT(0);
        if (stbtt_run_charstring(info, glyph_index, &count_ctx)) {
            *pvertices = static_cast<stbtt_vertex*>(
                std::malloc(count_ctx.num_vertices * sizeof(stbtt_vertex)));
            output_ctx.pvertices = *pvertices;
            if (stbtt_run_charstring(info, glyph_index, &output_ctx)) {
                debug_assert(output_ctx.num_vertices == count_ctx.num_vertices);
                return output_ctx.num_vertices;
            }
        }
        *pvertices = nullptr;
        return 0;
    }

    static i32 stbtt_get_glyph_info_t2(const stbtt_fontinfo* info, const i32 glyph_index, i32* x0,
                                       i32* y0, i32* x1, i32* y1)
    {
        stbtt_csctx c = STBTT_CSCTX_INIT(1);
        i32 r = stbtt_run_charstring(info, glyph_index, &c);
        if (x0)
            *x0 = r ? c.min_x : 0;
        if (y0)
            *y0 = r ? c.min_y : 0;
        if (x1)
            *x1 = r ? c.max_x : 0;
        if (y1)
            *y1 = r ? c.max_y : 0;
        return r ? c.num_vertices : 0;
    }

    i32 stbtt_get_glyph_shape(const stbtt_fontinfo* info, const i32 glyph_index,
                              stbtt_vertex** pvertices)
    {
        if (!info->cff.size)
            return stbtt_get_glyph_shape_tt(info, glyph_index, pvertices);
        return stbtt_GetGlyphShapeT2(info, glyph_index, pvertices);
    }

    void stbtt_get_glyph_h_metrics(const stbtt_fontinfo* info, const i32 glyph_index,
                                   i32* advance_width, i32* left_side_bearing)
    {
        u16 numOfLongHorMetrics = tt_aligned_u16(info->data + info->hhea + 34);
        if (glyph_index < numOfLongHorMetrics) {
            if (advance_width)
                *advance_width = tt_aligned_i16(info->data + info->hmtx + 4 * glyph_index);
            if (left_side_bearing)
                *left_side_bearing = tt_aligned_i16(info->data + info->hmtx + 4 * glyph_index + 2);
        }
        else {
            if (advance_width)
                *advance_width = tt_aligned_i16(
                    info->data + info->hmtx + 4 * (numOfLongHorMetrics - 1));
            if (left_side_bearing)
                *left_side_bearing = tt_aligned_i16(info->data + info->hmtx +
                                                    4 * numOfLongHorMetrics +
                                                    2 * (glyph_index - numOfLongHorMetrics));
        }
    }

    i32 stbtt_GetKerningTableLength(const stbtt_fontinfo* info)
    {
        u8* data = info->data + info->kern;

        // we only look at the first table. it must be 'horizontal' and format 0.
        if (!info->kern)
            return 0;
        if (tt_aligned_u16(data + 2) < 1)  // number of tables, need at least 1
            return 0;
        if (tt_aligned_u16(data + 8) != 1)  // horizontal flag must be set in format
            return 0;

        return tt_aligned_u16(data + 10);
    }

    i32 stbtt_GetKerningTable(const stbtt_fontinfo* info, stbtt_kerningentry* table,
                              const i32 table_length)
    {
        u8* data = info->data + info->kern;

        // we only look at the first table. it must be 'horizontal' and format 0.
        if (!info->kern)
            return 0;
        if (tt_aligned_u16(data + 2) < 1)  // number of tables, need at least 1
            return 0;
        if (tt_aligned_u16(data + 8) != 1)  // horizontal flag must be set in format
            return 0;

        i32 length = tt_aligned_u16(data + 10);
        if (table_length < length)
            length = table_length;

        for (i32 k = 0; k < length; k++) {
            table[k].glyph1 = tt_aligned_u16(data + 18 + (k * 6));
            table[k].glyph2 = tt_aligned_u16(data + 20 + (k * 6));
            table[k].advance = tt_aligned_i16(data + 22 + (k * 6));
        }

        return length;
    }

    static i32 stbtt_GetGlyphKernInfoAdvance(const stbtt_fontinfo* info, const i32 glyph1,
                                             const i32 glyph2)
    {
        u8* data = info->data + info->kern;

        // we only look at the first table. it must be 'horizontal' and format 0.
        if (!info->kern)
            return 0;
        if (tt_aligned_u16(data + 2) < 1)  // number of tables, need at least 1
            return 0;
        if (tt_aligned_u16(data + 8) != 1)  // horizontal flag must be set in format
            return 0;

        i32 l = 0;
        i32 r = tt_aligned_u16(data + 10) - 1;
        u32 needle = glyph1 << 16 | glyph2;
        while (l <= r) {
            i32 m = (l + r) >> 1;
            u32 straw = tt_aligned_u32(data + 18 + (m * 6));  // note: unaligned read
            if (needle < straw)
                r = m - 1;
            else if (needle > straw)
                l = m + 1;
            else
                return tt_aligned_i16(data + 22 + (m * 6));
        }
        return 0;
    }

    static i32 stbtt_GetCoverageIndex(u8* coverageTable, const i32 glyph)
    {
        u16 coverageFormat = tt_aligned_u16(coverageTable);
        switch (coverageFormat) {
            case 1:
            {
                u16 glyphCount = tt_aligned_u16(coverageTable + 2);

                // Binary search.
                i32 l = 0, r = glyphCount - 1;
                i32 needle = glyph;
                while (l <= r) {
                    u8* glyphArray = coverageTable + 4;
                    i32 m = (l + r) >> 1;
                    u16 glyphID = tt_aligned_u16(glyphArray + 2 * m);
                    i32 straw = glyphID;
                    if (needle < straw)
                        r = m - 1;
                    else if (needle > straw)
                        l = m + 1;
                    else
                        return m;
                }
                break;
            }

            case 2:
            {
                u16 rangeCount = tt_aligned_u16(coverageTable + 2);
                u8* rangeArray = coverageTable + 4;

                // Binary search.
                i32 l = 0, r = rangeCount - 1;
                i32 needle = glyph;
                while (l <= r) {
                    i32 m = (l + r) >> 1;
                    u8* rangeRecord = rangeArray + 6 * m;
                    i32 strawStart = tt_aligned_u16(rangeRecord);
                    i32 strawEnd = tt_aligned_u16(rangeRecord + 2);
                    if (needle < strawStart)
                        r = m - 1;
                    else if (needle > strawEnd)
                        l = m + 1;
                    else {
                        u16 startCoverageIndex = tt_aligned_u16(rangeRecord + 4);
                        return startCoverageIndex + glyph - strawStart;
                    }
                }
                break;
            }

            default:
                return -1;  // unsupported
        }

        return -1;
    }

    static i32 stbtt_GetGlyphClass(u8* classDefTable, const i32 glyph)
    {
        u16 classDefFormat = tt_aligned_u16(classDefTable);
        switch (classDefFormat) {
            case 1:
            {
                u16 startGlyphID = tt_aligned_u16(classDefTable + 2);
                u16 glyphCount = tt_aligned_u16(classDefTable + 4);
                u8* classDef1ValueArray = classDefTable + 6;

                if (glyph >= startGlyphID && glyph < startGlyphID + glyphCount)
                    return tt_aligned_u16(classDef1ValueArray + 2 * (glyph - startGlyphID));
                break;
            }

            case 2:
            {
                u16 classRangeCount = tt_aligned_u16(classDefTable + 2);
                u8* classRangeRecords = classDefTable + 4;

                // Binary search.
                i32 l = 0, r = classRangeCount - 1;
                i32 needle = glyph;
                while (l <= r) {
                    i32 m = (l + r) >> 1;
                    u8* classRangeRecord = classRangeRecords + 6 * m;
                    i32 strawStart = tt_aligned_u16(classRangeRecord);
                    i32 strawEnd = tt_aligned_u16(classRangeRecord + 2);
                    if (needle < strawStart)
                        r = m - 1;
                    else if (needle > strawEnd)
                        l = m + 1;
                    else
                        return tt_aligned_u16(classRangeRecord + 4);
                }
                break;
            }

            default:
                return -1;  // Unsupported definition type, return an error.
        }

        // "All glyphs not assigned to a class fall into class 0". (OpenType spec)
        return 0;
    }

    static i32 stbtt_get_glyph_gpos_info_advance(const stbtt_fontinfo* info, const i32 glyph1,
                                                 const i32 glyph2)
    {
        if (!info->gpos)
            return 0;

        u8* data = info->data + info->gpos;

        if (tt_aligned_u16(data + 0) != 1)
            return 0;  // Major version 1
        if (tt_aligned_u16(data + 2) != 0)
            return 0;  // Minor version 0

        u16 lookup_list_offset = tt_aligned_u16(data + 8);
        u8* lookup_list = data + lookup_list_offset;
        u16 lookup_count = tt_aligned_u16(lookup_list);

        for (i32 i = 0; i < lookup_count; ++i) {
            u16 lookupOffset = tt_aligned_u16(lookup_list + 2 + 2 * i);
            u8* lookup_table = lookup_list + lookupOffset;

            u16 lookup_type = tt_aligned_u16(lookup_table);
            u16 sub_table_count = tt_aligned_u16(lookup_table + 4);
            u8* sub_table_offsets = lookup_table + 6;
            if (lookup_type != 2)  // Pair Adjustment Positioning Subtable
                continue;

            for (i32 sti = 0; sti < sub_table_count; sti++) {
                u16 subtableOffset = tt_aligned_u16(sub_table_offsets + 2 * sti);
                u8* table = lookup_table + subtableOffset;
                u16 posFormat = tt_aligned_u16(table);
                u16 coverageOffset = tt_aligned_u16(table + 2);
                i32 coverageIndex = stbtt_GetCoverageIndex(table + coverageOffset, glyph1);
                if (coverageIndex == -1)
                    continue;

                switch (posFormat) {
                    case 1:
                    {
                        u16 value_format1 = tt_aligned_u16(table + 4);
                        u16 value_format2 = tt_aligned_u16(table + 6);
                        if (value_format1 == 4 && value_format2 == 0) {  // Support more formats?
                            i32 value_record_pair_size_in_bytes = 2;
                            u16 pair_set_count = tt_aligned_u16(table + 8);
                            u16 pair_pos_offset = tt_aligned_u16(table + 10 + 2 * coverageIndex);
                            u8* pair_value_table = table + pair_pos_offset;
                            u16 pair_value_count = tt_aligned_u16(pair_value_table);
                            u8* pair_value_array = pair_value_table + 2;

                            if (coverageIndex >= pair_set_count)
                                return 0;

                            i32 needle = glyph2;
                            i32 r = pair_value_count - 1;
                            i32 l = 0;

                            // Binary search.
                            while (l <= r) {
                                i32 m = (l + r) >> 1;
                                u8* pairValue = pair_value_array +
                                                (2 + value_record_pair_size_in_bytes) * m;
                                u16 secondGlyph = tt_aligned_u16(pairValue);
                                i32 straw = secondGlyph;
                                if (needle < straw)
                                    r = m - 1;
                                else if (needle > straw)
                                    l = m + 1;
                                else {
                                    i16 xAdvance = tt_aligned_i16(pairValue + 2);
                                    return xAdvance;
                                }
                            }
                        }
                        else
                            return 0;
                        break;
                    }

                    case 2:
                    {
                        u16 valueFormat1 = tt_aligned_u16(table + 4);
                        u16 valueFormat2 = tt_aligned_u16(table + 6);
                        if (valueFormat1 == 4 && valueFormat2 == 0) {  // Support more formats?
                            u16 classDef1Offset = tt_aligned_u16(table + 8);
                            u16 classDef2Offset = tt_aligned_u16(table + 10);
                            i32 glyph1class = stbtt_GetGlyphClass(table + classDef1Offset, glyph1);
                            i32 glyph2class = stbtt_GetGlyphClass(table + classDef2Offset, glyph2);

                            u16 class1Count = tt_aligned_u16(table + 12);
                            u16 class2Count = tt_aligned_u16(table + 14);

                            if (glyph1class < 0 || glyph1class >= class1Count)
                                return 0;  // malformed
                            if (glyph2class < 0 || glyph2class >= class2Count)
                                return 0;  // malformed

                            u8* class1Records = table + 16;
                            u8* class2Records = class1Records + 2 * (glyph1class * class2Count);
                            i16 xAdvance = tt_aligned_i16(class2Records + 2 * glyph2class);
                            return xAdvance;
                        }
                        else
                            return 0;
                    }

                    default:
                        return 0;  // Unsupported position format
                }
            }
        }

        return 0;
    }

    i32 stbtt_get_glyph_kern_advance(const stbtt_fontinfo* info, const i32 g1, const i32 g2)
    {
        i32 xAdvance = 0;

        if (info->gpos)
            xAdvance += stbtt_get_glyph_gpos_info_advance(info, g1, g2);
        else if (info->kern)
            xAdvance += stbtt_GetGlyphKernInfoAdvance(info, g1, g2);

        return xAdvance;
    }

    i32 stbtt_get_codepoint_kern_advance(const stbtt_fontinfo* info, const i32 ch1, const i32 ch2)
    {
        if (!info->kern && !info->gpos)  // if no kerning table, don't waste time looking up both
                                         // codepoint->glyphs
            return 0;
        return stbtt_get_glyph_kern_advance(info, stbtt_find_glyph_index(info, ch1),
                                            stbtt_find_glyph_index(info, ch2));
    }

    void stbtt_get_codepoint_h_metrics(const stbtt_fontinfo* info, const i32 codepoint,
                                       i32* advanceWidth, i32* leftSideBearing)
    {
        stbtt_get_glyph_h_metrics(info, stbtt_find_glyph_index(info, codepoint), advanceWidth,
                                  leftSideBearing);
    }

    void stbtt_get_font_v_metrics(const stbtt_fontinfo* info, i32* ascent, i32* descent,
                                  i32* lineGap)
    {
        if (ascent != nullptr)
            *ascent = tt_aligned_i16(info->data + info->hhea + 4);
        if (descent != nullptr)
            *descent = tt_aligned_i16(info->data + info->hhea + 6);
        if (lineGap != nullptr)
            *lineGap = tt_aligned_i16(info->data + info->hhea + 8);
    }

    i32 stbtt_get_font_v_metrics_os2(const stbtt_fontinfo* info, i32* typoAscent, i32* typoDescent,
                                     i32* typoLineGap)
    {
        u32 tab = stbtt_find_table(info->data, info->fontstart, "OS/2");
        if (tab == 0)
            return 0;

        if (typoAscent != nullptr)
            *typoAscent = tt_aligned_i16(info->data + tab + 68);
        if (typoDescent != nullptr)
            *typoDescent = tt_aligned_i16(info->data + tab + 70);
        if (typoLineGap != nullptr)
            *typoLineGap = tt_aligned_i16(info->data + tab + 72);

        return 1;
    }

    void stbtt_get_font_bounding_box(const stbtt_fontinfo* info, i32* x0, i32* y0, i32* x1, i32* y1)
    {
        *x0 = tt_aligned_i16(info->data + info->head + 36);
        *y0 = tt_aligned_i16(info->data + info->head + 38);
        *x1 = tt_aligned_i16(info->data + info->head + 40);
        *y1 = tt_aligned_i16(info->data + info->head + 42);
    }

    f32 stbtt_scale_for_pixel_height(const stbtt_fontinfo* info, const f32 pixel_height)
    {
        i32 fheight = tt_aligned_i16(info->data + info->hhea + 4) -
                      tt_aligned_i16(info->data + info->hhea + 6);
        return static_cast<f32>(pixel_height) / fheight;
    }

    f32 stbtt_scale_for_mapping_em_to_pixels(const stbtt_fontinfo* info, const f32 pixels)
    {
        i32 units_per_em = tt_aligned_u16(info->data + info->head + 18);
        return pixels / units_per_em;
    }

    void sbtt_free_shape(const stbtt_fontinfo*, stbtt_vertex* v)
    {
        std::free(v);
    }

    u8* stbtt_find_svg_doc(const stbtt_fontinfo* info, const i32 gl)
    {
        u8* data = info->data;
        u8* svg_doc_list = data + stbtt_get_svg(const_cast<stbtt_fontinfo*>(info));

        i32 num_entries = tt_aligned_u16(svg_doc_list);
        u8* svg_docs = svg_doc_list + 2;

        for (i32 i = 0; i < num_entries; i++) {
            u8* svg_doc = svg_docs + (12 * i);
            if ((gl >= tt_aligned_u16(svg_doc)) && (gl <= tt_aligned_u16(svg_doc + 2)))
                return svg_doc;
        }
        return nullptr;
    }

    i32 stbtt_get_glyph_svg(const stbtt_fontinfo* info, const i32 gl, const char** svg)
    {
        u8* data = info->data;

        if (info->svg == 0)
            return 0;

        u8* svg_doc = stbtt_find_svg_doc(info, gl);
        if (svg_doc != nullptr) {
            *svg = (char*)data + info->svg + tt_aligned_u32(svg_doc + 4);
            return tt_aligned_u32(svg_doc + 8);
        }
        else {
            return 0;
        }
    }

    i32 stbtt_get_codepoint_svg(const stbtt_fontinfo* info, const i32 unicode_codepoint,
                                const char** svg)
    {
        return stbtt_get_glyph_svg(info, stbtt_find_glyph_index(info, unicode_codepoint), svg);
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // antialiasing software rasterizer
    //

    void stbtt_get_glyph_bitmap_box_subpixel(
        const stbtt_fontinfo* font, const i32 glyph, const f32 scale_x, const f32 scale_y,
        const f32 shift_x, const f32 shift_y, i32* ix0, i32* iy0, i32* ix1, i32* iy1)
    {
        i32 x0{ 0 };
        i32 y0{ 0 };
        i32 x1{ 0 };
        i32 y1{ 0 };
        if (!stbtt_get_glyph_box(font, glyph, &x0, &y0, &x1, &y1)) {
            // e.g. space character
            if (ix0 != nullptr)
                *ix0 = 0;
            if (iy0 != nullptr)
                *iy0 = 0;
            if (ix1 != nullptr)
                *ix1 = 0;
            if (iy1 != nullptr)
                *iy1 = 0;
        }
        else {
            // move to integral bboxes (treating pixels as little squares, what pixels get touched)?
            if (ix0 != nullptr)
                *ix0 = std::lrintf(std::floorf(x0 * scale_x + shift_x));
            if (iy0 != nullptr)
                *iy0 = std::lrintf(std::floorf(-y1 * scale_y + shift_y));
            if (ix1 != nullptr)
                *ix1 = std::lrintf(std::ceilf(x1 * scale_x + shift_x));
            if (iy1 != nullptr)
                *iy1 = std::lrintf(std::ceilf(-y0 * scale_y + shift_y));
        }
    }

    void stbtt_get_glyph_bitmap_box(const stbtt_fontinfo* font, const i32 glyph, const f32 scale_x,
                                    const f32 scale_y, i32* ix0, i32* iy0, i32* ix1, i32* iy1)
    {
        stbtt_get_glyph_bitmap_box_subpixel(font, glyph, scale_x, scale_y, 0.0f, 0.0f, ix0, iy0,
                                            ix1, iy1);
    }

    void stbtt_get_codepoint_bitmap_box_subpixel(
        const stbtt_fontinfo* font, const i32 codepoint, const f32 scale_x, const f32 scale_y,
        const f32 shift_x, const f32 shift_y, i32* ix0, i32* iy0, i32* ix1, i32* iy1)
    {
        stbtt_get_glyph_bitmap_box_subpixel(font, stbtt_find_glyph_index(font, codepoint), scale_x,
                                            scale_y, shift_x, shift_y, ix0, iy0, ix1, iy1);
    }

    void stbtt_get_codepoint_bitmap_box(const stbtt_fontinfo* font, const i32 codepoint,
                                        const f32 scale_x, const f32 scale_y, i32* ix0, i32* iy0,
                                        i32* ix1, i32* iy1)
    {
        stbtt_get_codepoint_bitmap_box_subpixel(font, codepoint, scale_x, scale_y, 0.0f, 0.0f, ix0,
                                                iy0, ix1, iy1);
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    //  Rasterizer

    typedef struct stbtt_hheap_chunk
    {
        struct stbtt_hheap_chunk* next;
    } stbtt_hheap_chunk;

    typedef struct stbtt_hheap
    {
        struct stbtt_hheap_chunk* head;
        void* first_free;
        i32 num_remaining_in_head_chunk;
    } stbtt_hheap;

    static void* stbtt_hheap_alloc(stbtt_hheap* hh, const size_t size, const void*)
    {
        if (hh->first_free) {
            void* p = hh->first_free;
            hh->first_free = *static_cast<void**>(p);
            return p;
        }
        else {
            if (hh->num_remaining_in_head_chunk == 0) {
                i32 count{ size < 32 ? 2000 : size < 128 ? 800
                                                         : 100 };
                stbtt_hheap_chunk* c = static_cast<stbtt_hheap_chunk*>(
                    std::malloc(sizeof(stbtt_hheap_chunk) + size * count));
                if (c == nullptr)
                    return nullptr;
                c->next = hh->head;
                hh->head = c;
                hh->num_remaining_in_head_chunk = count;
            }
            --hh->num_remaining_in_head_chunk;
            return (char*)(hh->head) + sizeof(stbtt_hheap_chunk) +
                   size * hh->num_remaining_in_head_chunk;
        }
    }

    static void stbtt_hheap_free(stbtt_hheap* hh, void* p)
    {
        *static_cast<void**>(p) = hh->first_free;
        hh->first_free = p;
    }

    static void stbtt_hheap_cleanup(const stbtt_hheap* hh, const void*)
    {
        stbtt_hheap_chunk* c = hh->head;
        while (c) {
            stbtt_hheap_chunk* n = c->next;
            std::free(c);
            c = n;
        }
    }

    typedef struct stbtt_edge
    {
        f32 x0, y0, x1, y1;
        i32 invert;
    } stbtt_edge;

    typedef struct stbtt_active_edge
    {
        struct stbtt_active_edge* next;
#if STBTT_RASTERIZER_VERSION == 1
        i32 x, dx;
        f32 ey;
        i32 direction;
#elif STBTT_RASTERIZER_VERSION == 2
        f32 fx, fdx, fdy;
        f32 direction;
        f32 sy;
        f32 ey;
#else
  #error "Unrecognized value of STBTT_RASTERIZER_VERSION"
#endif
    } stbtt_active_edge;

#if STBTT_RASTERIZER_VERSION == 1
  #define STBTT_FIXSHIFT 10
  #define STBTT_FIX      (1 << STBTT_FIXSHIFT)
  #define STBTT_FIXMASK  (STBTT_FIX - 1)

    static stbtt_active_edge* stbtt_new_active(stbtt_hheap* hh, stbtt_edge* e, i32 off_x,
                                               f32 start_point, void* userdata)
    {
        stbtt_active_edge* z = (stbtt_active_edge*)stbtt_hheap_alloc(hh, sizeof(*z), userdata);
        f32 dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
        debug_assert(z != NULL);
        if (!z)
            return z;

        // round dx down to avoid overshooting
        if (dxdy < 0)
            z->dx = -std::lrintf(std::floorf(STBTT_FIX * -dxdy));
        else
            z->dx = std::lrintf(std::floorf(STBTT_FIX * dxdy));

        z->x = std::lrintf(
            std::floorf(STBTT_FIX * e->x0 + z->dx * (start_point - e->y0)));  // use z->dx so
                                                                              // when we offset
                                                                              // later it's by
                                                                              // the same amount
        z->x -= off_x * STBTT_FIX;

        z->ey = e->y1;
        z->next = 0;
        z->direction = e->invert ? 1 : -1;
        return z;
    }
#elif STBTT_RASTERIZER_VERSION == 2
    static stbtt_active_edge* stbtt_new_active(stbtt_hheap* hh, const stbtt_edge* e,
                                               const i32 off_x, const f32 start_point,
                                               void* userdata)
    {
        stbtt_active_edge* z = static_cast<stbtt_active_edge*>(
            stbtt_hheap_alloc(hh, sizeof(*z), userdata));
        f32 dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
        debug_assert(z != NULL);
        // debug_assert(e->y0 <= start_point);

        if (!z)
            return z;

        z->fdx = dxdy;
        z->fdy = dxdy != 0.0f ? (1.0f / dxdy) : 0.0f;
        z->fx = e->x0 + dxdy * (start_point - e->y0);
        z->fx -= off_x;
        z->direction = e->invert ? 1.0f : -1.0f;
        z->sy = e->y0;
        z->ey = e->y1;
        z->next = 0;
        return z;
    }
#else
  #error "Unrecognized value of STBTT_RASTERIZER_VERSION"
#endif

#if STBTT_RASTERIZER_VERSION == 1
    // note: this routine clips fills that extend off the edges... ideally this
    // wouldn't happen, but it could happen if the truetype glyph bounding boxes
    // are wrong, or if the user supplies a too-small bitmap
    static void stbtt_fill_active_edges(u8* scanline, i32 len, stbtt_active_edge* e, i32 max_weight)
    {
        // non-zero winding fill
        i32 x0 = 0, w = 0;

        while (e) {
            if (w == 0) {
                // if we're currently at zero, we need to record the edge start point
                x0 = e->x;
                w += e->direction;
            }
            else {
                i32 x1 = e->x;
                w += e->direction;
                // if we went to zero, we need to draw
                if (w == 0) {
                    i32 i = x0 >> STBTT_FIXSHIFT;
                    i32 j = x1 >> STBTT_FIXSHIFT;

                    if (i < len && j >= 0) {
                        if (i == j) {
                            // x0,x1 are the same pixel, so compute combined coverage
                            scanline[i] = scanline[i] +
                                          (u8)((x1 - x0) * max_weight >> STBTT_FIXSHIFT);
                        }
                        else {
                            if (i >= 0)  // add antialiasing for x0
                                scanline[i] = scanline[i] +
                                              (u8)(((STBTT_FIX - (x0 & STBTT_FIXMASK)) *
                                                    max_weight) >>
                                                   STBTT_FIXSHIFT);
                            else
                                i = -1;  // clip

                            if (j < len)  // add antialiasing for x1
                                scanline[j] = scanline[j] +
                                              (u8)(((x1 & STBTT_FIXMASK) * max_weight) >>
                                                   STBTT_FIXSHIFT);
                            else
                                j = len;  // clip

                            for (++i; i < j; ++i)  // fill pixels between x0 and x1
                                scanline[i] = scanline[i] + (u8)max_weight;
                        }
                    }
                }
            }

            e = e->next;
        }
    }

    static void stbtt_rasterize_sorted_edges(stbtt_bitmap* result, stbtt_edge* e, i32 n,
                                             i32 vsubsample, i32 off_x, i32 off_y, void* userdata)
    {
        stbtt_hheap hh = { 0, 0, 0 };
        stbtt_active_edge* active = NULL;
        i32 y, j = 0;
        i32 max_weight = (255 / vsubsample);  // weight per vertical scanline
        i32 s;                                // vertical subsample index
        u8 scanline_data[512], *scanline;

        if (result->w > 512)
            scanline = (u8*)std::malloc(result->w);
        else
            scanline = scanline_data;

        y = off_y * vsubsample;
        e[n].y0 = (off_y + result->h) * (f32)vsubsample + 1;

        while (j < result->h) {
            std::memset(scanline, 0, result->w);
            for (s = 0; s < vsubsample; ++s) {
                // find center of pixel for this scanline
                f32 scan_y = y + 0.5f;
                stbtt_active_edge** step = &active;

                // update all active edges;
                // remove all active edges that terminate before the center of this scanline
                while (*step) {
                    stbtt_active_edge* z = *step;
                    if (z->ey <= scan_y) {
                        *step = z->next;  // delete from list
                        debug_assert(z->direction);
                        z->direction = 0;
                        stbtt_hheap_free(&hh, z);
                    }
                    else {
                        z->x += z->dx;            // advance to position for current scanline
                        step = &((*step)->next);  // advance through list
                    }
                }

                // resort the list if needed
                for (;;) {
                    i32 changed = 0;
                    step = &active;
                    while (*step && (*step)->next) {
                        if ((*step)->x > (*step)->next->x) {
                            stbtt_active_edge* t = *step;
                            stbtt_active_edge* q = t->next;

                            t->next = q->next;
                            q->next = t;
                            *step = q;
                            changed = 1;
                        }
                        step = &(*step)->next;
                    }
                    if (!changed)
                        break;
                }

                // insert all edges that start before the center of this scanline -- omit ones that
                // also end on this scanline
                while (e->y0 <= scan_y) {
                    if (e->y1 > scan_y) {
                        stbtt_active_edge* z = stbtt_new_active(&hh, e, off_x, scan_y, userdata);
                        if (z != NULL) {
                            // find insertion point
                            if (active == NULL)
                                active = z;
                            else if (z->x < active->x) {
                                // insert at front
                                z->next = active;
                                active = z;
                            }
                            else {
                                // find thing to insert AFTER
                                stbtt_active_edge* p = active;
                                while (p->next && p->next->x < z->x)
                                    p = p->next;
                                // at this point, p->next->x is NOT < z->x
                                z->next = p->next;
                                p->next = z;
                            }
                        }
                    }
                    ++e;
                }

                // now process all active edges in XOR fashion
                if (active)
                    stbtt_fill_active_edges(scanline, result->w, active, max_weight);

                ++y;
            }
            std::memcpy(result->pixels + j * result->stride, scanline, result->w);
            ++j;
        }

        stbtt_hheap_cleanup(&hh, userdata);

        if (scanline != scanline_data)
            std::free(scanline);
    }

#elif STBTT_RASTERIZER_VERSION == 2

    // the edge passed in here does not cross the vertical line at x or the vertical line at x+1
    // (i.e. it has already been clipped to those)
    static void stbtt_handle_clipped_edge(f32* scanline, const i32 x, const stbtt_active_edge* e,
                                          f32 x0, f32 y0, f32 x1, f32 y1)
    {
        if (y0 == y1)
            return;
        debug_assert(y0 < y1);
        debug_assert(e->sy <= e->ey);
        if (y0 > e->ey)
            return;
        if (y1 < e->sy)
            return;
        if (y0 < e->sy) {
            x0 += (x1 - x0) * (e->sy - y0) / (y1 - y0);
            y0 = e->sy;
        }
        if (y1 > e->ey) {
            x1 += (x1 - x0) * (e->ey - y1) / (y1 - y0);
            y1 = e->ey;
        }

        if (x0 == x)
            debug_assert(x1 <= x + 1);
        else if (x0 == x + 1)
            debug_assert(x1 >= x);
        else if (x0 <= x)
            debug_assert(x1 <= x);
        else if (x0 >= x + 1)
            debug_assert(x1 >= x + 1);
        else
            debug_assert(x1 >= x && x1 <= x + 1);

        if (x0 <= x && x1 <= x)
            scanline[x] += e->direction * (y1 - y0);
        else if (x0 >= x + 1 && x1 >= x + 1)
            ;
        else {
            debug_assert(x0 >= x && x0 <= x + 1 && x1 >= x && x1 <= x + 1);
            scanline[x] += e->direction * (y1 - y0) * (1 - ((x0 - x) + (x1 - x)) / 2);  // coverage
                                                                                        // = 1 -
                                                                                        // average x
                                                                                        // position
        }
    }

    static f32 stbtt_sized_trapezoid_area(const f32 height, const f32 top_width,
                                          const f32 bottom_width)
    {
        debug_assert(top_width >= 0);
        debug_assert(bottom_width >= 0);
        return (top_width + bottom_width) / 2.0f * height;
    }

    static f32 stbtt_position_trapezoid_area(const f32 height, const f32 tx0, const f32 tx1,
                                             const f32 bx0, const f32 bx1)
    {
        return stbtt_sized_trapezoid_area(height, tx1 - tx0, bx1 - bx0);
    }

    static f32 stbtt_sized_triangle_area(const f32 height, const f32 width)
    {
        return height * width / 2;
    }

    static void stbtt_fill_active_edges_new(f32* scanline, f32* scanline_fill, const i32 len,
                                            stbtt_active_edge* e, const f32 y_top)
    {
        f32 y_bottom = y_top + 1;

        while (e) {
            // brute force every pixel

            // compute intersection points with top & bottom
            debug_assert(e->ey >= y_top);

            if (e->fdx == 0) {
                f32 x0 = e->fx;
                if (x0 < len) {
                    if (x0 >= 0) {
                        stbtt_handle_clipped_edge(scanline, static_cast<i32>(x0), e, x0, y_top, x0,
                                                  y_bottom);
                        stbtt_handle_clipped_edge(scanline_fill - 1, static_cast<i32>(x0) + 1, e,
                                                  x0, y_top, x0, y_bottom);
                    }
                    else {
                        stbtt_handle_clipped_edge(scanline_fill - 1, 0, e, x0, y_top, x0, y_bottom);
                    }
                }
            }
            else {
                f32 x0 = e->fx;
                f32 dx = e->fdx;
                f32 xb = x0 + dx;
                f32 x_top, x_bottom;
                f32 sy0, sy1;
                f32 dy = e->fdy;
                debug_assert(e->sy <= y_bottom && e->ey >= y_top);

                // compute endpoints of line segment clipped to this scanline (if the
                // line segment starts on this scanline. x0 is the intersection of the
                // line with y_top, but that may be off the line segment.
                if (e->sy > y_top) {
                    x_top = x0 + dx * (e->sy - y_top);
                    sy0 = e->sy;
                }
                else {
                    x_top = x0;
                    sy0 = y_top;
                }
                if (e->ey < y_bottom) {
                    x_bottom = x0 + dx * (e->ey - y_top);
                    sy1 = e->ey;
                }
                else {
                    x_bottom = xb;
                    sy1 = y_bottom;
                }

                if (x_top >= 0 && x_bottom >= 0 && x_top < len && x_bottom < len) {
                    // from here on, we don't have to range check x values

                    if (static_cast<i32>(x_top) == static_cast<i32>(x_bottom)) {
                        // simple case, only spans one pixel
                        i32 x = static_cast<i32>(x_top);
                        f32 height = (sy1 - sy0) * e->direction;
                        debug_assert(x >= 0 && x < len);
                        scanline[x] += stbtt_position_trapezoid_area(height, x_top, x + 1.0f,
                                                                     x_bottom, x + 1.0f);
                        scanline_fill[x] += height;  // everything right of this pixel is filled
                    }
                    else {
                        // covers 2+ pixels
                        if (x_top > x_bottom) {
                            // flip scanline vertically; signed area is the same
                            f32 t;
                            sy0 = y_bottom - (sy0 - y_top);
                            sy1 = y_bottom - (sy1 - y_top);
                            t = sy0, sy0 = sy1, sy1 = t;
                            t = x_bottom, x_bottom = x_top, x_top = t;
                            dx = -dx;
                            dy = -dy;
                            t = x0, x0 = xb, xb = t;
                        }
                        debug_assert(dy >= 0);
                        debug_assert(dx >= 0);

                        i32 x1 = static_cast<i32>(x_top);
                        i32 x2 = static_cast<i32>(x_bottom);
                        // compute intersection with y axis at x1+1
                        f32 y_crossing = y_top + dy * (x1 + 1 - x0);

                        // compute intersection with y axis at x2
                        f32 y_final = y_top + dy * (x2 - x0);

                        //           x1    x_top                            x2    x_bottom
                        //     y_top
                        //     +------|-----+------------+------------+--------|---+------------+
                        //            |            |            |            |            | | | | |
                        //            |            |            |
                        //       sy0  | Txxxxx|............|............|............|............|
                        // y_crossing | *xxxxx.......|............|............|............|
                        //            |            | xxxxx..|............|............|............|
                        //            |            |     /-
                        //            xx*xxxx........|............|............| |            | dy <
                        //            |    xxxxxx..|............|............|
                        //   y_final  |            |     \-     | xx*xxx.........|............|
                        //       sy1  |            |            |            |
                        //       xxxxxB...|............|
                        //            |            |            |            |            | | | | |
                        //            |            |            |
                        //  y_bottom
                        //  +------------+------------+------------+------------+------------+
                        //
                        // goal is to measure the area covered by '.' in each pixel

                        // if x2 is right at the right edge of x1, y_crossing can blow up, github
                        // #1057
                        // @TODO: maybe test against sy1 rather than y_bottom?
                        if (y_crossing > y_bottom)
                            y_crossing = y_bottom;

                        f32 sign = e->direction;

                        // area of the rectangle covered from sy0..y_crossing
                        f32 area = sign * (y_crossing - sy0);

                        // area of the triangle (x_top,sy0), (x1+1,sy0), (x1+1,y_crossing)
                        scanline[x1] += stbtt_sized_triangle_area(area, x1 + 1 - x_top);

                        // check if final y_crossing is blown up; no test case for this
                        if (y_final > y_bottom) {
                            y_final = y_bottom;
                            dy = (y_final - y_crossing) / (x2 - (x1 + 1));  // if denom=0, y_final =
                                                                            // y_crossing, so
                                                                            // y_final <= y_bottom
                        }

                        // in second pixel, area covered by line segment found in first pixel
                        // is always a rectangle 1 wide * the height of that line segment; this
                        // is exactly what the variable 'area' stores. it also gets a contribution
                        // from the line segment within it. the THIRD pixel will get the first
                        // pixel's rectangle contribution, the second pixel's rectangle
                        // contribution, and its own contribution. the 'own contribution' is the
                        // same in every pixel except the leftmost and rightmost, a trapezoid that
                        // slides down in each pixel. the second pixel's contribution to the third
                        // pixel will be the rectangle 1 wide times the height change in the second
                        // pixel, which is dy.

                        f32 step = sign * dy * 1;  // dy is dy/dx, change in y for every 1 change
                                                   // in x,
                        // which multiplied by 1-pixel-width is how much pixel area changes for each
                        // step in x so the area advances by 'step' every time

                        for (i32 x = x1 + 1; x < x2; ++x) {
                            scanline[x] += area + step / 2;  // area of trapezoid is 1*step/2
                            area += step;
                        }
                        debug_assert(std::fabs(area) <= 1.01f);  // accumulated error from area +=
                                                                 // step unless we round step down
                        debug_assert(sy1 > y_final - 0.01f);

                        // area covered in the last pixel is the rectangle from all the pixels to
                        // the left, plus the trapezoid filled by the line segment in this pixel all
                        // the way to the right edge
                        scanline[x2] += area + sign * stbtt_position_trapezoid_area(
                                                          sy1 - y_final, static_cast<f32>(x2),
                                                          x2 + 1.0f, x_bottom, x2 + 1.0f);

                        // the rest of the line is filled based on the total height of the line
                        // segment in this pixel
                        scanline_fill[x2] += sign * (sy1 - sy0);
                    }
                }
                else {
                    // if edge goes outside of box we're drawing, we require
                    // clipping logic. since this does not match the intended use
                    // of this library, we use a different, very slow brute
                    // force implementation
                    // note though that this does happen some of the time because
                    // x_top and x_bottom can be extrapolated at the top & bottom of
                    // the shape and actually lie outside the bounding box
                    for (i32 x = 0; x < len; ++x) {
                        // cases:
                        //
                        // there can be up to two intersections with the pixel. any intersection
                        // with left or right edges can be handled by splitting into two (or three)
                        // regions. intersections with top & bottom do not necessitate case-wise
                        // logic.
                        //
                        // the old way of doing this found the intersections with the left & right
                        // edges, then used some simple logic to produce up to three segments in
                        // sorted order from top-to-bottom. however, this had a problem: if an x
                        // edge was epsilon across the x border, then the corresponding y position
                        // might not be distinct from the other y segment, and it might ignored as
                        // an empty segment. to avoid that, we need to explicitly produce segments
                        // based on x positions.

                        // rename variables to clearly-defined pairs
                        f32 y0 = y_top;
                        f32 x1 = static_cast<f32>(x);
                        f32 x2 = static_cast<f32>(x + 1);
                        f32 x3 = xb;
                        f32 y3 = y_bottom;

                        // x = e->x + e->dx * (y-y_top)
                        // (y-y_top) = (x - e->x) / e->dx
                        // y = (x - e->x) / e->dx + y_top
                        f32 y1 = (x - x0) / dx + y_top;
                        f32 y2 = (x + 1 - x0) / dx + y_top;

                        if (x0 < x1 && x3 > x2) {  // three segments descending down-right
                            stbtt_handle_clipped_edge(scanline, x, e, x0, y0, x1, y1);
                            stbtt_handle_clipped_edge(scanline, x, e, x1, y1, x2, y2);
                            stbtt_handle_clipped_edge(scanline, x, e, x2, y2, x3, y3);
                        }
                        else if (x3 < x1 && x0 > x2) {  // three segments descending down-left
                            stbtt_handle_clipped_edge(scanline, x, e, x0, y0, x2, y2);
                            stbtt_handle_clipped_edge(scanline, x, e, x2, y2, x1, y1);
                            stbtt_handle_clipped_edge(scanline, x, e, x1, y1, x3, y3);
                        }
                        else if (x0 < x1 && x3 > x1) {  // two segments across x, down-right
                            stbtt_handle_clipped_edge(scanline, x, e, x0, y0, x1, y1);
                            stbtt_handle_clipped_edge(scanline, x, e, x1, y1, x3, y3);
                        }
                        else if (x3 < x1 && x0 > x1) {  // two segments across x, down-left
                            stbtt_handle_clipped_edge(scanline, x, e, x0, y0, x1, y1);
                            stbtt_handle_clipped_edge(scanline, x, e, x1, y1, x3, y3);
                        }
                        else if (x0 < x2 && x3 > x2) {  // two segments across x+1, down-right
                            stbtt_handle_clipped_edge(scanline, x, e, x0, y0, x2, y2);
                            stbtt_handle_clipped_edge(scanline, x, e, x2, y2, x3, y3);
                        }
                        else if (x3 < x2 && x0 > x2) {  // two segments across x+1, down-left
                            stbtt_handle_clipped_edge(scanline, x, e, x0, y0, x2, y2);
                            stbtt_handle_clipped_edge(scanline, x, e, x2, y2, x3, y3);
                        }
                        else {  // one segment
                            stbtt_handle_clipped_edge(scanline, x, e, x0, y0, x3, y3);
                        }
                    }
                }
            }
            e = e->next;
        }
    }

    // directly AA rasterize edges w/o supersampling
    static void stbtt_rasterize_sorted_edges(const stbtt_bitmap* result, stbtt_edge* e, const i32 n,
                                             const i32 vsubsample, const i32 off_x, const i32 off_y,
                                             void* userdata)
    {
        stbtt_hheap hh = { nullptr, nullptr, 0 };
        stbtt_active_edge* active = nullptr;
        i32 j = 0;
        f32 scanline_data[129], *scanline;

        STBTT_NOT_USED(vsubsample);

        if (result->w > 64)
            scanline = static_cast<f32*>(std::malloc((result->w * 2 + 1) * sizeof(f32)));
        else
            scanline = scanline_data;

        f32* scanline2 = scanline + result->w;

        i32 y = off_y;
        e[n].y0 = static_cast<f32>(off_y + result->h) + 1;

        while (j < result->h) {
            // find center of pixel for this scanline
            f32 scan_y_top = y + 0.0f;
            f32 scan_y_bottom = y + 1.0f;
            stbtt_active_edge** step = &active;

            std::memset(scanline, 0, result->w * sizeof(scanline[0]));
            std::memset(scanline2, 0, (result->w + 1) * sizeof(scanline[0]));

            // update all active edges;
            // remove all active edges that terminate before the top of this scanline
            while (*step) {
                stbtt_active_edge* z = *step;
                if (z->ey <= scan_y_top) {
                    *step = z->next;  // delete from list
                    debug_assert(z->direction);
                    z->direction = 0;
                    stbtt_hheap_free(&hh, z);
                }
                else {
                    step = &((*step)->next);  // advance through list
                }
            }

            // insert all edges that start before the bottom of this scanline
            while (e->y0 <= scan_y_bottom) {
                if (e->y0 != e->y1) {
                    stbtt_active_edge* z = stbtt_new_active(&hh, e, off_x, scan_y_top, userdata);
                    if (z != nullptr) {
                        if (j == 0 && off_y != 0) {
                            if (z->ey < scan_y_top) {
                                // this can happen due to subpixel positioning and some kind of fp
                                // rounding error i think
                                z->ey = scan_y_top;
                            }
                        }
                        debug_assert(z->ey >= scan_y_top);  // if we get really unlucky a tiny bit
                                                            // of an edge can be out of bounds
                        // insert at front
                        z->next = active;
                        active = z;
                    }
                }
                ++e;
            }

            // now process all active edges
            if (active)
                stbtt_fill_active_edges_new(scanline, scanline2 + 1, result->w, active, scan_y_top);

            {
                f32 sum = 0;
                for (i32 i = 0; i < result->w; ++i) {
                    sum += scanline2[i];
                    f32 k = scanline[i] + sum;
                    k = std::fabs(k) * 255 + 0.5f;
                    i32 m = static_cast<i32>(k);
                    if (m > 255)
                        m = 255;
                    result->pixels[j * result->stride + i] = static_cast<u8>(m);
                }
            }
            // advance all the edges
            step = &active;
            while (*step) {
                stbtt_active_edge* z = *step;
                z->fx += z->fdx;          // advance to position for current scanline
                step = &((*step)->next);  // advance through list
            }

            ++y;
            ++j;
        }

        stbtt_hheap_cleanup(&hh, userdata);

        if (scanline != scanline_data)
            std::free(scanline);
    }
#else
  #error "Unrecognized value of STBTT_RASTERIZER_VERSION"
#endif

    constexpr static i32 stbtt_compare(auto a, auto b)
    {
        return a->y0 < b->y0;
    }

    static void stbtt_sort_edges_ins_sort(stbtt_edge* p, const i32 n)
    {
        for (i32 i = 1; i < n; ++i) {
            stbtt_edge t = p[i], *a = &t;
            i32 j = i;
            while (j > 0) {
                stbtt_edge* b = &p[j - 1];
                i32 c = stbtt_compare(a, b);
                if (!c)
                    break;
                p[j] = p[j - 1];
                --j;
            }
            if (i != j)
                p[j] = t;
        }
    }

    static void stbtt_sort_edges_quicksort(stbtt_edge* p, i32 n)
    {
        /* threshold for transitioning to insertion sort */
        while (n > 12) {
            stbtt_edge t;

            /* compute median of three */
            i32 m = n >> 1;
            i32 c01 = stbtt_compare(&p[0], &p[m]);
            i32 c12 = stbtt_compare(&p[m], &p[n - 1]);
            /* if 0 >= mid >= end, or 0 < mid < end, then use mid */
            if (c01 != c12) {
                /* otherwise, we'll need to swap something else to middle */
                i32 c = stbtt_compare(&p[0], &p[n - 1]);
                /* 0>mid && mid<n:  0>n => n; 0<n => 0 */
                /* 0<mid && mid>n:  0>n => 0; 0<n => n */
                i32 z = (c == c12) ? 0 : n - 1;
                t = p[z];
                p[z] = p[m];
                p[m] = t;
            }
            /* now p[m] is the median-of-three */
            /* swap it to the beginning so it won't move around */
            t = p[0];
            p[0] = p[m];
            p[m] = t;

            /* partition loop */
            i32 i = 1;
            i32 j = n - 1;
            for (;;) {
                /* handling of equality is crucial here */
                /* for sentinels & efficiency with duplicates */
                for (;; ++i)
                    if (!stbtt_compare(&p[i], &p[0]))
                        break;
                for (;; --j)
                    if (!stbtt_compare(&p[0], &p[j]))
                        break;
                /* make sure we haven't crossed */
                if (i >= j)
                    break;
                t = p[i];
                p[i] = p[j];
                p[j] = t;

                ++i;
                --j;
            }
            /* recurse on smaller side, iterate on larger */
            if (j < (n - i)) {
                stbtt_sort_edges_quicksort(p, j);
                p = p + i;
                n = n - i;
            }
            else {
                stbtt_sort_edges_quicksort(p + i, n - i);
                n = j;
            }
        }
    }

    static void stbtt_sort_edges(stbtt_edge* p, const i32 n)
    {
        stbtt_sort_edges_quicksort(p, n);
        stbtt_sort_edges_ins_sort(p, n);
    }

    typedef struct
    {
        f32 x, y;
    } stbtt_point;

    static void stbtt_rasterize(stbtt_bitmap* result, stbtt_point* pts, const i32* wcount,
                                const i32 windings, const f32 scale_x, const f32 scale_y,
                                const f32 shift_x, const f32 shift_y, const i32 off_x,
                                const i32 off_y, const i32 invert, void* userdata)
    {
        f32 y_scale_inv = invert ? -scale_y : scale_y;
        stbtt_edge* e;
        i32 n, i, j, m;
#if STBTT_RASTERIZER_VERSION == 1
        i32 vsubsample = result->h < 8 ? 15 : 5;
#elif STBTT_RASTERIZER_VERSION == 2
        i32 vsubsample = 1;
#else
  #error "Unrecognized value of STBTT_RASTERIZER_VERSION"
#endif
        // vsubsample should divide 255 evenly; otherwise we won't reach full opacity

        // now we have to blow out the windings into explicit edge lists
        n = 0;
        for (i = 0; i < windings; ++i)
            n += wcount[i];

        e = static_cast<stbtt_edge*>(std::malloc(sizeof(*e) * (n + 1)));  // add an extra
                                                                          // one as a
                                                                          // sentinel
        if (e == nullptr)
            return;
        n = 0;

        m = 0;
        for (i = 0; i < windings; ++i) {
            stbtt_point* p = pts + m;
            m += wcount[i];
            j = wcount[i] - 1;
            for (i32 k = 0; k < wcount[i]; j = k++) {
                i32 a = k, b = j;
                // skip the edge if horizontal
                if (p[j].y == p[k].y)
                    continue;
                // add edge from j to k to the list
                e[n].invert = 0;
                if (invert ? p[j].y > p[k].y : p[j].y < p[k].y) {
                    e[n].invert = 1;
                    a = j, b = k;
                }
                e[n].x0 = p[a].x * scale_x + shift_x;
                e[n].y0 = (p[a].y * y_scale_inv + shift_y) * vsubsample;
                e[n].x1 = p[b].x * scale_x + shift_x;
                e[n].y1 = (p[b].y * y_scale_inv + shift_y) * vsubsample;
                ++n;
            }
        }

        // now sort the edges by their highest point (should snap to integer, and then by x)
        // STBTT_sort(e, n, sizeof(e[0]), stbtt_edge_compare);
        stbtt_sort_edges(e, n);

        // now, traverse the scanlines and find the intersections on each scanline, use xor winding
        // rule
        stbtt_rasterize_sorted_edges(result, e, n, vsubsample, off_x, off_y, userdata);

        std::free(e);
    }

    static void stbtt_add_point(stbtt_point* points, const i32 n, const f32 x, const f32 y)
    {
        if (!points)
            return;  // during first pass, it's unallocated
        points[n].x = x;
        points[n].y = y;
    }

    // tessellate until threshold p is happy... @TODO warped to compensate for non-linear stretching
    static i32 stbtt_tesselate_curve(stbtt_point* points, i32* num_points, const f32 x0,
                                     const f32 y0, const f32 x1, const f32 y1, const f32 x2,
                                     const f32 y2, const f32 objspace_flatness_squared, const i32 n)
    {
        // midpoint
        f32 mx = (x0 + 2 * x1 + x2) / 4;
        f32 my = (y0 + 2 * y1 + y2) / 4;
        // versus directly drawn line
        f32 dx = (x0 + x2) / 2 - mx;
        f32 dy = (y0 + y2) / 2 - my;
        if (n > 16)  // 65536 segments on one curve better be enough!
            return 1;
        if (dx * dx + dy * dy > objspace_flatness_squared) {  // half-pixel error allowed... need to
                                                              // be smaller if AA
            stbtt_tesselate_curve(points, num_points, x0, y0, (x0 + x1) / 2.0f, (y0 + y1) / 2.0f,
                                  mx, my, objspace_flatness_squared, n + 1);
            stbtt_tesselate_curve(points, num_points, mx, my, (x1 + x2) / 2.0f, (y1 + y2) / 2.0f,
                                  x2, y2, objspace_flatness_squared, n + 1);
        }
        else {
            stbtt_add_point(points, *num_points, x2, y2);
            *num_points = *num_points + 1;
        }
        return 1;
    }

    static void stbtt_tesselate_cubic(stbtt_point* points, i32* num_points, const f32 x0,
                                      const f32 y0, const f32 x1, const f32 y1, const f32 x2,
                                      const f32 y2, const f32 x3, const f32 y3,
                                      const f32 objspace_flatness_squared, const i32 n)
    {
        // @TODO this "flatness" calculation is just made-up nonsense that seems to work well enough
        f32 dx0 = x1 - x0;
        f32 dy0 = y1 - y0;
        f32 dx1 = x2 - x1;
        f32 dy1 = y2 - y1;
        f32 dx2 = x3 - x2;
        f32 dy2 = y3 - y2;
        f32 dx = x3 - x0;
        f32 dy = y3 - y0;
        f32 longlen = std::sqrt(dx0 * dx0 + dy0 * dy0) + std::sqrt(dx1 * dx1 + dy1 * dy1) +
                      std::sqrt(dx2 * dx2 + dy2 * dy2);
        f32 shortlen = std::sqrt(dx * dx + dy * dy);
        f32 flatness_squared = longlen * longlen - shortlen * shortlen;

        if (n > 16)  // 65536 segments on one curve better be enough!
            return;

        if (flatness_squared > objspace_flatness_squared) {
            f32 x01 = (x0 + x1) / 2;
            f32 y01 = (y0 + y1) / 2;
            f32 x12 = (x1 + x2) / 2;
            f32 y12 = (y1 + y2) / 2;
            f32 x23 = (x2 + x3) / 2;
            f32 y23 = (y2 + y3) / 2;

            f32 xa = (x01 + x12) / 2;
            f32 ya = (y01 + y12) / 2;
            f32 xb = (x12 + x23) / 2;
            f32 yb = (y12 + y23) / 2;

            f32 mx = (xa + xb) / 2;
            f32 my = (ya + yb) / 2;

            stbtt_tesselate_cubic(points, num_points, x0, y0, x01, y01, xa, ya, mx, my,
                                  objspace_flatness_squared, n + 1);
            stbtt_tesselate_cubic(points, num_points, mx, my, xb, yb, x23, y23, x3, y3,
                                  objspace_flatness_squared, n + 1);
        }
        else {
            stbtt_add_point(points, *num_points, x3, y3);
            *num_points = *num_points + 1;
        }
    }

    // returns number of contours
    static stbtt_point* stbtt_flatten_curves(const stbtt_vertex* vertices, const i32 num_verts,
                                             const f32 objspace_flatness, i32** contour_lengths,
                                             i32* num_contours, const void*)
    {
        stbtt_point* points = nullptr;
        i32 num_points = 0;

        f32 objspace_flatness_squared = objspace_flatness * objspace_flatness;
        i32 i, n = 0, start = 0;

        // count how many "moves" there are to get the contour count
        for (i = 0; i < num_verts; ++i)
            if (vertices[i].type == STBTT_vmove)
                ++n;

        *num_contours = n;
        if (n == 0)
            return nullptr;

        *contour_lengths = static_cast<i32*>(std::malloc(sizeof(**contour_lengths) * n));

        if (*contour_lengths == nullptr) {
            *num_contours = 0;
            return nullptr;
        }

        // make two passes through the points so we don't need to realloc
        for (i32 pass = 0; pass < 2; ++pass) {
            f32 x = 0, y = 0;
            if (pass == 1) {
                points = static_cast<stbtt_point*>(std::malloc(num_points * sizeof(points[0])));
                if (points == nullptr)
                    goto error;
            }
            num_points = 0;
            n = -1;
            for (i = 0; i < num_verts; ++i) {
                switch (vertices[i].type) {
                    case STBTT_vmove:
                        // start the next contour
                        if (n >= 0)
                            (*contour_lengths)[n] = num_points - start;
                        ++n;
                        start = num_points;

                        x = vertices[i].x;
                        y = vertices[i].y;
                        stbtt_add_point(points, num_points++, x, y);
                        break;
                    case STBTT_vline:
                        x = vertices[i].x;
                        y = vertices[i].y;
                        stbtt_add_point(points, num_points++, x, y);
                        break;
                    case STBTT_vcurve:
                        stbtt_tesselate_curve(points, &num_points, x, y, vertices[i].cx,
                                              vertices[i].cy, vertices[i].x, vertices[i].y,
                                              objspace_flatness_squared, 0);
                        x = vertices[i].x, y = vertices[i].y;
                        break;
                    case STBTT_vcubic:
                        stbtt_tesselate_cubic(points, &num_points, x, y, vertices[i].cx,
                                              vertices[i].cy, vertices[i].cx1, vertices[i].cy1,
                                              vertices[i].x, vertices[i].y,
                                              objspace_flatness_squared, 0);
                        x = vertices[i].x, y = vertices[i].y;
                        break;
                }
            }
            (*contour_lengths)[n] = num_points - start;
        }

        return points;
    error:
        std::free(points);
        std::free(*contour_lengths);
        *contour_lengths = nullptr;
        *num_contours = 0;
        return nullptr;
    }

    void stbtt_rasterize(stbtt_bitmap* result, const f32 flatness_in_pixels, stbtt_vertex* vertices,
                         const i32 num_verts, const f32 scale_x, const f32 scale_y,
                         const f32 shift_x, const f32 shift_y, const i32 x_off, const i32 y_off,
                         const i32 invert, void* userdata)
    {
        f32 scale = scale_x > scale_y ? scale_y : scale_x;
        i32 winding_count = 0;
        i32* winding_lengths = nullptr;
        stbtt_point* windings = stbtt_flatten_curves(vertices, num_verts,
                                                     flatness_in_pixels / scale, &winding_lengths,
                                                     &winding_count, userdata);
        if (windings) {
            stbtt_rasterize(result, windings, winding_lengths, winding_count, scale_x, scale_y,
                            shift_x, shift_y, x_off, y_off, invert, userdata);
            std::free(winding_lengths);
            std::free(windings);
        }
    }

    void stbtt_free_bitmap(u8* bitmap, const void*)
    {
        std::free(bitmap);
    }

    u8* stbtt_get_glyph_bitmap_subpixel(const stbtt_fontinfo* info, f32 scale_x, f32 scale_y,
                                        const f32 shift_x, const f32 shift_y, const i32 glyph,
                                        i32* width, i32* height, i32* xoff, i32* yoff)
    {
        i32 ix0, iy0, ix1, iy1;
        stbtt_bitmap gbm;
        stbtt_vertex* vertices;
        i32 num_verts = stbtt_get_glyph_shape(info, glyph, &vertices);

        if (scale_x == 0)
            scale_x = scale_y;
        if (scale_y == 0) {
            if (scale_x == 0) {
                std::free(vertices);
                return nullptr;
            }
            scale_y = scale_x;
        }

        stbtt_get_glyph_bitmap_box_subpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0,
                                            &iy0, &ix1, &iy1);

        // now we get the size
        gbm.w = (ix1 - ix0);
        gbm.h = (iy1 - iy0);
        gbm.pixels = nullptr;  // in case we error

        if (width)
            *width = gbm.w;
        if (height)
            *height = gbm.h;
        if (xoff)
            *xoff = ix0;
        if (yoff)
            *yoff = iy0;

        if (gbm.w && gbm.h) {
            gbm.pixels = static_cast<u8*>(std::malloc(gbm.w * gbm.h));
            if (gbm.pixels) {
                gbm.stride = gbm.w;

                stbtt_rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x,
                                shift_y, ix0, iy0, 1, info->userdata);
            }
        }
        std::free(vertices);
        return gbm.pixels;
    }

    u8* stbtt_get_glyph_bitmap(const stbtt_fontinfo* info, const f32 scale_x, const f32 scale_y,
                               const i32 glyph, i32* width, i32* height, i32* xoff, i32* yoff)
    {
        return stbtt_get_glyph_bitmap_subpixel(info, scale_x, scale_y, 0.0f, 0.0f, glyph, width,
                                               height, xoff, yoff);
    }

    void stbtt_make_glyph_bitmap_subpixel(const stbtt_fontinfo* info, u8* output, const i32 out_w,
                                          const i32 out_h, const i32 out_stride, const f32 scale_x,
                                          const f32 scale_y, const f32 shift_x, const f32 shift_y,
                                          const i32 glyph)
    {
        i32 ix0, iy0;
        stbtt_vertex* vertices;
        i32 num_verts = stbtt_get_glyph_shape(info, glyph, &vertices);
        stbtt_bitmap gbm;

        stbtt_get_glyph_bitmap_box_subpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0,
                                            &iy0, nullptr, nullptr);
        gbm.pixels = output;
        gbm.w = out_w;
        gbm.h = out_h;
        gbm.stride = out_stride;

        if (gbm.w && gbm.h)
            stbtt_rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y,
                            ix0, iy0, 1, info->userdata);

        std::free(vertices);
    }

    void stbtt_make_glyph_bitmap(const stbtt_fontinfo* info, u8* output, const i32 out_w,
                                 const i32 out_h, const i32 out_stride, const f32 scale_x,
                                 const f32 scale_y, const i32 glyph)
    {
        stbtt_make_glyph_bitmap_subpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y,
                                         0.0f, 0.0f, glyph);
    }

    u8* stbtt_get_codepoint_bitmap_subpixel(
        const stbtt_fontinfo* info, const f32 scale_x, const f32 scale_y, const f32 shift_x,
        const f32 shift_y, const i32 codepoint, i32* width, i32* height, i32* xoff, i32* yoff)
    {
        return stbtt_get_glyph_bitmap_subpixel(info, scale_x, scale_y, shift_x, shift_y,
                                               stbtt_find_glyph_index(info, codepoint), width,
                                               height, xoff, yoff);
    }

    void stbtt_make_codepoint_bitmap_subpixel_prefilter(
        const stbtt_fontinfo* info, u8* output, const i32 out_w, const i32 out_h,
        const i32 out_stride, const f32 scale_x, const f32 scale_y, const f32 shift_x,
        const f32 shift_y, const i32 oversample_x, const i32 oversample_y, f32* sub_x, f32* sub_y,
        const i32 codepoint)
    {
        stbtt_MakeGlyphBitmapSubpixelPrefilter(
            info, output, out_w, out_h, out_stride, scale_x, scale_y, shift_x, shift_y,
            oversample_x, oversample_y, sub_x, sub_y, stbtt_find_glyph_index(info, codepoint));
    }

    void stbtt_MakeCodepointBitmapSubpixel(const stbtt_fontinfo* info, u8* output, const i32 out_w,
                                           const i32 out_h, const i32 out_stride, const f32 scale_x,
                                           const f32 scale_y, const f32 shift_x, const f32 shift_y,
                                           const i32 codepoint)
    {
        stbtt_make_glyph_bitmap_subpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y,
                                         shift_x, shift_y, stbtt_find_glyph_index(info, codepoint));
    }

    u8* stbtt_get_codepoint_bitmap(const stbtt_fontinfo* info, const f32 scale_x, const f32 scale_y,
                                   const i32 codepoint, i32* width, i32* height, i32* xoff,
                                   i32* yoff)
    {
        return stbtt_get_codepoint_bitmap_subpixel(info, scale_x, scale_y, 0.0f, 0.0f, codepoint,
                                                   width, height, xoff, yoff);
    }

    void stbtt_make_codepoint_bitmap(const stbtt_fontinfo* info, u8* output, const i32 out_w,
                                     const i32 out_h, const i32 out_stride, const f32 scale_x,
                                     const f32 scale_y, const i32 codepoint)
    {
        stbtt_MakeCodepointBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y,
                                          0.0f, 0.0f, codepoint);
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // bitmap baking
    //
    // This is SUPER-CRAPPY packing to keep source code small

    static i32 stbtt_bake_font_bitmap_internal(const u8* data,
                                               const i32 offset,        // font location (use offset=0 for
                                                                        // plain .ttf)
                                               const f32 pixel_height,  // height of font in  pixels
                                               u8* pixels, const i32 pw,
                                               const i32 ph,  // bitmap to be filled in
                                               const i32 first_char,
                                               const i32 num_chars,  // characters to bake
                                               stbtt_bakedchar* chardata)
    {
        i32 y;
        stbtt_fontinfo f;
        f.userdata = nullptr;
        if (!stbtt_init_font(&f, data, offset))
            return -1;
        std::memset(pixels, 0, pw * ph);  // background of 0 around pixels
        i32 x = y = 1;
        i32 bottom_y = 1;

        f32 scale = stbtt_scale_for_pixel_height(&f, pixel_height);

        for (i32 i = 0; i < num_chars; ++i) {
            i32 advance, lsb, x0, y0, x1, y1;
            i32 g = stbtt_find_glyph_index(&f, first_char + i);
            stbtt_get_glyph_h_metrics(&f, g, &advance, &lsb);
            stbtt_get_glyph_bitmap_box(&f, g, scale, scale, &x0, &y0, &x1, &y1);
            i32 gw = x1 - x0;
            i32 gh = y1 - y0;
            if (x + gw + 1 >= pw)
                y = bottom_y, x = 1;  // advance to next row
            if (y + gh + 1 >= ph)     // check if it fits vertically AFTER potentially moving to next
                                      // row
                return -i;
            debug_assert(x + gw < pw);
            debug_assert(y + gh < ph);
            stbtt_make_glyph_bitmap(&f, pixels + x + y * pw, gw, gh, pw, scale, scale, g);
            chardata[i].x0 = static_cast<i16>(x);
            chardata[i].y0 = static_cast<i16>(y);
            chardata[i].x1 = static_cast<i16>(x + gw);
            chardata[i].y1 = static_cast<i16>(y + gh);
            chardata[i].xadvance = scale * advance;
            chardata[i].xoff = static_cast<f32>(x0);
            chardata[i].yoff = static_cast<f32>(y0);
            x = x + gw + 1;
            if (y + gh + 1 > bottom_y)
                bottom_y = y + gh + 1;
        }
        return bottom_y;
    }

    void stbtt_get_baked_quad(const stbtt_bakedchar* chardata, const i32 pw, const i32 ph,
                              const i32 char_index, f32* xpos, const f32* ypos,
                              stbtt_aligned_quad* q, const i32 opengl_fillrule)
    {
        f32 d3d_bias = opengl_fillrule ? 0 : -0.5f;
        f32 ipw = 1.0f / pw, iph = 1.0f / ph;
        const stbtt_bakedchar* b = chardata + char_index;
        i32 round_x = std::lrintf(std::floorf((*xpos + b->xoff) + 0.5f));
        i32 round_y = std::lrintf(std::floorf((*ypos + b->yoff) + 0.5f));

        q->x0 = round_x + d3d_bias;
        q->y0 = round_y + d3d_bias;
        q->x1 = round_x + b->x1 - b->x0 + d3d_bias;
        q->y1 = round_y + b->y1 - b->y0 + d3d_bias;

        q->s0 = b->x0 * ipw;
        q->t0 = b->y0 * iph;
        q->s1 = b->x1 * ipw;
        q->t1 = b->y1 * iph;

        *xpos += b->xadvance;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // rectangle packing replacement routines if you don't have stb_rect_pack.h
    //

#ifndef STB_RECT_PACK_VERSION

    typedef i32 stbrp_coord;

    ////////////////////////////////////////////////////////////////////////////////////
    //                                                                                //
    //                                                                                //
    // COMPILER WARNING ?!?!?                                                         //
    //                                                                                //
    //                                                                                //
    // if you get a compile warning due to these symbols being defined more than      //
    // once, move #include "stb_rect_pack.h" before #include "stb_truetype.h"         //
    //                                                                                //
    ////////////////////////////////////////////////////////////////////////////////////

    typedef struct
    {
        i32 width, height;
        i32 x, y, bottom_y;
    } stbrp_context;

    typedef struct
    {
        u8 x;
    } stbrp_node;

    struct stbrp_rect
    {
        stbrp_coord x, y;
        i32 id, w, h, was_packed;
    };

    static void stbrp_init_target(stbrp_context* con, const i32 pw, const i32 ph,
                                  const stbrp_node* nodes, const i32 num_nodes)
    {
        con->width = pw;
        con->height = ph;
        con->x = 0;
        con->y = 0;
        con->bottom_y = 0;
        STBTT_NOT_USED(nodes);
        STBTT_NOT_USED(num_nodes);
    }

    static void stbrp_pack_rects(stbrp_context* con, stbrp_rect* rects, const i32 num_rects)
    {
        i32 i;
        for (i = 0; i < num_rects; ++i) {
            if (con->x + rects[i].w > con->width) {
                con->x = 0;
                con->y = con->bottom_y;
            }
            if (con->y + rects[i].h > con->height)
                break;
            rects[i].x = con->x;
            rects[i].y = con->y;
            rects[i].was_packed = 1;
            con->x += rects[i].w;
            if (con->y + rects[i].h > con->bottom_y)
                con->bottom_y = con->y + rects[i].h;
        }
        for (; i < num_rects; ++i)
            rects[i].was_packed = 0;
    }
#endif

    //////////////////////////////////////////////////////////////////////////////
    //
    // bitmap baking
    //
    // This is SUPER-AWESOME (tm Ryan Gordon) packing using stb_rect_pack.h. If
    // stb_rect_pack.h isn't available, it uses the BakeFontBitmap strategy.

    i32 stbtt_pack_begin(stbtt_pack_context* spc, u8* pixels, const i32 width, const i32 height,
                         const i32 stride_in_bytes, const i32 padding, void* alloc_context)
    {
        stbrp_context* context = static_cast<stbrp_context*>(std::malloc(sizeof(*context)));
        i32 num_nodes = width - padding;
        stbrp_node* nodes = static_cast<stbrp_node*>(std::malloc(sizeof(*nodes) * num_nodes));

        if (context == nullptr || nodes == nullptr) {
            if (context != nullptr)
                std::free(context);
            if (nodes != nullptr)
                std::free(nodes);
            return 0;
        }

        spc->user_allocator_context = alloc_context;
        spc->width = width;
        spc->height = height;
        spc->pixels = pixels;
        spc->pack_info = context;
        spc->nodes = nodes;
        spc->padding = padding;
        spc->stride_in_bytes = stride_in_bytes != 0 ? stride_in_bytes : width;
        spc->h_oversample = 1;
        spc->v_oversample = 1;
        spc->skip_missing = 0;

        stbrp_init_target(context, width - padding, height - padding, nodes, num_nodes);

        if (pixels)
            std::memset(pixels, 0, width * height);  // background of 0 around pixels

        return 1;
    }

    void stbtt_pack_end(const stbtt_pack_context* spc)
    {
        std::free(spc->nodes);
        std::free(spc->pack_info);
    }

    void stbtt_pack_set_oversampling(stbtt_pack_context* spc, const u32 h_oversample,
                                     const u32 v_oversample)
    {
        debug_assert(h_oversample <= MaxOversample);
        debug_assert(v_oversample <= MaxOversample);
        if (h_oversample <= MaxOversample)
            spc->h_oversample = h_oversample;
        if (v_oversample <= MaxOversample)
            spc->v_oversample = v_oversample;
    }

    void stbtt_pack_set_skip_missing_codepoints(stbtt_pack_context* spc, const i32 skip)
    {
        spc->skip_missing = skip;
    }

    static void stbtt_h_prefilter(u8* pixels, const i32 w, const i32 h, const i32 stride_in_bytes,
                                  const u32 kernel_width)
    {
        u8 buffer[MaxOversample];
        i32 safe_w = w - kernel_width;
        std::memset(buffer, 0, MaxOversample);  // suppress bogus warning from VS2013
                                                // -analyze
        for (i32 j = 0; j < h; ++j) {
            i32 i;
            std::memset(buffer, 0, kernel_width);

            u32 total = 0;

            // make kernel_width a constant in common cases so compiler can optimize out the divide
            switch (kernel_width) {
                case 2:
                    for (i = 0; i <= safe_w; ++i) {
                        total += pixels[i] - buffer[i & OversampleMask];
                        buffer[(i + kernel_width) & OversampleMask] = pixels[i];
                        pixels[i] = static_cast<u8>(total / 2);
                    }
                    break;
                case 3:
                    for (i = 0; i <= safe_w; ++i) {
                        total += pixels[i] - buffer[i & OversampleMask];
                        buffer[(i + kernel_width) & OversampleMask] = pixels[i];
                        pixels[i] = static_cast<u8>(total / 3);
                    }
                    break;
                case 4:
                    for (i = 0; i <= safe_w; ++i) {
                        total += pixels[i] - buffer[i & OversampleMask];
                        buffer[(i + kernel_width) & OversampleMask] = pixels[i];
                        pixels[i] = static_cast<u8>(total / 4);
                    }
                    break;
                case 5:
                    for (i = 0; i <= safe_w; ++i) {
                        total += pixels[i] - buffer[i & OversampleMask];
                        buffer[(i + kernel_width) & OversampleMask] = pixels[i];
                        pixels[i] = static_cast<u8>(total / 5);
                    }
                    break;
                default:
                    for (i = 0; i <= safe_w; ++i) {
                        total += pixels[i] - buffer[i & OversampleMask];
                        buffer[(i + kernel_width) & OversampleMask] = pixels[i];
                        pixels[i] = static_cast<u8>(total / kernel_width);
                    }
                    break;
            }

            for (; i < w; ++i) {
                debug_assert(pixels[i] == 0);
                total -= buffer[i & OversampleMask];
                pixels[i] = static_cast<u8>(total / kernel_width);
            }

            pixels += stride_in_bytes;
        }
    }

    static void stbtt_v_prefilter(u8* pixels, const i32 w, const i32 h, const i32 stride_in_bytes,
                                  const u32 kernel_width)
    {
        u8 buffer[MaxOversample];
        i32 safe_h = h - kernel_width;
        std::memset(buffer, 0, MaxOversample);  // suppress bogus warning from VS2013
                                                // -analyze
        for (i32 j = 0; j < w; ++j) {
            i32 i;
            std::memset(buffer, 0, kernel_width);

            u32 total = 0;

            // make kernel_width a constant in common cases so compiler can optimize out the divide
            switch (kernel_width) {
                case 2:
                    for (i = 0; i <= safe_h; ++i) {
                        total += pixels[i * stride_in_bytes] - buffer[i & OversampleMask];
                        buffer[(i + kernel_width) & OversampleMask] = pixels[i * stride_in_bytes];
                        pixels[i * stride_in_bytes] = static_cast<u8>(total / 2);
                    }
                    break;
                case 3:
                    for (i = 0; i <= safe_h; ++i) {
                        total += pixels[i * stride_in_bytes] - buffer[i & OversampleMask];
                        buffer[(i + kernel_width) & OversampleMask] = pixels[i * stride_in_bytes];
                        pixels[i * stride_in_bytes] = static_cast<u8>(total / 3);
                    }
                    break;
                case 4:
                    for (i = 0; i <= safe_h; ++i) {
                        total += pixels[i * stride_in_bytes] - buffer[i & OversampleMask];
                        buffer[(i + kernel_width) & OversampleMask] = pixels[i * stride_in_bytes];
                        pixels[i * stride_in_bytes] = static_cast<u8>(total / 4);
                    }
                    break;
                case 5:
                    for (i = 0; i <= safe_h; ++i) {
                        total += pixels[i * stride_in_bytes] - buffer[i & OversampleMask];
                        buffer[(i + kernel_width) & OversampleMask] = pixels[i * stride_in_bytes];
                        pixels[i * stride_in_bytes] = static_cast<u8>(total / 5);
                    }
                    break;
                default:
                    for (i = 0; i <= safe_h; ++i) {
                        total += pixels[i * stride_in_bytes] - buffer[i & OversampleMask];
                        buffer[(i + kernel_width) & OversampleMask] = pixels[i * stride_in_bytes];
                        pixels[i * stride_in_bytes] = static_cast<u8>(total / kernel_width);
                    }
                    break;
            }

            for (; i < h; ++i) {
                debug_assert(pixels[i * stride_in_bytes] == 0);
                total -= buffer[i & OversampleMask];
                pixels[i * stride_in_bytes] = static_cast<u8>(total / kernel_width);
            }

            pixels += 1;
        }
    }

    static f32 stbtt_oversample_shift(const i32 oversample)
    {
        if (!oversample)
            return 0.0f;

        // The prefilter is a box filter of width "oversample",
        // which shifts phase by (oversample - 1)/2 pixels in
        // oversampled space. We want to shift in the opposite
        // direction to counter this.
        return static_cast<f32>(-(oversample - 1)) / (2.0f * static_cast<f32>(oversample));
    }

    // rects array must be big enough to accommodate all characters in the given ranges
    i32 stbtt_pack_font_ranges_gather_rects(const stbtt_pack_context* spc,
                                            const stbtt_fontinfo* info, stbtt_pack_range* ranges,
                                            const i32 num_ranges, stbrp_rect* rects)
    {
        i32 missing_glyph_added = 0;

        i32 k = 0;
        for (i32 i = 0; i < num_ranges; ++i) {
            f32 fh = ranges[i].font_size;
            f32 scale = fh > 0 ? stbtt_scale_for_pixel_height(info, fh)
                               : stbtt_scale_for_mapping_em_to_pixels(info, -fh);
            ranges[i].h_oversample = static_cast<u8>(spc->h_oversample);
            ranges[i].v_oversample = static_cast<u8>(spc->v_oversample);
            for (i32 j = 0; j < ranges[i].num_chars; ++j) {
                i32 x0, y0, x1, y1;
                i32 codepoint = ranges[i].array_of_unicode_codepoints == nullptr
                                  ? ranges[i].first_unicode_codepoint_in_range + j
                                  : ranges[i].array_of_unicode_codepoints[j];
                i32 glyph = stbtt_find_glyph_index(info, codepoint);
                if (glyph == 0 && (spc->skip_missing || missing_glyph_added)) {
                    rects[k].w = rects[k].h = 0;
                }
                else {
                    stbtt_get_glyph_bitmap_box_subpixel(info, glyph, scale * spc->h_oversample,
                                                        scale * spc->v_oversample, 0, 0, &x0, &y0,
                                                        &x1, &y1);
                    rects[k].w = static_cast<stbrp_coord>(
                        x1 - x0 + spc->padding + spc->h_oversample - 1);
                    rects[k].h = static_cast<stbrp_coord>(
                        y1 - y0 + spc->padding + spc->v_oversample - 1);
                    if (glyph == 0)
                        missing_glyph_added = 1;
                }
                ++k;
            }
        }

        return k;
    }

    void stbtt_MakeGlyphBitmapSubpixelPrefilter(
        const stbtt_fontinfo* info, u8* output, const i32 out_w, const i32 out_h,
        const i32 out_stride, const f32 scale_x, const f32 scale_y, const f32 shift_x,
        const f32 shift_y, const i32 prefilter_x, const i32 prefilter_y, f32* sub_x, f32* sub_y,
        const i32 glyph)
    {
        stbtt_make_glyph_bitmap_subpixel(info, output, out_w - (prefilter_x - 1),
                                         out_h - (prefilter_y - 1), out_stride, scale_x, scale_y,
                                         shift_x, shift_y, glyph);

        if (prefilter_x > 1)
            stbtt_h_prefilter(output, out_w, out_h, out_stride, prefilter_x);

        if (prefilter_y > 1)
            stbtt_v_prefilter(output, out_w, out_h, out_stride, prefilter_y);

        *sub_x = stbtt_oversample_shift(prefilter_x);
        *sub_y = stbtt_oversample_shift(prefilter_y);
    }

    // rects array must be big enough to accommodate all characters in the given ranges
    i32 stbtt_pack_font_ranges_render_into_rects(
        stbtt_pack_context* spc, const stbtt_fontinfo* info, const stbtt_pack_range* ranges,
        const i32 num_ranges, stbrp_rect* rects)
    {
        i32 missing_glyph = -1, return_value = 1;

        // save current values
        i32 old_h_over = spc->h_oversample;
        i32 old_v_over = spc->v_oversample;

        i32 k = 0;
        for (i32 i = 0; i < num_ranges; ++i) {
            f32 fh = ranges[i].font_size;
            f32 scale = fh > 0 ? stbtt_scale_for_pixel_height(info, fh)
                               : stbtt_scale_for_mapping_em_to_pixels(info, -fh);
            spc->h_oversample = ranges[i].h_oversample;
            spc->v_oversample = ranges[i].v_oversample;
            f32 recip_h = 1.0f / spc->h_oversample;
            f32 recip_v = 1.0f / spc->v_oversample;
            f32 sub_x = stbtt_oversample_shift(spc->h_oversample);
            f32 sub_y = stbtt_oversample_shift(spc->v_oversample);
            for (i32 j = 0; j < ranges[i].num_chars; ++j) {
                stbrp_rect* r = &rects[k];
                if (r->was_packed && r->w != 0 && r->h != 0) {
                    stbtt_packedchar* bc = &ranges[i].chardata_for_range[j];
                    i32 advance, lsb, x0, y0, x1, y1;
                    i32 codepoint = ranges[i].array_of_unicode_codepoints == nullptr
                                      ? ranges[i].first_unicode_codepoint_in_range + j
                                      : ranges[i].array_of_unicode_codepoints[j];
                    i32 glyph = stbtt_find_glyph_index(info, codepoint);
                    stbrp_coord pad = spc->padding;

                    // pad on left and top
                    r->x += pad;
                    r->y += pad;
                    r->w -= pad;
                    r->h -= pad;
                    stbtt_get_glyph_h_metrics(info, glyph, &advance, &lsb);
                    stbtt_get_glyph_bitmap_box(info, glyph, scale * spc->h_oversample,
                                               scale * spc->v_oversample, &x0, &y0, &x1, &y1);
                    stbtt_make_glyph_bitmap_subpixel(
                        info, spc->pixels + r->x + r->y * spc->stride_in_bytes,
                        r->w - spc->h_oversample + 1, r->h - spc->v_oversample + 1,
                        spc->stride_in_bytes, scale * spc->h_oversample, scale * spc->v_oversample,
                        0, 0, glyph);

                    if (spc->h_oversample > 1)
                        stbtt_h_prefilter(spc->pixels + r->x + r->y * spc->stride_in_bytes, r->w,
                                          r->h, spc->stride_in_bytes, spc->h_oversample);

                    if (spc->v_oversample > 1)
                        stbtt_v_prefilter(spc->pixels + r->x + r->y * spc->stride_in_bytes, r->w,
                                          r->h, spc->stride_in_bytes, spc->v_oversample);

                    bc->x0 = static_cast<i16>(r->x);
                    bc->y0 = static_cast<i16>(r->y);
                    bc->x1 = static_cast<i16>(r->x + r->w);
                    bc->y1 = static_cast<i16>(r->y + r->h);
                    bc->xadvance = scale * advance;
                    bc->xoff = static_cast<f32>(x0) * recip_h + sub_x;
                    bc->yoff = static_cast<f32>(y0) * recip_v + sub_y;
                    bc->xoff2 = (x0 + r->w) * recip_h + sub_x;
                    bc->yoff2 = (y0 + r->h) * recip_v + sub_y;

                    if (glyph == 0)
                        missing_glyph = j;
                }
                else if (spc->skip_missing) {
                    return_value = 0;
                }
                else if (r->was_packed && r->w == 0 && r->h == 0 && missing_glyph >= 0) {
                    ranges[i].chardata_for_range[j] = ranges[i].chardata_for_range[missing_glyph];
                }
                else {
                    return_value = 0;  // if any fail, report failure
                }

                ++k;
            }
        }

        // restore original values
        spc->h_oversample = old_h_over;
        spc->v_oversample = old_v_over;

        return return_value;
    }

    void stbtt_pack_font_ranges_pack_rects(const stbtt_pack_context* spc, stbrp_rect* rects,
                                           const i32 num_rects)
    {
        stbrp_pack_rects(static_cast<stbrp_context*>(spc->pack_info), rects, num_rects);
    }

    i32 stbtt_pack_font_ranges(stbtt_pack_context* spc, const u8* fontdata, const i32 font_index,
                               stbtt_pack_range* ranges, const i32 num_ranges)
    {
        stbtt_fontinfo info;
        i32 i, return_value = 1;
        // stbrp_context *context = (stbrp_context *) spc->pack_info;

        // flag all characters as NOT packed
        for (i = 0; i < num_ranges; ++i) {
            for (i32 j = 0; j < ranges[i].num_chars; ++j) {
                ranges[i].chardata_for_range[j].x0 = 0;
                ranges[i].chardata_for_range[j].y0 = 0;
                ranges[i].chardata_for_range[j].x1 = 0;
                ranges[i].chardata_for_range[j].y1 = 0;
            }
        }

        i32 n = 0;
        for (i = 0; i < num_ranges; ++i)
            n += ranges[i].num_chars;

        stbrp_rect* rects = static_cast<stbrp_rect*>(std::malloc(sizeof(*rects) * n));
        if (rects == nullptr)
            return 0;

        info.userdata = spc->user_allocator_context;
        stbtt_init_font(&info, fontdata, stbtt_get_font_offset_for_index(fontdata, font_index));

        n = stbtt_pack_font_ranges_gather_rects(spc, &info, ranges, num_ranges, rects);

        stbtt_pack_font_ranges_pack_rects(spc, rects, n);

        return_value = stbtt_pack_font_ranges_render_into_rects(spc, &info, ranges, num_ranges,
                                                                rects);

        std::free(rects);
        return return_value;
    }

    i32 stbtt_pack_font_range(stbtt_pack_context* spc, const u8* fontdata, const i32 font_index,
                              const f32 font_size, const i32 first_unicode_codepoint_in_range,
                              const i32 num_chars_in_range, stbtt_packedchar* chardata_for_range)
    {
        stbtt_pack_range range;
        range.first_unicode_codepoint_in_range = first_unicode_codepoint_in_range;
        range.array_of_unicode_codepoints = nullptr;
        range.num_chars = num_chars_in_range;
        range.chardata_for_range = chardata_for_range;
        range.font_size = font_size;
        return stbtt_pack_font_ranges(spc, fontdata, font_index, &range, 1);
    }

    void stbtt_get_scaled_font_v_metrics(const u8* fontdata, const i32 index, const f32 size,
                                         f32* ascent, f32* descent, f32* lineGap)
    {
        i32 i_ascent, i_descent, i_lineGap;
        stbtt_fontinfo info;
        stbtt_init_font(&info, fontdata, stbtt_get_font_offset_for_index(fontdata, index));
        f32 scale = size > 0 ? stbtt_scale_for_pixel_height(&info, size)
                             : stbtt_scale_for_mapping_em_to_pixels(&info, -size);
        stbtt_get_font_v_metrics(&info, &i_ascent, &i_descent, &i_lineGap);
        *ascent = static_cast<f32>(i_ascent) * scale;
        *descent = static_cast<f32>(i_descent) * scale;
        *lineGap = static_cast<f32>(i_lineGap) * scale;
    }

    void stbtt_get_packed_quad(const stbtt_packedchar* chardata, const i32 pw, const i32 ph,
                               const i32 char_index, f32* xpos, const f32* ypos,
                               stbtt_aligned_quad* q, const i32 align_to_integer)
    {
        f32 ipw = 1.0f / pw, iph = 1.0f / ph;
        const stbtt_packedchar* b = chardata + char_index;

        if (align_to_integer) {
            f32 x = std::floorf(std::round(*xpos + b->xoff));
            f32 y = std::floorf(std::round(*ypos + b->yoff));
            q->x0 = x;
            q->y0 = y;
            q->x1 = x + b->xoff2 - b->xoff;
            q->y1 = y + b->yoff2 - b->yoff;
        }
        else {
            q->x0 = *xpos + b->xoff;
            q->y0 = *ypos + b->yoff;
            q->x1 = *xpos + b->xoff2;
            q->y1 = *ypos + b->yoff2;
        }

        q->s0 = b->x0 * ipw;
        q->t0 = b->y0 * iph;
        q->s1 = b->x1 * ipw;
        q->t1 = b->y1 * iph;

        *xpos += b->xadvance;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // sdf computation
    //

#define STBTT_min(a, b) ((a) < (b) ? (a) : (b))
#define STBTT_max(a, b) ((a) < (b) ? (b) : (a))

    static i32 stbtt_ray_intersect_bezier(f32 orig[2], f32 ray[2], f32 q0[2], f32 q1[2], f32 q2[2],
                                          f32 hits[2][2])
    {
        f32 q0perp = q0[1] * ray[0] - q0[0] * ray[1];
        f32 q1perp = q1[1] * ray[0] - q1[0] * ray[1];
        f32 q2perp = q2[1] * ray[0] - q2[0] * ray[1];
        f32 roperp = orig[1] * ray[0] - orig[0] * ray[1];

        f32 a = q0perp - 2 * q1perp + q2perp;
        f32 b = q1perp - q0perp;
        f32 c = q0perp - roperp;

        f32 s0 = 0., s1 = 0.;
        i32 num_s = 0;

        if (a != 0.0) {
            f32 discr = b * b - a * c;
            if (discr > 0.0) {
                f32 rcpna = -1 / a;
                f32 d = std::sqrt(discr);
                s0 = (b + d) * rcpna;
                s1 = (b - d) * rcpna;
                if (s0 >= 0.0 && s0 <= 1.0)
                    num_s = 1;
                if (d > 0.0 && s1 >= 0.0 && s1 <= 1.0) {
                    if (num_s == 0)
                        s0 = s1;
                    ++num_s;
                }
            }
        }
        else {
            // 2*b*s + c = 0
            // s = -c / (2*b)
            s0 = c / (-2 * b);
            if (s0 >= 0.0 && s0 <= 1.0)
                num_s = 1;
        }

        if (num_s == 0)
            return 0;
        else {
            f32 rcp_len2 = 1 / (ray[0] * ray[0] + ray[1] * ray[1]);
            f32 rayn_x = ray[0] * rcp_len2, rayn_y = ray[1] * rcp_len2;

            f32 q0d = q0[0] * rayn_x + q0[1] * rayn_y;
            f32 q1d = q1[0] * rayn_x + q1[1] * rayn_y;
            f32 q2d = q2[0] * rayn_x + q2[1] * rayn_y;
            f32 rod = orig[0] * rayn_x + orig[1] * rayn_y;

            f32 q10d = q1d - q0d;
            f32 q20d = q2d - q0d;
            f32 q0rd = q0d - rod;

            hits[0][0] = q0rd + s0 * (2.0f - 2.0f * s0) * q10d + s0 * s0 * q20d;
            hits[0][1] = a * s0 + b;

            if (num_s > 1) {
                hits[1][0] = q0rd + s1 * (2.0f - 2.0f * s1) * q10d + s1 * s1 * q20d;
                hits[1][1] = a * s1 + b;
                return 2;
            }
            else {
                return 1;
            }
        }
    }

    static i32 equal(const f32* a, const f32* b)
    {
        return a[0] == b[0] && a[1] == b[1];
    }

    static i32 stbtt_compute_crossings_x(const f32 x, f32 y, const i32 nverts,
                                         const stbtt_vertex* verts)
    {
        f32 orig[2], ray[2] = { 1, 0 };
        i32 winding = 0;

        // make sure y never passes through a vertex of the shape
        f32 y_frac = std::fmod(y, 1.0f);
        if (y_frac < 0.01f)
            y += 0.01f;
        else if (y_frac > 0.99f)
            y -= 0.01f;

        orig[0] = x;
        orig[1] = y;

        // test a ray from (-infinity,y) to (x,y)
        for (i32 i = 0; i < nverts; ++i) {
            if (verts[i].type == STBTT_vline) {
                i32 x0 = verts[i - 1].x, y0 = verts[i - 1].y;
                i32 x1 = verts[i].x, y1 = verts[i].y;
                if (y > STBTT_min(y0, y1) && y < STBTT_max(y0, y1) && x > STBTT_min(x0, x1)) {
                    f32 x_inter = (y - y0) / (y1 - y0) * (x1 - x0) + x0;
                    if (x_inter < x)
                        winding += (y0 < y1) ? 1 : -1;
                }
            }
            if (verts[i].type == STBTT_vcurve) {
                i32 x0 = verts[i - 1].x, y0 = verts[i - 1].y;
                i32 x1 = verts[i].cx, y1 = verts[i].cy;
                i32 x2 = verts[i].x, y2 = verts[i].y;
                i32 ax = STBTT_min(x0, STBTT_min(x1, x2)), ay = STBTT_min(y0, STBTT_min(y1, y2));
                i32 by = STBTT_max(y0, STBTT_max(y1, y2));
                if (y > ay && y < by && x > ax) {
                    f32 q0[2], q1[2], q2[2];
                    f32 hits[2][2];
                    q0[0] = static_cast<f32>(x0);
                    q0[1] = static_cast<f32>(y0);
                    q1[0] = static_cast<f32>(x1);
                    q1[1] = static_cast<f32>(y1);
                    q2[0] = static_cast<f32>(x2);
                    q2[1] = static_cast<f32>(y2);
                    if (equal(q0, q1) || equal(q1, q2)) {
                        x0 = static_cast<i32>(verts[i - 1].x);
                        y0 = static_cast<i32>(verts[i - 1].y);
                        x1 = static_cast<i32>(verts[i].x);
                        y1 = static_cast<i32>(verts[i].y);
                        if (y > STBTT_min(y0, y1) && y < STBTT_max(y0, y1) &&
                            x > STBTT_min(x0, x1)) {
                            f32 x_inter = (y - y0) / (y1 - y0) * (x1 - x0) + x0;
                            if (x_inter < x)
                                winding += (y0 < y1) ? 1 : -1;
                        }
                    }
                    else {
                        i32 num_hits = stbtt_ray_intersect_bezier(orig, ray, q0, q1, q2, hits);
                        if (num_hits >= 1)
                            if (hits[0][0] < 0)
                                winding += (hits[0][1] < 0 ? -1 : 1);
                        if (num_hits >= 2)
                            if (hits[1][0] < 0)
                                winding += (hits[1][1] < 0 ? -1 : 1);
                    }
                }
            }
        }
        return winding;
    }

    static f32 stbtt_cuberoot(const f32 x)
    {
        if (x < 0)
            return -std::pow(-x, 1.0f / 3.0f);
        else
            return std::pow(x, 1.0f / 3.0f);
    }

    // x^3 + a*x^2 + b*x + c = 0
    static i32 stbtt_solve_cubic(const f32 a, const f32 b, const f32 c, f32* r)
    {
        f32 s = -a / 3;
        f32 p = b - a * a / 3;
        f32 q = a * (2 * a * a - 9 * b) / 27 + c;
        f32 p3 = p * p * p;
        f32 d = q * q + 4 * p3 / 27;
        if (d >= 0) {
            f32 z = std::sqrt(d);
            f32 u = (-q + z) / 2;
            f32 v = (-q - z) / 2;
            u = stbtt_cuberoot(u);
            v = stbtt_cuberoot(v);
            r[0] = s + u + v;
            return 1;
        }
        else {
            f32 u = std::sqrt(-p / 3);
            f32 v = std::acos(-std::sqrt(-27 / p3) * q / 2) / 3;  // p3 must be negative,
                                                                  // since d is negative
            f32 m = std::cos(v);
            f32 n = static_cast<f32>(std::cos(v - 3.141592 / 2)) * 1.732050808f;
            r[0] = s + u * 2 * m;
            r[1] = s - u * (m + n);
            r[2] = s - u * (m - n);

            // debug_assert( std::fabs(((r[0]+a)*r[0]+b)*r[0]+c) < 0.05f);  // these asserts may
            // not be safe at all scales, though they're in bezier t parameter units so maybe?
            // debug_assert( std::fabs(((r[1]+a)*r[1]+b)*r[1]+c) < 0.05f);
            // debug_assert( std::fabs(((r[2]+a)*r[2]+b)*r[2]+c) < 0.05f);
            return 3;
        }
    }

    u8* stbtt_GetGlyphSDF(const stbtt_fontinfo* info, f32 scale, i32 glyph, i32 padding,
                          u8 onedge_value, f32 pixel_dist_scale, i32* width, i32* height, i32* xoff,
                          i32* yoff)
    {
        f32 scale_x = scale, scale_y = scale;
        i32 ix0, iy0, ix1, iy1;
        i32 w, h;
        u8* data;

        if (scale == 0)
            return nullptr;

        stbtt_get_glyph_bitmap_box_subpixel(info, glyph, scale, scale, 0.0f, 0.0f, &ix0, &iy0, &ix1,
                                            &iy1);

        // if empty, return NULL
        if (ix0 == ix1 || iy0 == iy1)
            return nullptr;

        ix0 -= padding;
        iy0 -= padding;
        ix1 += padding;
        iy1 += padding;

        w = (ix1 - ix0);
        h = (iy1 - iy0);

        if (width)
            *width = w;
        if (height)
            *height = h;
        if (xoff)
            *xoff = ix0;
        if (yoff)
            *yoff = iy0;

        // invert for y-downwards bitmaps
        scale_y = -scale_y;

        {
            i32 x, y, i, j;
            f32* precompute;
            stbtt_vertex* verts;
            i32 num_verts = stbtt_get_glyph_shape(info, glyph, &verts);
            data = static_cast<u8*>(std::malloc(w * h));
            precompute = static_cast<f32*>(std::malloc(num_verts * sizeof(f32)));

            for (i = 0, j = num_verts - 1; i < num_verts; j = i++) {
                if (verts[i].type == STBTT_vline) {
                    f32 x0 = verts[i].x * scale_x, y0 = verts[i].y * scale_y;
                    f32 x1 = verts[j].x * scale_x, y1 = verts[j].y * scale_y;
                    f32 dist = std::sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
                    precompute[i] = (dist == 0) ? 0.0f : 1.0f / dist;
                }
                else if (verts[i].type == STBTT_vcurve) {
                    f32 x2 = verts[j].x * scale_x, y2 = verts[j].y * scale_y;
                    f32 x1 = verts[i].cx * scale_x, y1 = verts[i].cy * scale_y;
                    f32 x0 = verts[i].x * scale_x, y0 = verts[i].y * scale_y;
                    f32 bx = x0 - 2 * x1 + x2, by = y0 - 2 * y1 + y2;
                    f32 len2 = bx * bx + by * by;
                    if (len2 != 0.0f)
                        precompute[i] = 1.0f / (bx * bx + by * by);
                    else
                        precompute[i] = 0.0f;
                }
                else
                    precompute[i] = 0.0f;
            }

            for (y = iy0; y < iy1; ++y) {
                for (x = ix0; x < ix1; ++x) {
                    f32 val;
                    f32 min_dist = 999999.0f;
                    f32 sx = static_cast<f32>(x) + 0.5f;
                    f32 sy = static_cast<f32>(y) + 0.5f;
                    f32 x_gspace = (sx / scale_x);
                    f32 y_gspace = (sy / scale_y);

                    // @OPTIMIZE: this could just be a rasterization, but needs to be
                    // line vs. non-tesselated curves so a new path
                    i32 winding = stbtt_compute_crossings_x(x_gspace, y_gspace, num_verts, verts);

                    for (i = 0; i < num_verts; ++i) {
                        f32 x0 = verts[i].x * scale_x, y0 = verts[i].y * scale_y;

                        if (verts[i].type == STBTT_vline && precompute[i] != 0.0f) {
                            f32 x1 = verts[i - 1].x * scale_x, y1 = verts[i - 1].y * scale_y;

                            f32 dist, dist2 = (x0 - sx) * (x0 - sx) + (y0 - sy) * (y0 - sy);
                            if (dist2 < min_dist * min_dist)
                                min_dist = std::sqrt(dist2);

                            // coarse culling against bbox
                            // if (sx > STBTT_min(x0,x1)-min_dist && sx < STBTT_max(x0,x1)+min_dist
                            // &&
                            //    sy > STBTT_min(y0,y1)-min_dist && sy < STBTT_max(y0,y1)+min_dist)
                            dist = std::fabs((x1 - x0) * (y0 - sy) - (y1 - y0) * (x0 - sx)) *
                                   precompute[i];
                            debug_assert(i != 0);
                            if (dist < min_dist) {
                                // check position along line
                                // x' = x0 + t*(x1-x0), y' = y0 + t*(y1-y0)
                                // minimize (x'-sx)*(x'-sx)+(y'-sy)*(y'-sy)
                                f32 dx = x1 - x0, dy = y1 - y0;
                                f32 px = x0 - sx, py = y0 - sy;
                                // minimize (px+t*dx)^2 + (py+t*dy)^2 = px*px + 2*px*dx*t +
                                // t^2*dx*dx + py*py + 2*py*dy*t + t^2*dy*dy derivative: 2*px*dx +
                                // 2*py*dy + (2*dx*dx+2*dy*dy)*t, set to 0 and solve
                                f32 t = -(px * dx + py * dy) / (dx * dx + dy * dy);
                                if (t >= 0.0f && t <= 1.0f)
                                    min_dist = dist;
                            }
                        }
                        else if (verts[i].type == STBTT_vcurve) {
                            f32 x2 = verts[i - 1].x * scale_x, y2 = verts[i - 1].y * scale_y;
                            f32 x1 = verts[i].cx * scale_x, y1 = verts[i].cy * scale_y;
                            f32 box_x0 = STBTT_min(STBTT_min(x0, x1), x2);
                            f32 box_y0 = STBTT_min(STBTT_min(y0, y1), y2);
                            f32 box_x1 = STBTT_max(STBTT_max(x0, x1), x2);
                            f32 box_y1 = STBTT_max(STBTT_max(y0, y1), y2);
                            // coarse culling against bbox to avoid computing cubic unnecessarily
                            if (sx > box_x0 - min_dist && sx < box_x1 + min_dist &&
                                sy > box_y0 - min_dist && sy < box_y1 + min_dist) {
                                i32 num = 0;
                                f32 ax = x1 - x0, ay = y1 - y0;
                                f32 bx = x0 - 2 * x1 + x2, by = y0 - 2 * y1 + y2;
                                f32 mx = x0 - sx, my = y0 - sy;
                                f32 res[3] = { 0.f, 0.f, 0.f };
                                f32 px, py, t, it, dist2;
                                f32 a_inv = precompute[i];
                                if (a_inv == 0.0) {  // if a_inv is 0, it's 2nd degree so use
                                                     // quadratic formula
                                    f32 a = 3 * (ax * bx + ay * by);
                                    f32 b = 2 * (ax * ax + ay * ay) + (mx * bx + my * by);
                                    f32 c = mx * ax + my * ay;
                                    if (a == 0.0) {  // if a is 0, it's linear
                                        if (b != 0.0)
                                            res[num++] = -c / b;
                                    }
                                    else {
                                        f32 discriminant = b * b - 4 * a * c;
                                        if (discriminant < 0)
                                            num = 0;
                                        else {
                                            f32 root = std::sqrt(discriminant);
                                            res[0] = (-b - root) / (2 * a);
                                            res[1] = (-b + root) / (2 * a);
                                            num = 2;  // don't bother distinguishing 1-solution
                                                      // case, as code below will still work
                                        }
                                    }
                                }
                                else {
                                    f32 b = 3 * (ax * bx + ay * by) * a_inv;  // could precompute
                                                                              // this as it
                                                                              // doesn't depend on
                                                                              // sample point
                                    f32 c = (2 * (ax * ax + ay * ay) + (mx * bx + my * by)) * a_inv;
                                    f32 d = (mx * ax + my * ay) * a_inv;
                                    num = stbtt_solve_cubic(b, c, d, res);
                                }
                                dist2 = (x0 - sx) * (x0 - sx) + (y0 - sy) * (y0 - sy);
                                if (dist2 < min_dist * min_dist)
                                    min_dist = std::sqrt(dist2);

                                if (num >= 1 && res[0] >= 0.0f && res[0] <= 1.0f) {
                                    t = res[0], it = 1.0f - t;
                                    px = it * it * x0 + 2 * t * it * x1 + t * t * x2;
                                    py = it * it * y0 + 2 * t * it * y1 + t * t * y2;
                                    dist2 = (px - sx) * (px - sx) + (py - sy) * (py - sy);
                                    if (dist2 < min_dist * min_dist)
                                        min_dist = std::sqrt(dist2);
                                }
                                if (num >= 2 && res[1] >= 0.0f && res[1] <= 1.0f) {
                                    t = res[1], it = 1.0f - t;
                                    px = it * it * x0 + 2 * t * it * x1 + t * t * x2;
                                    py = it * it * y0 + 2 * t * it * y1 + t * t * y2;
                                    dist2 = (px - sx) * (px - sx) + (py - sy) * (py - sy);
                                    if (dist2 < min_dist * min_dist)
                                        min_dist = std::sqrt(dist2);
                                }
                                if (num >= 3 && res[2] >= 0.0f && res[2] <= 1.0f) {
                                    t = res[2], it = 1.0f - t;
                                    px = it * it * x0 + 2 * t * it * x1 + t * t * x2;
                                    py = it * it * y0 + 2 * t * it * y1 + t * t * y2;
                                    dist2 = (px - sx) * (px - sx) + (py - sy) * (py - sy);
                                    if (dist2 < min_dist * min_dist)
                                        min_dist = std::sqrt(dist2);
                                }
                            }
                        }
                    }
                    if (winding == 0)
                        min_dist = -min_dist;  // if outside the shape, value is negative
                    val = onedge_value + pixel_dist_scale * min_dist;
                    if (val < 0)
                        val = 0;
                    else if (val > 255)
                        val = 255;
                    data[(y - iy0) * w + (x - ix0)] = static_cast<u8>(val);
                }
            }
            std::free(precompute);
            std::free(verts);
        }
        return data;
    }

    u8* stbtt_get_codepoint_sdf(const stbtt_fontinfo* info, const f32 scale, const i32 codepoint,
                                const i32 padding, const u8 onedge_value,
                                const f32 pixel_dist_scale, i32* width, i32* height, i32* xoff,
                                i32* yoff)
    {
        return stbtt_GetGlyphSDF(info, scale, stbtt_find_glyph_index(info, codepoint), padding,
                                 onedge_value, pixel_dist_scale, width, height, xoff, yoff);
    }

    void stbtt_free_sdf(u8* bitmap, const void*)
    {
        std::free(bitmap);
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // font name matching -- recommended not to use this
    //

    // check if a utf8 string contains a prefix which is the utf16 string; if so return length of
    // matching utf8 string
    static i32 stbtt_compare_utf8_to_utf16_bigendian_prefix(const u8* s1, const i32 len1,
                                                            const u8* s2, i32 len2)
    {
        i32 i = 0;

        // convert utf16 to utf8 and compare the results while converting
        while (len2) {
            u16 ch = s2[0] * 256 + s2[1];
            if (ch < 0x80) {
                if (i >= len1)
                    return -1;
                if (s1[i++] != ch)
                    return -1;
            }
            else if (ch < 0x800) {
                if (i + 1 >= len1)
                    return -1;
                if (s1[i++] != 0xc0 + (ch >> 6))
                    return -1;
                if (s1[i++] != 0x80 + (ch & 0x3f))
                    return -1;
            }
            else if (ch >= 0xd800 && ch < 0xdc00) {
                u16 ch2 = s2[2] * 256 + s2[3];
                if (i + 3 >= len1)
                    return -1;
                u32 c = ((ch - 0xd800) << 10) + (ch2 - 0xdc00) + 0x10000;
                if (s1[i++] != 0xf0 + (c >> 18))
                    return -1;
                if (s1[i++] != 0x80 + ((c >> 12) & 0x3f))
                    return -1;
                if (s1[i++] != 0x80 + ((c >> 6) & 0x3f))
                    return -1;
                if (s1[i++] != 0x80 + ((c) & 0x3f))
                    return -1;
                s2 += 2;  // plus another 2 below
                len2 -= 2;
            }
            else if (ch >= 0xdc00 && ch < 0xe000) {
                return -1;
            }
            else {
                if (i + 2 >= len1)
                    return -1;
                if (s1[i++] != 0xe0 + (ch >> 12))
                    return -1;
                if (s1[i++] != 0x80 + ((ch >> 6) & 0x3f))
                    return -1;
                if (s1[i++] != 0x80 + ((ch) & 0x3f))
                    return -1;
            }
            s2 += 2;
            len2 -= 2;
        }
        return i;
    }

    static i32 stbtt_compare_utf8_to_utf16_bigendian_internal(char* s1, const i32 len1, char* s2,
                                                              const i32 len2)
    {
        return len1 == stbtt_compare_utf8_to_utf16_bigendian_prefix((u8*)s1, len1, (u8*)s2, len2);
    }

    // returns results in whatever encoding you request... but note that 2-byte encodings
    // will be BIG-ENDIAN... use stbtt_CompareUTF8toUTF16_bigendian() to compare
    const char* stbtt_get_font_name_string(const stbtt_fontinfo* font, i32* length,
                                           const i32 platform_id, const i32 encoding_id,
                                           const i32 language_id, const i32 name_id)
    {
        u8* fc = font->data;
        u32 offset = font->fontstart;
        u32 nm = stbtt_find_table(fc, offset, "name");
        if (!nm)
            return nullptr;

        i32 count = tt_aligned_u16(fc + nm + 2);
        i32 string_offset = nm + tt_aligned_u16(fc + nm + 4);
        for (i32 i = 0; i < count; ++i) {
            u32 loc = nm + 6 + 12 * i;
            if (platform_id == tt_aligned_u16(fc + loc + 0) &&
                encoding_id == tt_aligned_u16(fc + loc + 2) &&
                language_id == tt_aligned_u16(fc + loc + 4) &&
                name_id == tt_aligned_u16(fc + loc + 6)) {
                *length = tt_aligned_u16(fc + loc + 8);
                return reinterpret_cast<const char*>(
                    fc + string_offset + tt_aligned_u16(fc + loc + 10));
            }
        }
        return nullptr;
    }

    static i32 stbtt_matchpair(u8* fc, const u32 nm, u8* name, const i32 nlen, const i32 target_id,
                               const i32 next_id)
    {
        i32 count = tt_aligned_u16(fc + nm + 2);
        i32 string_offset = nm + tt_aligned_u16(fc + nm + 4);

        for (i32 i = 0; i < count; ++i) {
            u32 loc = nm + 6 + 12 * i;
            i32 id = tt_aligned_u16(fc + loc + 6);
            if (id == target_id) {
                i32 platform = tt_aligned_u16(fc + loc + 0);
                i32 encoding = tt_aligned_u16(fc + loc + 2);
                i32 language = tt_aligned_u16(fc + loc + 4);

                // is this a Unicode encoding?
                if (platform == 0 || (platform == 3 && encoding == 1) ||
                    (platform == 3 && encoding == 10)) {
                    i32 slen = tt_aligned_u16(fc + loc + 8);
                    i32 off = tt_aligned_u16(fc + loc + 10);

                    // check if there's a prefix match
                    i32 matchlen = stbtt_compare_utf8_to_utf16_bigendian_prefix(
                        name, nlen, fc + string_offset + off, slen);
                    if (matchlen >= 0) {
                        // check for target_id+1 immediately following, with same encoding &
                        // language
                        if (i + 1 < count && tt_aligned_u16(fc + loc + 12 + 6) == next_id &&
                            tt_aligned_u16(fc + loc + 12) == platform &&
                            tt_aligned_u16(fc + loc + 12 + 2) == encoding &&
                            tt_aligned_u16(fc + loc + 12 + 4) == language) {
                            slen = tt_aligned_u16(fc + loc + 12 + 8);
                            off = tt_aligned_u16(fc + loc + 12 + 10);
                            if (slen == 0) {
                                if (matchlen == nlen)
                                    return 1;
                            }
                            else if (matchlen < nlen && name[matchlen] == ' ') {
                                ++matchlen;
                                if (stbtt_compare_utf8_to_utf16_bigendian_internal(
                                        reinterpret_cast<char*>(name + matchlen), nlen - matchlen,
                                        reinterpret_cast<char*>(fc + string_offset + off), slen))
                                    return 1;
                            }
                        }
                        else {
                            // if nothing immediately following
                            if (matchlen == nlen)
                                return 1;
                        }
                    }
                }

                // @TODO handle other encodings
            }
        }
        return 0;
    }

    static i32 stbtt_matches(u8* fc, const u32 offset, u8* name, const i32 flags)
    {
        i32 nlen = static_cast<i32>(std::strlen(reinterpret_cast<char*>(name)));
        if (!stbtt_isfont(fc + offset))
            return 0;

        // check italics/bold/underline flags in macStyle...
        if (flags) {
            u32 hd = stbtt_find_table(fc, offset, "head");
            if ((tt_aligned_u16(fc + hd + 44) & 7) != (flags & 7))
                return 0;
        }

        u32 nm = stbtt_find_table(fc, offset, "name");
        if (!nm)
            return 0;

        if (flags) {
            // if we checked the macStyle flags, then just check the family and ignore the subfamily
            if (stbtt_matchpair(fc, nm, name, nlen, 16, -1))
                return 1;
            if (stbtt_matchpair(fc, nm, name, nlen, 1, -1))
                return 1;
            if (stbtt_matchpair(fc, nm, name, nlen, 3, -1))
                return 1;
        }
        else {
            if (stbtt_matchpair(fc, nm, name, nlen, 16, 17))
                return 1;
            if (stbtt_matchpair(fc, nm, name, nlen, 1, 2))
                return 1;
            if (stbtt_matchpair(fc, nm, name, nlen, 3, -1))
                return 1;
        }

        return 0;
    }

    static i32 stbtt_find_matching_font_internal(u8* font_collection, char* name_utf8,
                                                 const i32 flags)
    {
        for (i32 i = 0; /*empty*/; ++i) {
            i32 off = stbtt_get_font_offset_for_index(font_collection, i);
            if (off < 0)
                return off;
            if (stbtt_matches(font_collection, off, reinterpret_cast<u8*>(name_utf8), flags))
                return off;
        }
    }

    i32 stbtt_bake_font_bitmap(const u8* data, const i32 offset, const f32 pixel_height, u8* pixels,
                               const i32 pw, const i32 ph, const i32 first_char,
                               const i32 num_chars, stbtt_bakedchar* chardata)
    {
        return stbtt_bake_font_bitmap_internal(data, offset, pixel_height, pixels, pw, ph,
                                               first_char, num_chars, chardata);
    }

    i32 stbtt_get_font_offset_for_index(const u8* data, const i32 index)
    {
        return stbtt_GetFontOffsetForIndex_internal(data, index);
    }

    i32 stbtt_get_number_of_fonts(const u8* data)
    {
        return stbtt_GetNumberOfFonts_internal(data);
    }

    i32 stbtt_init_font(stbtt_fontinfo* info, const u8* data, const i32 offset)
    {
        return stbtt_InitFont_internal(info, const_cast<u8*>(data), offset);
    }

    i32 stbtt_find_matching_font(const u8* fontdata, const char* name, const i32 flags)
    {
        return stbtt_find_matching_font_internal(const_cast<u8*>(fontdata), const_cast<char*>(name),
                                                 flags);
    }

    i32 stbtt_compare_utf8_to_utf16_bigendian(const char* s1, const i32 len1, const char* s2,
                                              const i32 len2)
    {
        return stbtt_compare_utf8_to_utf16_bigendian_internal(const_cast<char*>(s1), len1,
                                                              const_cast<char*>(s2), len2);
    }
}
