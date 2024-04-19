#pragma once

#include "utils/numeric.hpp"

#ifdef _MSC_VER
  #define STBTT_NOT_USED(v) (void)(v)
#else
  #define STBTT_NOT_USED(v) (void)sizeof(v)
#endif

namespace rl::stb {
    // using stbtt_ui328 = u8;
    // using stbtt_i328 = i8;
    // using stbtt_ui3216 = u16;
    // using stbtt_i3216 = i16;
    // using stbtt_ui3232 = u32;
    // using stbtt_i3232 = i32;

    typedef char stbtt_check_size32[sizeof(i32) == 4 ? 1 : -1];
    typedef char stbtt_check_size16[sizeof(i16) == 2 ? 1 : -1];

    // private structure
    struct stbtt_buf
    {
        u8* data;
        i32 cursor;
        i32 size;
    };

    //////////////////////////////////////////////////////////////////////////////
    //
    // TEXTURE BAKING API
    //
    // If you use this API, you only have to call two functions ever.
    //

    struct stbtt_bakedchar
    {
        u16 x0, y0, x1, y1;  // coordinates of bbox in bitmap
        f32 xoff, yoff, xadvance;
    };

    i32 stbtt_bake_font_bitmap(const u8* data, i32 offset,     // font location
                                                               // (use offset=0
                                                               // for plain
                                                               // .ttf)
                               f32 pixel_height,               // height of font in pixels
                               u8* pixels, i32 pw, i32 ph,     // bitmap to be
                                                               // filled in
                               i32 first_char, i32 num_chars,  // characters to bake
                               stbtt_bakedchar* chardata);     // you allocate this, it's
                                                               // num_chars long

    // if return is positive, the first unused row of the bitmap
    // if return is negative, returns the negative of the number of characters that fit
    // if return is 0, no characters fit and no rows were used
    // This uses a very crappy packing.

    struct stbtt_aligned_quad
    {
        f32 x0, y0, s0, t0;  // top-left
        f32 x1, y1, s1, t1;  // bottom-right
    };

    void stbtt_get_baked_quad(const stbtt_bakedchar* chardata, i32 pw, i32 ph,  // same
                                                                                // data
                                                                                // as
                                                                                // above
                              i32 char_index,                                   // character to display
                              f32* xpos, const f32* ypos,                       // poi32ers to current position
                                                                                // in screen pixel space
                              stbtt_aligned_quad* q,                            // output: quad to draw
                              i32 opengl_fillrule);                             // true if opengl fill rule; false
                                                                                // if DX9 or earlier
    // Call GetBakedQuad with char_index = 'character - first_char', and it
    // creates the quad you need to draw and advances the current position.
    //
    // The coordinate system used assumes y increases downwards.
    //
    // Characters will extend both above and below the current position;
    // see discussion of "BASELINE" above.
    //
    // It's inefficient; you might want to c&p it and optimize it.

    void stbtt_get_scaled_font_v_metrics(const u8* fontdata, i32 index, f32 size,
                                         f32* ascent, f32* descent, f32* lineGap);

    // Query the font vertical metrics without having to create a font first.

    //////////////////////////////////////////////////////////////////////////////
    //
    // NEW TEXTURE BAKING API
    //
    // This provides options for packing multiple fonts i32o one atlas, not
    // perfectly but better than nothing.

    struct stbtt_packedchar
    {
        u16 x0, y0, x1, y1;  // coordinates of bbox in bitmap
        f32 xoff, yoff, xadvance;
        f32 xoff2, yoff2;
    };

    typedef struct stbtt_pack_context stbtt_pack_context;
    typedef struct stbtt_fontinfo stbtt_fontinfo;
#ifndef STB_RECT_PACK_VERSION
    typedef struct stbrp_rect stbrp_rect;
#endif

    i32 stbtt_pack_begin(stbtt_pack_context* spc, u8* pixels, i32 width, i32 height,
                         i32 stride_in_bytes, i32 padding, void* alloc_context);
    // Initializes a packing context stored in the passed-in stbtt_pack_context.
    // Future calls using this context will pack characters i32o the bitmap passed
    // in here: a 1-channel bitmap that is width * height. stride_in_bytes is
    // the distance from one row to the next (or 0 to mean they are packed tightly
    // together). "padding" is the amount of padding to leave between each
    // character (normally you want '1' for bitmaps you'll use as textures with
    // bilinear filtering).
    //
    // Returns 0 on failure, 1 on success.

