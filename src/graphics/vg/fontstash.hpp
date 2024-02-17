#pragma once

#include <cmath>

#include "graphics/stb/stb_truetype.hpp"

constexpr int32_t FONS_INVALID{ -1 };
constexpr int32_t FONS_SCRATCH_BUF_SIZE{ 96000 };
constexpr int32_t FONS_HASH_LUT_SIZE{ 256 };
constexpr int32_t FONS_INIT_FONTS{ 4 };
constexpr int32_t FONS_INIT_GLYPHS{ 256 };
constexpr int32_t FONS_INIT_ATLAS_NODES{ 256 };
constexpr int32_t FONS_VERTEX_COUNT{ 1024 };
constexpr int32_t FONS_MAX_STATES{ 20 };
constexpr int32_t FONS_MAX_FALLBACKS{ 20 };

namespace rl::nvg {
    struct FONSttFontImpl
    {
        stb::stbtt_fontinfo font;
    };

    struct FONSglyph
    {
        uint32_t codepoint;
        int32_t index;
        int32_t next;
        int16_t size;
        int16_t blur;
        int16_t x0;
        int16_t y0;
        int16_t x1;
        int16_t y1;
        int16_t xadv;
        int16_t xoff;
        int16_t yoff;
    };

    struct FONSfont
    {
        FONSttFontImpl font;
        char name[64];
        uint8_t* data;
        int32_t data_size;
        bool free_data;
        float ascender;
        float descender;
        float lineh;
        FONSglyph* glyphs;
        int32_t cglyphs;
        int32_t nglyphs;
        int32_t lut[FONS_HASH_LUT_SIZE];
        int32_t fallbacks[FONS_MAX_FALLBACKS];
        int32_t nfallbacks;
    };

    struct FONSstate
    {
        int32_t font;
        int32_t align;
        float size;
        uint32_t color;
        float blur;
        float spacing;
    };

    struct FONSatlasNode
    {
        int16_t x;
        int16_t y;
        int16_t width;
    };

    struct FONSatlas
    {
        int32_t width;
        int32_t height;
        FONSatlasNode* nodes;
        int32_t nnodes;
        int32_t cnodes;
    };

    struct FONSparams
    {
        int32_t width;
        int32_t height;
        uint8_t flags;
        void* user_ptr;
        int32_t (*render_create)(void* uptr, int32_t width, int32_t height);
        int32_t (*render_resize)(void* uptr, int32_t width, int32_t height);
        void (*render_update)(void* uptr, int32_t* rect, const uint8_t* data);
        void (*render_draw)(void* uptr, const float* verts, const float* tcoords,
                            const uint32_t* colors, int32_t nverts);
        void (*render_delete)(void* uptr);
    };

    struct FONScontext
    {
        FONSparams params;
        float itw;
        float ith;
        uint8_t* tex_data;
        int32_t dirty_rect[4];
        FONSfont** fonts;
        FONSatlas* atlas;
        int32_t cfonts;
        int32_t nfonts;
        float verts[FONS_VERTEX_COUNT * 2];
        float tcoords[FONS_VERTEX_COUNT * 2];
        uint32_t colors[FONS_VERTEX_COUNT];
        int32_t nverts;
        uint8_t* scratch;
        int32_t nscratch;
        FONSstate states[FONS_MAX_STATES];
        int32_t nstates;
        void (*handle_error)(void* uptr, int32_t error, int32_t val);
        void* error_uptr;
    };

    enum FONSflags {
        FonsZeroTopleft = 1,
        FonsZeroBottomleft = 2,
    };

    enum FONSalign {
        // Horizontal align
        FonsAlignLeft = 1 << 0,  // Default
        FonsAlignCenter = 1 << 1,
        FonsAlignRight = 1 << 2,
        // Vertical align
        FonsAlignTop = 1 << 3,
        FonsAlignMiddle = 1 << 4,
        FonsAlignBottom = 1 << 5,
        FonsAlignBaseline = 1 << 6,  // Default
    };

