#pragma once

#include <math.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FONS_INVALID -1

static void* fons__tmpalloc(size_t size, void* up);
static void fons__tmpfree(void* ptr, void* up);
#define STBTT_malloc(x, u) fons__tmpalloc(x, u)
#define STBTT_free(x, u)   fons__tmpfree(x, u)
#include "graphics/stb/stb_truetype.hpp"

#ifndef FONS_SCRATCH_BUF_SIZE
  #define FONS_SCRATCH_BUF_SIZE 96000
#endif
#ifndef FONS_HASH_LUT_SIZE
  #define FONS_HASH_LUT_SIZE 256
#endif
#ifndef FONS_INIT_FONTS
  #define FONS_INIT_FONTS 4
#endif
#ifndef FONS_INIT_GLYPHS
  #define FONS_INIT_GLYPHS 256
#endif
#ifndef FONS_INIT_ATLAS_NODES
  #define FONS_INIT_ATLAS_NODES 256
#endif
#ifndef FONS_VERTEX_COUNT
  #define FONS_VERTEX_COUNT 1024
#endif
#ifndef FONS_MAX_STATES
  #define FONS_MAX_STATES 20
#endif
#ifndef FONS_MAX_FALLBACKS
  #define FONS_MAX_FALLBACKS 20
#endif

namespace rl::vg {
    using namespace stb;

    struct FONSttFontImpl
    {
        stbtt_fontinfo font;
    };

    struct FONSglyph
    {
        unsigned int codepoint;
        int index;
        int next;
        short size, blur;
        short x0, y0, x1, y1;
        short xadv, xoff, yoff;
    };

    struct FONSfont
    {
        FONSttFontImpl font;
        char name[64];
        unsigned char* data;
        int dataSize;
        unsigned char freeData;
        float ascender;
        float descender;
        float lineh;
        FONSglyph* glyphs;
        int cglyphs;
        int nglyphs;
        int lut[FONS_HASH_LUT_SIZE];
        int fallbacks[FONS_MAX_FALLBACKS];
        int nfallbacks;
    };

    struct FONSstate
    {
        int font;
        int align;
        float size;
        unsigned int color;
        float blur;
        float spacing;
    };

    struct FONSatlasNode
    {
        short x, y, width;
    };

    struct FONSatlas
    {
        int width, height;
        FONSatlasNode* nodes;
        int nnodes;
        int cnodes;
    };

    struct FONSparams
    {
        int width, height;
        unsigned char flags;
        void* userPtr;
        int (*renderCreate)(void* uptr, int width, int height);
        int (*renderResize)(void* uptr, int width, int height);
        void (*renderUpdate)(void* uptr, int* rect, const unsigned char* data);
        void (*renderDraw)(void* uptr, const float* verts, const float* tcoords,
                           const unsigned int* colors, int nverts);
        void (*renderDelete)(void* uptr);
    };

    struct FONScontext
    {
        FONSparams params;
        float itw, ith;
        unsigned char* texData;
        int dirtyRect[4];
        FONSfont** fonts;
        FONSatlas* atlas;
        int cfonts;
        int nfonts;
        float verts[FONS_VERTEX_COUNT * 2];
        float tcoords[FONS_VERTEX_COUNT * 2];
        unsigned int colors[FONS_VERTEX_COUNT];
        int nverts;
        unsigned char* scratch;
        int nscratch;
        FONSstate states[FONS_MAX_STATES];
        int nstates;
        void (*handleError)(void* uptr, int error, int val);
        void* errorUptr;
    };

    enum FONSflags {
        FONS_ZERO_TOPLEFT = 1,
        FONS_ZERO_BOTTOMLEFT = 2,
    };

    enum FONSalign {
        // Horizontal align
        FONS_ALIGN_LEFT = 1 << 0,  // Default
        FONS_ALIGN_CENTER = 1 << 1,
        FONS_ALIGN_RIGHT = 1 << 2,
        // Vertical align
        FONS_ALIGN_TOP = 1 << 3,
        FONS_ALIGN_MIDDLE = 1 << 4,
        FONS_ALIGN_BOTTOM = 1 << 5,
        FONS_ALIGN_BASELINE = 1 << 6,  // Default
    };