    void stbtt_pack_end(const stbtt_pack_context* spc);
    // Cleans up the packing context and frees all memory.

    i32 stbtt_pack_font_range(stbtt_pack_context* spc, const u8* fontdata,
                              i32 font_index, f32 font_size, i32 first_unicode_codepoint_in_range,
                              i32 num_chars_in_range, stbtt_packedchar* chardata_for_range);

    // Creates character bitmaps from the font_index'th font found in fontdata (use
    // font_index=0 if you don't know what that is). It creates num_chars_in_range
    // bitmaps for characters with unicode values starting at first_unicode_char_in_range
    // and increasing. Data for how to render them is stored in chardata_for_range;
    // pass these to stbtt_GetPackedQuad to get back renderable quads.
    //
    // font_size is the full height of the character from ascender to descender,
    // as computed by stbtt_ScaleForPixelHeight. To use a poi32 size as computed
    // by stbtt_ScaleForMappingEmToPixels, wrap the poi32 size in STBTT_POi32_SIZE()
    // and pass that result as 'font_size':
    //       ...,                  20 , ... // font max minus min y is 20 pixels tall
    //       ..., STBTT_POi32_SIZE(20), ... // 'M' is 20 pixels tall

    struct stbtt_pack_range
    {
        f32 font_size;
        i32 first_unicode_codepoint_in_range;  // if non-zero, then the chars are continuous, and this is the first codepoint
        i32* array_of_unicode_codepoints;      // if non-zero, then this is an array of unicode codepoints
        i32 num_chars;
        stbtt_packedchar* chardata_for_range;  // output
        u8 h_oversample, v_oversample;         // don't set these, they're used i32ernally
    };

    i32 stbtt_pack_font_ranges(stbtt_pack_context* spc, const u8* fontdata,
                               i32 font_index, stbtt_pack_range* ranges, i32 num_ranges);
    // Creates character bitmaps from multiple ranges of characters stored in
    // ranges. This will usually create a better-packed bitmap than multiple
    // calls to stbtt_PackFontRange. Note that you can call this multiple
    // times within a single PackBegin/PackEnd.

    void stbtt_pack_set_oversampling(stbtt_pack_context* spc, u32 h_oversample,
                                     u32 v_oversample);
    // Oversampling a font increases the quality by allowing higher-quality subpixel
    // positioning, and is especially valuable at smaller text sizes.
    //
    // This function sets the amount of oversampling for all following calls to
    // stbtt_PackFontRange(s) or stbtt_PackFontRangesGatherRects for a given
    // pack context. The default (no oversampling) is achieved by h_oversample=1
    // and v_oversample=1. The total number of pixels required is
    // h_oversample*v_oversample larger than the default; for example, 2x2
    // oversampling requires 4x the storage of 1x1. For best results, render
    // oversampled textures with bilinear filtering. Look at the readme in
    // stb/tests/oversample for information about oversampled fonts
    //
    // To use with PackFontRangesGather etc., you must set it before calls
    // call to PackFontRangesGatherRects.

    void stbtt_pack_set_skip_missing_codepoints(stbtt_pack_context* spc, i32 skip);
    // If skip != 0, this tells stb_truetype to skip any codepoints for which
    // there is no corresponding glyph. If skip=0, which is the default, then
    // codepoints without a glyph recived the font's "missing character" glyph,
    // typically an empty box by convention.

    void stbtt_get_packed_quad(const stbtt_packedchar* chardata, i32 pw,
                               i32 ph,                      // same
                                                            // data as
                                                            // above
                               i32 char_index,              // character to display
                               f32* xpos, const f32* ypos,  // poi32ers to current
                                                            // position in screen pixel
                                                            // space
                               stbtt_aligned_quad* q,       // output: quad to draw
                               i32 align_to_integer);

    i32 stbtt_pack_font_ranges_gather_rects(const stbtt_pack_context* spc, const stbtt_fontinfo* info,
                                            stbtt_pack_range* ranges, i32 num_ranges,
                                            stbrp_rect* rects);
    void stbtt_pack_font_ranges_pack_rects(const stbtt_pack_context* spc, stbrp_rect* rects,
                                           i32 num_rects);
    i32 stbtt_pack_font_ranges_render_i32o_rects(
        stbtt_pack_context* spc, const stbtt_fontinfo* info, const stbtt_pack_range* ranges,
        i32 num_ranges, stbrp_rect* rects);