    enum FONSglyphBitmap {
        FonsGlyphBitmapOptional = 1,
        FonsGlyphBitmapRequired = 2,
    };

    enum FONSerrorCode {
        // Font atlas is full.
        FonsAtlasFull = 1,
        // Scratch memory used to render glyphs is full, requested size reported in 'val', you may
        // need to bump up FONS_SCRATCH_BUF_SIZE.
        FonsScratchFull = 2,
        // Calls to fonsPushState has created too large stack, if you need deep state stack bump up
        // FONS_MAX_STATES.
        FonsStatesOverflow = 3,
        // Trying to pop too many states fonsPopState().
        FonsStatesUnderflow = 4,
    };

    struct FONSquad
    {
        float x0, y0, s0, t0;
        float x1, y1, s1, t1;
    };

    struct FONStextIter
    {
        float x{};
        float y{};
        float nextx{};
        float nexty{};
        float scale{};
        float spacing{};
        uint32_t codepoint{};
        int16_t isize{};
        int16_t iblur{};
        FONSfont* font;
        int32_t prev_glyph_index;
        const char* str;
        const char* next;
        const char* end;
        uint32_t utf8_state;
        int32_t bitmap_option;
    };

    // Constructor and destructor.
    FONScontext* fons_create_internal(const FONSparams* params);
    void fons_delete_internal(FONScontext* stash);

    void fons_set_error_callback(
        FONScontext* s, void (*callback)(void* uptr, int32_t error, int32_t val), void* uptr);

    // Returns current atlas size.
    void fons_get_atlas_size(const FONScontext* s, int32_t* width, int32_t* height);

    // Expands the atlas size.
    int32_t fons_expand_atlas(FONScontext* s, int32_t width, int32_t height);

    // Resets the whole stash.
    int32_t fons_reset_atlas(FONScontext* stash, int32_t width, int32_t height);

    // Add fonts
    int32_t fons_add_font(FONScontext* stash, const char* name, const char* path,
                          int32_t font_index);
    int32_t fons_add_font_mem(FONScontext* s, const char* name, uint8_t* data, int32_t data_size,
                              int32_t free_data, int32_t font_index);
    int32_t fons_get_font_by_name(const FONScontext* s, const char* name);

    // State handling
    void fons_push_state(FONScontext* s);
    void fons_pop_state(FONScontext* stash);
    void fons_clear_state(FONScontext* stash);

    // State setting
    void fons_set_size(FONScontext* stash, float size);
    void fons_set_color(FONScontext* stash, uint32_t color);
    void fons_set_spacing(FONScontext* stash, float spacing);
    void fons_set_blur(FONScontext* stash, float blur);
    void fons_set_align(FONScontext* stash, int32_t align);
    void fons_set_font(FONScontext* stash, int32_t font);

    // Draw text
    float fons_draw_text(FONScontext* s, float x, float y, const char* string, const char* end);

    // Measure text
    float fons_text_bounds(FONScontext* stash, float x, float y, const char* str, const char* end,
                           float* bounds);
    void fons_line_bounds(FONScontext* s, float y, float* miny, float* maxy);
    void fons_vert_metrics(FONScontext* s, float* ascender, float* descender, float* lineh);

    // Text iterator
    int32_t fons_text_iter_init(FONScontext* stash, FONStextIter* iter, float x, float y,
                                const char* str, const char* end, int32_t bitmap_option);
    int32_t fons_text_iter_next(FONScontext* stash, FONStextIter* iter, FONSquad* quad);

    // Pull texture changes
    const uint8_t* fons_get_texture_data(const FONScontext* stash, int32_t* width, int32_t* height);
    int32_t fons_validate_texture(FONScontext* s, int32_t* dirty);

    // Draws the stash texture for debugging
    void fons_draw_debug(FONScontext* s, float x, float y);

    int32_t fons_add_fallback_font(const FONScontext* stash, int32_t base, int32_t fallback);
    void fons_reset_fallback_font(const FONScontext* stash, int32_t base);
}
