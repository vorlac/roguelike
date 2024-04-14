#pragma once

#include <cmath>

#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "graphics/stb/stb_truetype.hpp"
#include "utils/numeric.hpp"

namespace rl {
    enum class Align;
}

namespace rl::nvg::font {
    constexpr i32 INVALID{ -1 };
    constexpr i32 SCRATCH_BUF_SIZE{ 96000 };
    constexpr i32 HASH_LUT_SIZE{ 256 };
    constexpr i32 INIT_FONTS{ 4 };
    constexpr i32 INIT_GLYPHS{ 256 };
    constexpr i32 INIT_ATLAS_NODES{ 256 };
    constexpr i32 VERTEX_COUNT{ 1024 };
    constexpr i32 MAX_STATES{ 20 };
    constexpr i32 MAX_FALLBACKS{ 20 };

    struct STTFontImpl
    {
        stb::stbtt_fontinfo font{};
    };

    struct Glyph
    {
        u32 codepoint{ 0 };
        i32 index{ 0 };
        i32 next{ 0 };
        i16 size{ 0 };
        i16 blur{ 0 };
        i16 x0{ 0 };
        i16 y0{ 0 };
        i16 x1{ 0 };
        i16 y1{ 0 };
        i16 x_adv{ 0 };
        i16 x_off{ 0 };
        i16 y_off{ 0 };
    };

    struct Font
    {
        STTFontImpl font{};
        char name[64]{};
        u8* data{ nullptr };
        i32 data_size{ 0 };
        bool free_data{ false };
        f32 ascender{ 0.0f };
        f32 descender{ 0.0f };
        f32 lineh{ 0.0f };
        Glyph* glyphs{ nullptr };
        i32 cglyphs{ 0 };
        i32 nglyphs{ 0 };
        i32 lut[HASH_LUT_SIZE]{};
        i32 fallbacks[MAX_FALLBACKS]{};
        i32 nfallbacks{ 0 };
    };

    struct State
    {
        i32 font{ 0 };
        Align align{ 0 };
        f32 size{ 0 };
        u32 color{ 0 };
        f32 blur{ 0 };
        f32 spacing{ 0 };
    };

    struct AtlasNode
    {
        i32 x{ 0 };
        i32 y{ 0 };
        i32 width{ 0 };
    };

    struct Atlas
    {
        i32 width{ 0 };
        i32 height{ 0 };
        AtlasNode* nodes{ nullptr };
        i32 nnodes{ 0 };
        i32 cnodes{ 0 };
    };

    struct Params
    {
        i32 width;
        i32 height;
        u8 flags;
        void* user_ptr;
        i32 (*render_create)(void* uptr, i32 width, i32 height);
        i32 (*render_resize)(void* uptr, i32 width, i32 height);
        void (*render_update)(void* uptr, i32* rect, const u8* data);
        void (*render_draw)(void* uptr, const f32* verts, const f32* tcoords, const u32* colors,
                            i32 nverts);
        void (*render_delete)(void* uptr);
    };

    enum class ErrorCode {
        // Font atlas is full.
        FonsAtlasFull = 1,
        // Scratch memory used to render glyphs is full,
        // requested size reported in 'val', you may
        // need to bump up FONS_SCRATCH_BUF_SIZE.
        FonsScratchFull = 2,
        // Calls to fonsPushState has created too large stack,
        // if you need deep state stack bump up FONS_MAX_STATES.
        FonsStatesOverflow = 3,
        // Trying to pop too many states fonsPopState().
        FonsStatesUnderflow = 4,
    };

    struct Context
    {
        Params params{};
        f32 itw{ 0.0f };
        f32 ith{ 0.0f };
        u8* tex_data{ nullptr };
        i32 dirty_rect[4]{};
        Font** fonts{ nullptr };
        Atlas* atlas{ nullptr };
        i32 cfonts{ 0 };
        i32 nfonts{ 0 };
        f32 verts[VERTEX_COUNT * 2]{};
        f32 tcoords[VERTEX_COUNT * 2]{};
        u32 colors[VERTEX_COUNT]{};
        i32 nverts{ 0 };
        u8* scratch{ nullptr };
        i32 nscratch{ 0 };
        State states[MAX_STATES]{};
        i32 nstates{ 0 };
        void (*handle_error)(void* uptr, ErrorCode error, i32 val);
        void* error_uptr{ nullptr };
    };