    // Calling these functions in sequence is roughly equivalent to calling
    // stbtt_PackFontRanges(). If you more control over the packing of multiple
    // fonts, or if you want to pack custom data i32o a font texture, take a look
    // at the source to of stbtt_PackFontRanges() and create a custom version
    // using these functions, e.g. call GatherRects multiple times,
    // building up a single array of rects, then call PackRects once,
    // then call Renderi32oRects repeatedly. This may result in a
    // better packing than calling PackFontRanges multiple times
    // (or it may not).

    // this is an opaque structure that you shouldn't mess with which holds
    // all the context needed from PackBegin to PackEnd.
    struct stbtt_pack_context
    {
        void* user_allocator_context;
        void* pack_info;
        i32 width;
        i32 height;
        i32 stride_in_bytes;
        i32 padding;
        i32 skip_missing;
        u32 h_oversample, v_oversample;
        u8* pixels;
        void* nodes;
    };

    //////////////////////////////////////////////////////////////////////////////
    //
    // FONT LOADING
    //
    //

    i32 stbtt_get_number_of_fonts(const u8* data);
    // This function will determine the number of fonts in a font file.  TrueType
    // collection (.ttc) files may contain multiple fonts, while TrueType font
    // (.ttf) files only contain one font. The number of fonts can be used for
    // indexing with the previous function where the index is between zero and one
    // less than the total fonts. If an error occurs, -1 is returned.

    i32 stbtt_get_font_offset_for_index(const u8* data, i32 index);

    // Each .ttf/.ttc file may have more than one font. Each font has a sequential
    // index number starting from 0. Call this function to get the font offset for
    // a given index; it returns -1 if the index is out of range. A regular .ttf
    // file will only define one font and it always be at offset 0, so it will
    // return '0' for index 0, and -1 for all other indices.

    // The following structure is defined publicly so you can declare one on
    // the stack or as a global or etc, but you should treat it as opaque.
    struct stbtt_fontinfo
    {
        void* userdata;
        u8* data;       // poi32er to .ttf file
        i32 fontstart;  // offset of start of font

        i32 num_glyphs;  // number of glyphs, needed for range checking

        i32 loca, head, glyf, hhea, hmtx, kern, gpos, svg;  // table locations as offset from
                                                            // start of .ttf
        i32 index_map;                                      // a cmap mapping for our chosen character encoding
        i32 index_to_loc_format;                            // format needed to map from glyph index to glyph

        stbtt_buf cff;          // cff font data
        stbtt_buf charstrings;  // the charstring index
        stbtt_buf gsubrs;       // global charstring subroutines index
        stbtt_buf subrs;        // private charstring subroutines index
        stbtt_buf fontdicts;    // array of font dicts
        stbtt_buf fdselect;     // map from glyph to fontdict
    };

    i32 stbtt_init_font(stbtt_fontinfo* info, const u8* data, i32 offset);
    // Given an offset i32o the file that defines a font, this function builds
    // the necessary cached info for the rest of the system. You must allocate
    // the stbtt_fontinfo yourself, and stbtt_InitFont will fill it out. You don't
    // need to do anything special to free it, because the contents are pure
    // value data with no additional data structures. Returns 0 on failure.

    //////////////////////////////////////////////////////////////////////////////
    //
    // CHARACTER TO GLYPH-INDEX CONVERSIOn

    i32 stbtt_find_glyph_index(const stbtt_fontinfo* info, i32 unicode_codepoint);
    // If you're going to perform multiple operations on the same character
    // and you want a speed-up, call this function with the character you're
    // going to process, then use glyph-based functions instead of the
    // codepoint-based functions.
    // Returns 0 if the character codepoint is not defined in the font.

    //////////////////////////////////////////////////////////////////////////////
    //
    // CHARACTER PROPERTIES
    //

    f32 stbtt_scale_for_pixel_height(const stbtt_fontinfo* info, f32 pixels);
    // computes a scale factor to produce a font whose "height" is 'pixels' tall.
    // Height is measured as the distance from the highest ascender to the lowest
    // descender; in other words, it's equivalent to calling stbtt_GetFontVMetrics
    // and computing:
    //       scale = pixels / (ascent - descent)
    // so if you prefer to measure height by the ascent only, use a similar calculation.