    enum FONSglyphBitmap {
        FONS_GLYPH_BITMAP_OPTIONAL = 1,
        FONS_GLYPH_BITMAP_REQUIRED = 2,
    };

    enum FONSerrorCode {
        // Font atlas is full.
        FONS_ATLAS_FULL = 1,
        // Scratch memory used to render glyphs is full, requested size reported in 'val', you may
        // need to bump up FONS_SCRATCH_BUF_SIZE.
        FONS_SCRATCH_FULL = 2,
        // Calls to fonsPushState has created too large stack, if you need deep state stack bump up
        // FONS_MAX_STATES.
        FONS_STATES_OVERFLOW = 3,
        // Trying to pop too many states fonsPopState().
        FONS_STATES_UNDERFLOW = 4,
    };

    struct FONSquad
    {
        float x0, y0, s0, t0;
        float x1, y1, s1, t1;
    };

    struct FONStextIter
    {
        float x, y, nextx, nexty, scale, spacing;
        unsigned int codepoint;
        short isize, iblur;
        struct FONSfont* font;
        int prevGlyphIndex;
        const char* str;
        const char* next;
        const char* end;
        unsigned int utf8state;
        int bitmapOption;
    };

    // Constructor and destructor.
    FONScontext* fonsCreateInternal(FONSparams* params);
    void fonsDeleteInternal(FONScontext* s);

    void fonsSetErrorCallback(FONScontext* s, void (*callback)(void* uptr, int error, int val),
                              void* uptr);
    // Returns current atlas size.
    void fonsGetAtlasSize(FONScontext* s, int* width, int* height);
    // Expands the atlas size.
    int fonsExpandAtlas(FONScontext* s, int width, int height);
    // Resets the whole stash.
    int fonsResetAtlas(FONScontext* stash, int width, int height);

    // Add fonts
    int fonsAddFont(FONScontext* s, const char* name, const char* path, int fontIndex);
    int fonsAddFontMem(FONScontext* s, const char* name, unsigned char* data, int ndata,
                       int freeData, int fontIndex);
    int fonsGetFontByName(FONScontext* s, const char* name);

    // State handling
    void fonsPushState(FONScontext* s);
    void fonsPopState(FONScontext* s);
    void fonsClearState(FONScontext* s);

    // State setting
    void fonsSetSize(FONScontext* s, float size);
    void fonsSetColor(FONScontext* s, unsigned int color);
    void fonsSetSpacing(FONScontext* s, float spacing);
    void fonsSetBlur(FONScontext* s, float blur);
    void fonsSetAlign(FONScontext* s, int align);
    void fonsSetFont(FONScontext* s, int font);

    // Draw text
    float fonsDrawText(FONScontext* s, float x, float y, const char* string, const char* end);

    // Measure text
    float fonsTextBounds(FONScontext* s, float x, float y, const char* string, const char* end,
                         float* bounds);
    void fonsLineBounds(FONScontext* s, float y, float* miny, float* maxy);
    void fonsVertMetrics(FONScontext* s, float* ascender, float* descender, float* lineh);

    // Text iterator
    int fonsTextIterInit(FONScontext* stash, FONStextIter* iter, float x, float y, const char* str,
                         const char* end, int bitmapOption);
    int fonsTextIterNext(FONScontext* stash, FONStextIter* iter, struct FONSquad* quad);

    // Pull texture changes
    const unsigned char* fonsGetTextureData(FONScontext* stash, int* width, int* height);
    int fonsValidateTexture(FONScontext* s, int* dirty);

    // Draws the stash texture for debugging
    void fonsDrawDebug(FONScontext* s, float x, float y);

    int fonsAddFallbackFont(FONScontext* stash, int base, int fallback);

    void fonsResetFallbackFont(FONScontext* stash, int base);
}