    enum FontFlags {
        FonsZeroTopleft = 1,
        FonsZeroBottomleft = 2,
    };

    enum GlyphBitmap {
        FonsGlyphBitmapOptional = 1,
        FonsGlyphBitmapRequired = 2,
    };

    struct FontQuad
    {
        f32 x0{ 0.0f };
        f32 y0{ 0.0f };
        f32 s0{ 0.0f };
        f32 t0{ 0.0f };
        f32 x1{ 0.0f };
        f32 y1{ 0.0f };
        f32 s1{ 0.0f };
        f32 t1{ 0.0f };
    };

    struct TextIter
    {
        f32 x{ 0.0f };
        f32 y{ 0.0f };
        f32 nextx{ 0.0f };
        f32 nexty{ 0.0f };
        f32 scale{ 0.0f };
        f32 spacing{ 0.0f };
        u32 codepoint{ 0 };
        i16 isize{ 0 };
        i16 iblur{ 0 };
        Font* font{ nullptr };
        i32 prev_glyph_index{ 0 };
        const char* str{ nullptr };
        const char* next{ nullptr };
        const char* end{ nullptr };
        u32 utf8_state{ 0 };
        i32 bitmap_option{ 0 };
    };

    // Constructor and destructor.
    Context* create_internal(const Params* params);
    void delete_internal(Context* font_ctx);

    void set_error_callback(Context* font_ctx,
                            void (*callback)(void* uptr, ErrorCode error, i32 val), void* uptr);

    // Returns current atlas size.
    void get_atlas_size(const Context* font_ctx, i32* width, i32* height);

    // Expands the atlas size.
    i32 expand_atlas(Context* font_ctx, i32 width, i32 height);

    // Resets the whole stash.
    i32 reset_atlas(Context* font_ctx, i32 width, i32 height);

    // Add fonts
    i32 add_font(Context* font_ctx, const char* name, const char* path, i32 font_index);
    i32 add_font_mem(Context* font_ctx, const char* name, u8* data, i32 data_size, i32 free_data,
                     i32 font_index);
    i32 get_font_by_name(const Context* font_ctx, const char* name);

    // State handling
    void push_state(Context* font_ctx);
    void pop_state(Context* font_ctx);
    void clear_state(Context* font_ctx);

    // State setting
    void set_size(Context* font_ctx, f32 size);
    void set_color(Context* font_ctx, u32 color);
    void set_spacing(Context* font_ctx, f32 spacing);
    void set_blur(Context* font_ctx, f32 blur);
    void set_align(Context* font_ctx, Align align);
    void set_font(Context* font_ctx, i32 font);

    // Draw text
    f32 draw_text(Context* font_ctx, f32 x, f32 y, const char* string, const char* end);

    // Measure text
    f32 text_bounds(Context* font_ctx, ds::point<f32> pos, const char* str,
                    const char* end = nullptr);
    f32 text_bounds(Context* font_ctx, ds::point<f32> pos, const char* str, const char* end,
                    ds::rect<f32>& bounds);
    void line_bounds(Context* font_ctx, f32 y, f32* miny, f32* maxy);
    void vert_metrics(Context* font_ctx, f32* ascender, f32* descender, f32* lineh);

    // Text iterator
    i32 text_iter_init(Context* font_ctx, TextIter* iter, ds::point<f32> pos,
                       const char* str, const char* end, i32 bitmap_option);
    i32 text_iter_next(Context* font_ctx, TextIter* iter, FontQuad* quad);

    // Pull texture changes
    const u8* get_texture_data(const Context* font_ctx, i32* width, i32* height);
    i32 validate_texture(Context* font_ctx, i32* dirty);

    // Draws the stash texture for debugging
    void draw_debug(Context* font_ctx, f32 x, f32 y);

    i32 add_fallback_font(const Context* font_ctx, i32 base, i32 fallback);
    void reset_fallback_font(const Context* font_ctx, i32 base);
}