    f32 stbtt_scale_for_mapping_em_to_pixels(const stbtt_fontinfo* info, f32 pixels);
    // computes a scale factor to produce a font whose EM size is mapped to
    // 'pixels' tall. This is probably what traditional APIs compute, but
    // I'm not positive.

    void stbtt_get_font_v_metrics(const stbtt_fontinfo* info, i32* ascent, i32* descent, i32* lineGap);
    // ascent is the coordinate above the baseline the font extends; descent
    // is the coordinate below the baseline the font extends (i.e. it is typically negative)
    // lineGap is the spacing between one row's descent and the next row's ascent...
    // so you should advance the vertical position by "*ascent - *descent + *lineGap"
    //   these are expressed in unscaled coordinates, so you must multiply by
    //   the scale factor for a given size

    i32 stbtt_get_font_v_metrics_os2(const stbtt_fontinfo* info, i32* typoAscent, i32* typoDescent,
                                     i32* typoLineGap);
    // analogous to GetFontVMetrics, but returns the "typographic" values from the OS/2
    // table (specific to MS/Windows TTF files).
    //
    // Returns 1 on success (table present), 0 on failure.

    void stbtt_get_font_bounding_box(const stbtt_fontinfo* info, i32* x0, i32* y0, i32* x1, i32* y1);
    // the bounding box around all possible characters

    void stbtt_get_codepoint_h_metrics(const stbtt_fontinfo* info, i32 codepoint, i32* advanceWidth,
                                       i32* leftSideBearing);
    // leftSideBearing is the offset from the current horizontal position to the left edge of
    // the character advanceWidth is the offset from the current horizontal position to the next
    // horizontal position
    //   these are expressed in unscaled coordinates

    i32 stbtt_get_codepoint_kern_advance(const stbtt_fontinfo* info, i32 ch1, i32 ch2);
    // an additional amount to add to the 'advance' value between ch1 and ch2

    i32 stbtt_get_codepoint_box(const stbtt_fontinfo* info, i32 codepoint, i32* x0, i32* y0,
                                i32* x1, i32* y1);
    // Gets the bounding box of the visible part of the glyph, in unscaled coordinates

    void stbtt_get_glyph_h_metrics(const stbtt_fontinfo* info, i32 glyph_index, i32* advance_width,
                                   i32* left_side_bearing);
    i32 stbtt_get_glyph_kern_advance(const stbtt_fontinfo* info, i32 glyph1, i32 glyph2);
    i32 stbtt_get_glyph_box(const stbtt_fontinfo* info, i32 glyph_index, i32* x0, i32* y0, i32* x1,
                            i32* y1);

    // as above, but takes one or more glyph indices for greater efficiency

    typedef struct stbtt_kerningentry
    {
        i32 glyph1;  // use stbtt_FindGlyphIndex
        i32 glyph2;
        i32 advance;
    } stbtt_kerningentry;

    i32 stbtt_GetKerningTableLength(const stbtt_fontinfo* info);
    i32 stbtt_GetKerningTable(const stbtt_fontinfo* info, stbtt_kerningentry* table,
                              i32 table_length);
    // Retrieves a complete list of all of the kerning pairs provided by the font
    // stbtt_GetKerningTable never writes more than table_length entries and returns how many
    // entries it did write. The table will be sorted by (a.glyph1 == b.glyph1)?(a.glyph2 <
    // b.glyph2):(a.glyph1 < b.glyph1)

    //////////////////////////////////////////////////////////////////////////////
    //
    // GLYPH SHAPES (you probably don't need these, but they have to go before
    // the bitmaps for C declaration-order reasons)
    //

#ifndef STBTT_vmove  // you can predefine these to use different values (but why?)
    enum {
        STBTT_vmove = 1,
        STBTT_vline,
        STBTT_vcurve,
        STBTT_vcubic
    };
#endif

#ifndef stbtt_vertex  // you can predefine this to use different values
                      // (we share this with other code at RAD)
  #define stbtt_vertex_type \
      short  // can't use i16 because that's not visible in the header file

    typedef struct
    {
        stbtt_vertex_type x, y, cx, cy, cx1, cy1;
        u8 type, padding;
    } stbtt_vertex;
#endif

    i32 stbtt_is_glyph_empty(const stbtt_fontinfo* info, i32 glyph_index);
    // returns non-zero if nothing is drawn for this glyph

    i32 stbtt_get_codepoint_shape(const stbtt_fontinfo* info, i32 unicode_codepoint,
                                  stbtt_vertex** vertices);
    i32 stbtt_get_glyph_shape(const stbtt_fontinfo* info, i32 glyph_index, stbtt_vertex** vertices);
    // returns # of vertices and fills *vertices with the poi32er to them
    //   these are expressed in "unscaled" coordinates
    //
    // The shape is a series of contours. Each one starts with
    // a STBTT_moveto, then consists of a series of mixed
    // STBTT_lineto and STBTT_curveto segments. A lineto
    // draws a line from previous endpoi32 to its x,y; a curveto
    // draws a quadratic bezier from previous endpoi32 to
    // its x,y, using cx,cy as the bezier control poi32.

    void stbtt_FreeShape(const stbtt_fontinfo* info, stbtt_vertex* vertices);
    // frees the data allocated above

    u8* stbtt_find_svg_doc(const stbtt_fontinfo* info, i32 gl);
    i32 stbtt_get_codepoint_svg(const stbtt_fontinfo* info, i32 unicode_codepoint, const char** svg);
    i32 stbtt_get_glyph_svg(const stbtt_fontinfo* info, i32 gl, const char** svg);
    // fills svg with the character's SVG data.
    // returns data size or 0 if SVG not found.

    //////////////////////////////////////////////////////////////////////////////
    //
    // BITMAP RENDERING
    //

    void stbtt_FreeBitmap(u8* bitmap, const void* userdata);
    // frees the bitmap allocated below

    u8* stbtt_get_codepoint_bitmap(const stbtt_fontinfo* info, f32 scale_x,
                                   f32 scale_y, i32 codepoint, i32* width, i32* height,
                                   i32* xoff, i32* yoff);
    // allocates a large-enough single-channel 8bpp bitmap and renders the
    // specified character/glyph at the specified scale i32o it, with
    // antialiasing. 0 is no coverage (transparent), 255 is fully covered (opaque).
    // *width & *height are filled out with the width & height of the bitmap,
    // which is stored left-to-right, top-to-bottom.
    //
    // xoff/yoff are the offset it pixel space from the glyph origin to the top-left of the
    // bitmap

    u8* stbtt_get_codepoint_bitmap_subpixel(
        const stbtt_fontinfo* info, f32 scale_x, f32 scale_y, f32 shift_x, f32 shift_y,
        i32 codepoint, i32* width, i32* height, i32* xoff, i32* yoff);
    // the same as stbtt_GetCodepoitnBitmap, but you can specify a subpixel
    // shift for the character

    void stbtt_make_codepoint_bitmap(const stbtt_fontinfo* info, u8* output, i32 out_w,
                                     i32 out_h, i32 out_stride, f32 scale_x, f32 scale_y,
                                     i32 codepoint);
    // the same as stbtt_GetCodepoi32Bitmap, but you pass in storage for the bitmap
    // in the form of 'output', with row spacing of 'out_stride' bytes. the bitmap
    // is clipped to out_w/out_h bytes. Call stbtt_GetCodepoi32BitmapBox to get the
    // width and height and positioning info for it first.

    void stbtt_MakeCodepoi32BitmapSubpixel(
        const stbtt_fontinfo* info, u8* output, i32 out_w, i32 out_h, i32 out_stride,
        f32 scale_x, f32 scale_y, f32 shift_x, f32 shift_y, i32 codepoint);
    // same as stbtt_MakeCodepoi32Bitmap, but you can specify a subpixel
    // shift for the character

    void stbtt_make_codepoint_bitmap_subpixel_prefilter(
        const stbtt_fontinfo* info, u8* output, i32 out_w, i32 out_h, i32 out_stride,
        f32 scale_x, f32 scale_y, f32 shift_x, f32 shift_y, i32 oversample_x,
        i32 oversample_y, f32* sub_x, f32* sub_y, i32 codepoint);
    // same as stbtt_MakeCodepoi32BitmapSubpixel, but prefiltering
    // is performed (see stbtt_PackSetOversampling)

    void stbtt_get_codepoint_bitmap_box(const stbtt_fontinfo* font, i32 codepoint, f32 scale_x,
                                        f32 scale_y, i32* ix0, i32* iy0, i32* ix1, i32* iy1);
    // get the bbox of the bitmap centered around the glyph origin; so the
    // bitmap width is ix1-ix0, height is iy1-iy0, and location to place
    // the bitmap top left is (leftSideBearing*scale,iy0).
    // (Note that the bitmap uses y-increases-down, but the shape uses
    // y-increases-up, so Codepoi32BitmapBox and Codepoi32Box are inverted.)

    void stbtt_get_codepoint_bitmap_box_subpixel(const stbtt_fontinfo* font, i32 codepoint,
                                                 f32 scale_x, f32 scale_y, f32 shift_x,
                                                 f32 shift_y, i32* ix0, i32* iy0, i32* ix1, i32* iy1);
    // same as stbtt_GetCodepoi32BitmapBox, but you can specify a subpixel
    // shift for the character

    // the following functions are equivalent to the above functions, but operate
    // on glyph indices instead of Unicode codepoints (for efficiency)
    u8* stbtt_get_glyph_bitmap(const stbtt_fontinfo* info, f32 scale_x, f32 scale_y,
                               i32 glyph, i32* width, i32* height, i32* xoff, i32* yoff);
    u8* stbtt_get_glyph_bitmap_subpixel(
        const stbtt_fontinfo* info, f32 scale_x, f32 scale_y, f32 shift_x, f32 shift_y,
        i32 glyph, i32* width, i32* height, i32* xoff, i32* yoff);
    void stbtt_make_glyph_bitmap(const stbtt_fontinfo* info, u8* output, i32 out_w,
                                 i32 out_h, i32 out_stride, f32 scale_x, f32 scale_y, i32 glyph);
    void stbtt_make_glyph_bitmap_subpixel(const stbtt_fontinfo* info, u8* output, i32 out_w,
                                          i32 out_h, i32 out_stride, f32 scale_x, f32 scale_y,
                                          f32 shift_x, f32 shift_y, i32 glyph);
    void stbtt_MakeGlyphBitmapSubpixelPrefilter(
        const stbtt_fontinfo* info, u8* output, i32 out_w, i32 out_h, i32 out_stride,
        f32 scale_x, f32 scale_y, f32 shift_x, f32 shift_y, i32 oversample_x,
        i32 oversample_y, f32* sub_x, f32* sub_y, i32 glyph);
    void stbtt_get_glyph_bitmap_box(const stbtt_fontinfo* font, i32 glyph, f32 scale_x,
                                    f32 scale_y, i32* ix0, i32* iy0, i32* ix1, i32* iy1);
    void stbtt_get_glyph_bitmap_box_subpixel(const stbtt_fontinfo* font, i32 glyph, f32 scale_x,
                                             f32 scale_y, f32 shift_x, f32 shift_y, i32* ix0,
                                             i32* iy0, i32* ix1, i32* iy1);

    // @TODO: don't expose this structure
    typedef struct
    {
        i32 w, h, stride;
        u8* pixels;
    } stbtt_bitmap;

    // rasterize a shape with quadratic beziers i32o a bitmap
    void stbtt_rasterize(stbtt_bitmap* result,      // 1-channel bitmap to draw i32o
                         f32 flatness_in_pixels,    // allowable error of curve in
                                                    // pixels
                         stbtt_vertex* vertices,    // array of vertices defining shape
                         i32 num_verts,             // number of vertices in above array
                         f32 scale_x, f32 scale_y,  // scale applied to input
                                                    // vertices
                         f32 shift_x, f32 shift_y,  // translation applied to
                                                    // input vertices
                         i32 x_off, i32 y_off,      // another translation applied to
                                                    // input
                         i32 invert,                // if non-zero, vertically flip shape
                         void* userdata);           // context for to STBTT_MALLOC

    //////////////////////////////////////////////////////////////////////////////
    //
    // Signed Distance Function (or Field) rendering

    void stbtt_free_sdf(u8* bitmap, const void* userdata);
    // frees the SDF bitmap allocated below

    u8* stbtt_GetGlyphSDF(
        const stbtt_fontinfo* info, f32 scale, i32 glyph, i32 padding, u8 onedge_value,
        f32 pixel_dist_scale, i32* width, i32* height, i32* xoff, i32* yoff);
    u8* stbtt_get_codepoint_sdf(const stbtt_fontinfo* info, f32 scale, i32 codepoint,
                                i32 padding, u8 onedge_value,
                                f32 pixel_dist_scale, i32* width, i32* height,
                                i32* xoff, i32* yoff);
    // These functions compute a discretized SDF field for a single character, suitable for
    // storing in a single-channel texture, sampling with bilinear filtering, and testing
    // against larger than some threshold to produce scalable fonts.
    //        info              --  the font
    //        scale             --  controls the size of the resulting SDF bitmap, same as it
    //        would be creating a regular bitmap glyph/codepoint   --  the character to generate
    //        the SDF for padding           --  extra "pixels" around the character which are
    //        filled with the distance to the character (not 0),
    //                                 which allows effects like bit outlines
    //        onedge_value      --  value 0-255 to test the SDF against to reconstruct the
    //        character (i.e. the isocontour of the character) pixel_dist_scale  --  what value
    //        the SDF should increase by when moving one SDF "pixel" away from the edge (on the
    //        0..255 scale)
    //                                 if positive, > onedge_value is inside; if negative, <
    //                                 onedge_value is inside
    //        width,height      --  output height & width of the SDF bitmap (including padding)
    //        xoff,yoff         --  output origin of the character
    //        return value      --  a 2D array of bytes 0..255, width*height in size
    //
    // pixel_dist_scale & onedge_value are a scale & bias that allows you to make
    // optimal use of the limited 0..255 for your application, trading off precision
    // and special effects. SDF values outside the range 0..255 are clamped to 0..255.
    //
    // Example:
    //      scale = stbtt_ScaleForPixelHeight(22)
    //      padding = 5
    //      onedge_value = 180
    //      pixel_dist_scale = 180/5.0 = 36.0
    //
    //      This will create an SDF bitmap in which the character is about 22 pixels
    //      high but the whole bitmap is about 22+5+5=32 pixels high. To produce a filled
    //      shape, sample the SDF at each pixel and fill the pixel if the SDF value
    //      is greater than or equal to 180/255. (You'll actually want to antialias,
    //      which is beyond the scope of this example.) Additionally, you can compute
    //      offset outlines (e.g. to stroke the character border inside & outside,
    //      or only outside). For example, to fill outside the character up to 3 SDF
    //      pixels, you would compare against (180-36.0*3)/255 = 72/255. The above
    //      choice of variables maps a range from 5 pixels outside the shape to
    //      2 pixels inside the shape to 0..255; this is i32ended primarily for apply
    //      outside effects only (the i32erior range is needed to allow proper
    //      antialiasing of the font at *smaller* sizes)
    //
    // The function computes the SDF analytically at each SDF pixel, not by e.g.
    // building a higher-res bitmap and approximating it. In theory the quality
    // should be as high as possible for an SDF of this size & representation, but
    // unclear if this is true in practice (perhaps building a higher-res bitmap
    // and computing from that can allow drop-out prevention).
    //
    // The algorithm has not been optimized at all, so expect it to be slow
    // if computing lots of characters or very large sizes.

    //////////////////////////////////////////////////////////////////////////////
    //
    // Finding the right font...
    //
    // You should really just solve this offline, keep your own tables
    // of what font is what, and don't try to get it out of the .ttf file.
    // That's because getting it out of the .ttf file is really hard, because
    // the names in the file can appear in many possible encodings, in many
    // possible languages, and e.g. if you need a case-insensitive comparison,
    // the details of that depend on the encoding & language in a complex way
    // (actually underspecified in truetype, but also gigantic).
    //
    // But you can use the provided functions in two possible ways:
    //     stbtt_FindMatchingFont() will use *case-sensitive* comparisons on
    //             unicode-encoded names to try to find the font you want;
    //             you can run this before calling stbtt_InitFont()
    //
    //     stbtt_GetFontNameString() lets you get any of the various strings
    //             from the file yourself and do your own comparisons on them.
    //             You have to have called stbtt_InitFont() first.

    i32 stbtt_find_matching_font(const u8* fontdata, const char* name, i32 flags);
// returns the offset (not index) of the font that matches, or -1 if none
//   if you use STBTT_MACSTYLE_DONTCARE, use a font name like "Arial Bold".
//   if you use any other flag, use a font name like "Arial"; this checks
//     the 'macStyle' header field; i don't know if fonts set this consistently
#define STBTT_MACSTYLE_DONTCARE   0
#define STBTT_MACSTYLE_BOLD       1
#define STBTT_MACSTYLE_ITALIC     2
#define STBTT_MACSTYLE_UNDERSCORE 4
#define STBTT_MACSTYLE_NONE       8  // <= not same as 0, this makes us check the bitfield is 0

    i32 stbtt_compare_utf8_to_utf16_bigendian(const char* s1, i32 len1, const char* s2, i32 len2);
    // returns 1/0 whether the first string i32erpreted as utf8 is identical to
    // the second string i32erpreted as big-endian utf16... useful for strings from next func

    const char* stbtt_get_font_name_string(const stbtt_fontinfo* font, i32* length, i32 platform_id,
                                           i32 encoding_id, i32 language_id, i32 name_id);

    // returns the string (which may be big-endian double byte, e.g. for unicode)
    // and puts the length in bytes in *length.
    //
    // some of the values for the IDs are below; for more see the truetype spec:
    //     http://developer.apple.com/textfonts/TTRefMan/RM06/Chap6name.html
    //     http://www.microsoft.com/typography/otspec/name.htm

    enum {  // platformID
        STBTT_PLATFORM_ID_UNICODE = 0,
        STBTT_PLATFORM_ID_MAC = 1,
        STBTT_PLATFORM_ID_ISO = 2,
        STBTT_PLATFORM_ID_MICROSOFT = 3
    };

    enum {  // encodingID for STBTT_PLATFORM_ID_UNICODE
        STBTT_UNICODE_EID_UNICODE_1_0 = 0,
        STBTT_UNICODE_EID_UNICODE_1_1 = 1,
        STBTT_UNICODE_EID_ISO_10646 = 2,
        STBTT_UNICODE_EID_UNICODE_2_0_BMP = 3,
        STBTT_UNICODE_EID_UNICODE_2_0_FULL = 4
    };

    enum {  // encodingID for STBTT_PLATFORM_ID_MICROSOFT
        STBTT_MS_EID_SYMBOL = 0,
        STBTT_MS_EID_UNICODE_BMP = 1,
        STBTT_MS_EID_SHIFTJIS = 2,
        STBTT_MS_EID_UNICODE_FULL = 10
    };

    enum {  // encodingID for STBTT_PLATFORM_ID_MAC; same as Script Manager codes
        STBTT_MAC_EID_ROMAN = 0,
        STBTT_MAC_EID_ARABIC = 4,
        STBTT_MAC_EID_JAPANESE = 1,
        STBTT_MAC_EID_HEBREW = 5,
        STBTT_MAC_EID_CHINESE_TRAD = 2,
        STBTT_MAC_EID_GREEK = 6,
        STBTT_MAC_EID_KOREAN = 3,
        STBTT_MAC_EID_RUSSIAN = 7
    };

    enum {  // languageID for STBTT_PLATFORM_ID_MICROSOFT; same as LCID...
            // problematic because there are e.g. 16 english LCIDs and 16 arabic LCIDs
        STBTT_MS_LANG_ENGLISH = 0x0409,
        STBTT_MS_LANG_ITALIAN = 0x0410,
        STBTT_MS_LANG_CHINESE = 0x0804,
        STBTT_MS_LANG_JAPANESE = 0x0411,
        STBTT_MS_LANG_DUTCH = 0x0413,
        STBTT_MS_LANG_KOREAN = 0x0412,
        STBTT_MS_LANG_FRENCH = 0x040c,
        STBTT_MS_LANG_RUSSIAN = 0x0419,
        STBTT_MS_LANG_GERMAN = 0x0407,
        STBTT_MS_LANG_SPANISH = 0x0409,
        STBTT_MS_LANG_HEBREW = 0x040d,
        STBTT_MS_LANG_SWEDISH = 0x041D
    };

    enum {  // languageID for STBTT_PLATFORM_ID_MAC
        STBTT_MAC_LANG_ENGLISH = 0,
        STBTT_MAC_LANG_JAPANESE = 11,
        STBTT_MAC_LANG_ARABIC = 12,
        STBTT_MAC_LANG_KOREAN = 23,
        STBTT_MAC_LANG_DUTCH = 4,
        STBTT_MAC_LANG_RUSSIAN = 32,
        STBTT_MAC_LANG_FRENCH = 1,
        STBTT_MAC_LANG_SPANISH = 6,
        STBTT_MAC_LANG_GERMAN = 2,
        STBTT_MAC_LANG_SWEDISH = 5,
        STBTT_MAC_LANG_HEBREW = 10,
        STBTT_MAC_LANG_CHINESE_SIMPLIFIED = 33,
        STBTT_MAC_LANG_ITALIAN = 3,
        STBTT_MAC_LANG_CHINESE_TRAD = 19
    };

    u8* stbi_load(const char* filename, i32* p, i32* h, i32* n, i32 i);
}
