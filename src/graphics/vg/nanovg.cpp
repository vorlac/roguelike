#include <array>
#include <cstdio>
#include <cstdlib>
#include <numbers>
#include <print>
#include <utility>

#include "graphics/stb/stb_image.hpp"
#include "graphics/vg/fontstash.hpp"
#include "graphics/vg/nanovg.hpp"

#ifdef _MSC_VER
  #pragma warning(disable : 4100)  // unreferenced formal parameter
  #pragma warning(disable : 4127)  // conditional expression is constant
  #pragma warning(disable : 4204)  // nonstandard extension used : non-constant aggregate
  // initializer
  #pragma warning(disable : 4706)  // assignment within conditional expression
  #pragma warning(disable : 4244)
  #pragma warning(disable : 4267)
#endif

enum {
    NVG_INIT_FONTIMAGE_SIZE = 512,
    NVG_MAX_FONTIMAGE_SIZE = 2048,
    NVG_MAX_FONTIMAGES = 4
};

enum {
    NVG_INIT_COMMANDS_SIZE = 256,
    NVG_INIT_POINTS_SIZE = 128,
    NVG_INIT_PATHS_SIZE = 16,
    NVG_INIT_VERTS_SIZE = 256
};

#ifndef NVG_MAX_STATES
  #define NVG_MAX_STATES 64
#endif

#define NVG_KAPPA90 \
    0.5522847493f  // Length proportional to radius of a cubic bezier handle for 90deg arcs.

#define NVG_COUNTOF(arr) (sizeof(arr) / sizeof(0 [arr]))

namespace rl::nvg {
    enum NVGcommands {
        MoveTo = 0,
        LineTo = 1,
        Bezierto = 2,
        Close = 3,
        Winding = 4,
    };

    enum NVGpointFlags {
        NVG_PT_CORNER = 0x01,
        NVG_PT_LEFT = 0x02,
        NVG_PT_BEVEL = 0x04,
        NVG_PR_INNERBEVEL = 0x08,
    };

    struct NVGstate
    {
        NVGcompositeOperationState composite_operation;
        int shapeAntiAlias;
        NVGpaint fill;
        NVGpaint stroke;
        float strokeWidth;
        float miterLimit;
        int lineJoin;
        int lineCap;
        float alpha;
        float xform[6];
        NVGscissor scissor;
        float fontSize;
        float letterSpacing;
        float lineHeight;
        float fontBlur;
        int textAlign;
        int fontId;
    };

    using NVGstate = NVGstate;

    struct NVGpoint
    {
        float x, y;
        float dx, dy;
        float len;
        float dmx, dmy;
        unsigned char flags;
    };

    using NVGpoint = NVGpoint;

    struct NVGpathCache
    {
        NVGpoint* points;
        int npoints;
        int cpoints;
        NVGpath* paths;
        int npaths;
        int cpaths;
        NVGvertex* verts;
        int nverts;
        int cverts;
        float bounds[4];
    };

    using NVGpathCache = NVGpathCache;

    struct NVGcontext
    {
        NVGparams params;
        float* commands;
        int ccommands;
        int ncommands;
        float commandx;
        float commandy;
        NVGstate states[NVG_MAX_STATES];
        int nstates;
        NVGpathCache* cache;
        float tessTol;
        float distTol;
        float fringeWidth;
        float devicePxRatio;
        FONScontext* fs;
        int fontImages[NVG_MAX_FONTIMAGES];
        int fontImageIdx;
        int drawCallCount;
        int fillTriCount;
        int strokeTriCount;
        int textTriCount;
    };

    static float nvg_sqrtf(const float a)
    {
        return sqrtf(a);
    }

    static float nvg_modf(const float a, const float b)
    {
        return fmodf(a, b);
    }

    static float nvg_sinf(const float a)
    {
        return sinf(a);
    }

    static float nvg_cosf(const float a)
    {
        return cosf(a);
    }

    static float nvg_tanf(const float a)
    {
        return tanf(a);
    }

    static float nvg_atan2f(const float a, const float b)
    {
        return atan2f(a, b);
    }

    static float nvg_acosf(const float a)
    {
        return acosf(a);
    }

    static int nvg_mini(const int a, const int b)
    {
        return a < b ? a : b;
    }

    static int nvg_maxi(const int a, const int b)
    {
        return a > b ? a : b;
    }

    static int nvg_clampi(const int a, const int mn, const int mx)
    {
        return a < mn ? mn : (a > mx ? mx : a);
    }

    static float nvg_minf(const float a, const float b)
    {
        return a < b ? a : b;
    }

    static float nvg_maxf(const float a, const float b)
    {
        return a > b ? a : b;
    }

    static float nvg_absf(const float a)
    {
        return a >= 0.0f ? a : -a;
    }

    static float nvg_signf(const float a)
    {
        return a >= 0.0f ? 1.0f : -1.0f;
    }

    static float nvg_clampf(const float a, const float mn, const float mx)
    {
        return a < mn ? mn : (a > mx ? mx : a);
    }

    static float nvg_cross(const float dx0, const float dy0, const float dx1, const float dy1)
    {
        return dx1 * dy0 - dx0 * dy1;
    }

    static float nvg_normalize(float* x, float* y)
    {
        const float d = nvg_sqrtf((*x) * (*x) + (*y) * (*y));
        if (d > 1e-6f)
        {
            const float id = 1.0f / d;
            *x *= id;
            *y *= id;
        }
        return d;
    }

    static void nvg_deletePathCache(NVGpathCache* c)
    {
        if (c == nullptr)
            return;
        if (c->points != nullptr)
            free(c->points);
        if (c->paths != nullptr)
            free(c->paths);
        if (c->verts != nullptr)
            free(c->verts);
        free(c);
    }

    static NVGpathCache* nvg_allocPathCache(void)
    {
        const auto c = static_cast<NVGpathCache*>(malloc(sizeof(NVGpathCache)));
        if (c == nullptr)
            goto error;
        memset(c, 0, sizeof(NVGpathCache));

        c->points = static_cast<NVGpoint*>(malloc(sizeof(NVGpoint) * NVG_INIT_POINTS_SIZE));
        if (!c->points)
            goto error;
        c->npoints = 0;
        c->cpoints = NVG_INIT_POINTS_SIZE;

        c->paths = static_cast<NVGpath*>(malloc(sizeof(NVGpath) * NVG_INIT_PATHS_SIZE));
        if (!c->paths)
            goto error;
        c->npaths = 0;
        c->cpaths = NVG_INIT_PATHS_SIZE;

        c->verts = static_cast<NVGvertex*>(malloc(sizeof(NVGvertex) * NVG_INIT_VERTS_SIZE));
        if (!c->verts)
            goto error;
        c->nverts = 0;
        c->cverts = NVG_INIT_VERTS_SIZE;

        return c;
    error:
        nvg_deletePathCache(c);
        return nullptr;
    }

    static void nvg_setDevicePixelRatio(NVGcontext* ctx, const float ratio)
    {
        ctx->tessTol = 0.25f / ratio;
        ctx->distTol = 0.01f / ratio;
        ctx->fringeWidth = 1.0f / ratio;
        ctx->devicePxRatio = ratio;
    }

    static NVGcompositeOperationState nvg_compositeOperationState(const int op)
    {
        int sfactor, dfactor;

        if (op == NVG_SOURCE_OVER)
        {
            sfactor = NVG_ONE;
            dfactor = NVG_ONE_MINUS_SRC_ALPHA;
        }
        else if (op == NVG_SOURCE_IN)
        {
            sfactor = NVG_DST_ALPHA;
            dfactor = NVG_ZERO;
        }
        else if (op == NVG_SOURCE_OUT)
        {
            sfactor = NVG_ONE_MINUS_DST_ALPHA;
            dfactor = NVG_ZERO;
        }
        else if (op == NVG_ATOP)
        {
            sfactor = NVG_DST_ALPHA;
            dfactor = NVG_ONE_MINUS_SRC_ALPHA;
        }
        else if (op == NVG_DESTINATION_OVER)
        {
            sfactor = NVG_ONE_MINUS_DST_ALPHA;
            dfactor = NVG_ONE;
        }
        else if (op == NVG_DESTINATION_IN)
        {
            sfactor = NVG_ZERO;
            dfactor = NVG_SRC_ALPHA;
        }
        else if (op == NVG_DESTINATION_OUT)
        {
            sfactor = NVG_ZERO;
            dfactor = NVG_ONE_MINUS_SRC_ALPHA;
        }
        else if (op == NVG_DESTINATION_ATOP)
        {
            sfactor = NVG_ONE_MINUS_DST_ALPHA;
            dfactor = NVG_SRC_ALPHA;
        }
        else if (op == NVG_LIGHTER)
        {
            sfactor = NVG_ONE;
            dfactor = NVG_ONE;
        }
        else if (op == NVG_COPY)
        {
            sfactor = NVG_ONE;
            dfactor = NVG_ZERO;
        }
        else if (op == NVG_XOR)
        {
            sfactor = NVG_ONE_MINUS_DST_ALPHA;
            dfactor = NVG_ONE_MINUS_SRC_ALPHA;
        }
        else
        {
            sfactor = NVG_ONE;
            dfactor = NVG_ZERO;
        }

        NVGcompositeOperationState state{};
        state.srcRGB = sfactor;
        state.dstRGB = dfactor;
        state.srcAlpha = sfactor;
        state.dstAlpha = dfactor;
        return state;
    }

    static NVGstate* nvg_getState(NVGcontext* ctx)
    {
        return &ctx->states[ctx->nstates - 1];
    }

    NVGcontext* nvg::CreateInternal(const NVGparams* params)
    {
        FONSparams font_params;
        const auto ctx = static_cast<NVGcontext*>(malloc(sizeof(NVGcontext)));
        if (ctx == nullptr)
            goto error;
        memset(ctx, 0, sizeof(NVGcontext));

        ctx->params = *params;
        for (int& fontImage : ctx->fontImages)
            fontImage = 0;

        ctx->commands = static_cast<float*>(malloc(sizeof(float) * NVG_INIT_COMMANDS_SIZE));
        if (!ctx->commands)
            goto error;
        ctx->ncommands = 0;
        ctx->ccommands = NVG_INIT_COMMANDS_SIZE;

        ctx->cache = nvg_allocPathCache();
        if (ctx->cache == nullptr)
            goto error;

        save(ctx);
        reset(ctx);

        nvg_setDevicePixelRatio(ctx, 1.0f);

        if (ctx->params.renderCreate(ctx->params.userPtr) == 0)
            goto error;

        // Init font rendering
        memset(&font_params, 0, sizeof(font_params));
        font_params.width = NVG_INIT_FONTIMAGE_SIZE;
        font_params.height = NVG_INIT_FONTIMAGE_SIZE;
        font_params.flags = FONS_ZERO_TOPLEFT;
        font_params.renderCreate = nullptr;
        font_params.renderUpdate = nullptr;
        font_params.renderDraw = nullptr;
        font_params.renderDelete = nullptr;
        font_params.userPtr = nullptr;
        ctx->fs = fonsCreateInternal(&font_params);
        if (ctx->fs == nullptr)
            goto error;

        // Create font texture
        ctx->fontImages[0] = ctx->params.renderCreateTexture(ctx->params.userPtr, NVG_TEXTURE_ALPHA,
                                                             font_params.width, font_params.height,
                                                             0, nullptr);
        if (ctx->fontImages[0] == 0)
            goto error;
        ctx->fontImageIdx = 0;

        return ctx;

    error:
        delete_internal(ctx);
        return nullptr;
    }

    NVGparams* internal_params(NVGcontext* ctx)
    {
        return &ctx->params;
    }

    void delete_internal(NVGcontext* ctx)
    {
        if (ctx == nullptr)
            return;
        if (ctx->commands != nullptr)
            free(ctx->commands);
        if (ctx->cache != nullptr)
            nvg_deletePathCache(ctx->cache);

        if (ctx->fs)
            fonsDeleteInternal(ctx->fs);

        for (int i = 0; i < NVG_MAX_FONTIMAGES; i++)
            if (ctx->fontImages[i] != 0)
            {
                delete_image(ctx, ctx->fontImages[i]);
                ctx->fontImages[i] = 0;
            }

        if (ctx->params.renderDelete != nullptr)
            ctx->params.renderDelete(ctx->params.userPtr);

        free(ctx);
    }

    void begin_frame(NVGcontext* ctx, const float window_width, const float window_height,
                     const float device_pixel_ratio)
    {
        /*	printf("Tris: draws:%d  fill:%d  stroke:%d  text:%d  TOT:%d\n",
                ctx->drawCallCount, ctx->fillTriCount, ctx->strokeTriCount, ctx->textTriCount,
                ctx->fillTriCount+ctx->strokeTriCount+ctx->textTriCount);*/

        ctx->nstates = 0;
        save(ctx);
        reset(ctx);

        nvg_setDevicePixelRatio(ctx, device_pixel_ratio);

        ctx->params.renderViewport(ctx->params.userPtr, window_width, window_height,
                                   device_pixel_ratio);

        ctx->drawCallCount = 0;
        ctx->fillTriCount = 0;
        ctx->strokeTriCount = 0;
        ctx->textTriCount = 0;
    }

    void cancel_frame(const NVGcontext* ctx)
    {
        ctx->params.renderCancel(ctx->params.userPtr);
    }

    void end_frame(NVGcontext* ctx)
    {
        ctx->params.renderFlush(ctx->params.userPtr);
        if (ctx->fontImageIdx != 0)
        {
            const int fontImage = ctx->fontImages[ctx->fontImageIdx];
            ctx->fontImages[ctx->fontImageIdx] = 0;
            int j, iw, ih;
            // delete images that smaller than current one
            if (fontImage == 0)
                return;
            image_size(ctx, fontImage, &iw, &ih);
            for (int i = j = 0; i < ctx->fontImageIdx; i++)
                if (ctx->fontImages[i] != 0)
                {
                    int nw, nh;
                    const int image = ctx->fontImages[i];
                    ctx->fontImages[i] = 0;
                    image_size(ctx, image, &nw, &nh);
                    if (nw < iw || nh < ih)
                        delete_image(ctx, image);
                    else
                        ctx->fontImages[j++] = image;
                }
            // make current font image to first
            ctx->fontImages[j] = ctx->fontImages[0];
            ctx->fontImages[0] = fontImage;
            ctx->fontImageIdx = 0;
        }
    }

    NVGcolor rgb(const unsigned char r, const unsigned char g, const unsigned char b)
    {
        return RGBA(r, g, b, 255);
    }

    NVGcolor rgb_f(const float r, const float g, const float b)
    {
        return RGBAf(r, g, b, 1.0f);
    }

    NVGcolor RGBA(const unsigned char r, const unsigned char g, const unsigned char b,
                  const unsigned char a)
    {
        NVGcolor color{};
        // Use longer initialization to suppress warning.
        color.r = r / 255.0f;
        color.g = g / 255.0f;
        color.b = b / 255.0f;
        color.a = a / 255.0f;
        return color;
    }

    NVGcolor RGBAf(const float r, const float g, const float b, const float a)
    {
        NVGcolor color{};
        // Use longer initialization to suppress warning.
        color.r = r;
        color.g = g;
        color.b = b;
        color.a = a;
        return color;
    }

    NVGcolor trans_rgba(NVGcolor c, const unsigned char a)
    {
        c.a = a / 255.0f;
        return c;
    }

    NVGcolor trans_rgba_f(NVGcolor c, const float a)
    {
        c.a = a;
        return c;
    }

    NVGcolor lerp_rgba(const NVGcolor c0, const NVGcolor c1, float u)
    {
        NVGcolor cint = { { { 0 } } };

        u = nvg_clampf(u, 0.0f, 1.0f);
        const float oneminu = 1.0f - u;
        for (int i = 0; i < 4; i++)
            cint.rgba[i] = c0.rgba[i] * oneminu + c1.rgba[i] * u;

        return cint;
    }

    NVGcolor hsl(const float h, const float s, const float l)
    {
        return hsla(h, s, l, 255);
    }

    static float nvg_hue(float h, const float m1, const float m2)
    {
        if (h < 0)
            h += 1;
        if (h > 1)
            h -= 1;
        if (h < 1.0f / 6.0f)
            return m1 + (m2 - m1) * h * 6.0f;
        if (h < 3.0f / 6.0f)
            return m2;
        if (h < 4.0f / 6.0f)
            return m1 + (m2 - m1) * (2.0f / 3.0f - h) * 6.0f;
        return m1;
    }

    NVGcolor hsla(float h, float s, float l, const unsigned char a)
    {
        NVGcolor col{};
        h = nvg_modf(h, 1.0f);
        if (h < 0.0f)
            h += 1.0f;
        s = nvg_clampf(s, 0.0f, 1.0f);
        l = nvg_clampf(l, 0.0f, 1.0f);
        const float m2 = l <= 0.5f ? (l * (1 + s)) : (l + s - l * s);
        const float m1 = 2 * l - m2;
        col.r = nvg_clampf(nvg_hue(h + 1.0f / 3.0f, m1, m2), 0.0f, 1.0f);
        col.g = nvg_clampf(nvg_hue(h, m1, m2), 0.0f, 1.0f);
        col.b = nvg_clampf(nvg_hue(h - 1.0f / 3.0f, m1, m2), 0.0f, 1.0f);
        col.a = a / 255.0f;
        return col;
    }

    void transform_identity(float* t)
    {
        t[0] = 1.0f;
        t[1] = 0.0f;
        t[2] = 0.0f;
        t[3] = 1.0f;
        t[4] = 0.0f;
        t[5] = 0.0f;
    }

    void transform_translate(float* t, const float tx, const float ty)
    {
        t[0] = 1.0f;
        t[1] = 0.0f;
        t[2] = 0.0f;
        t[3] = 1.0f;
        t[4] = tx;
        t[5] = ty;
    }

    void transform_translate(float* t, ds::vector2<f32>&& translation)
    {
        t[0] = 1.0f;
        t[1] = 0.0f;
        t[2] = 0.0f;
        t[3] = 1.0f;
        t[4] = translation.x;
        t[5] = translation.y;
    }

    void transform_scale(float* dst, const float sx, const float sy)
    {
        dst[0] = sx;
        dst[1] = 0.0f;
        dst[2] = 0.0f;
        dst[3] = sy;
        dst[4] = 0.0f;
        dst[5] = 0.0f;
    }

    void transform_rotate(float* dst, const float a)
    {
        const float cs = nvg_cosf(a);
        const float sn = nvg_sinf(a);
        dst[0] = cs;
        dst[1] = sn;
        dst[2] = -sn;
        dst[3] = cs;
        dst[4] = 0.0f;
        dst[5] = 0.0f;
    }

    void transform_skew_x(float* dst, const float a)
    {
        dst[0] = 1.0f;
        dst[1] = 0.0f;
        dst[2] = nvg_tanf(a);
        dst[3] = 1.0f;
        dst[4] = 0.0f;
        dst[5] = 0.0f;
    }

    void transform_skew_y(float* dst, const float a)
    {
        dst[0] = 1.0f;
        dst[1] = nvg_tanf(a);
        dst[2] = 0.0f;
        dst[3] = 1.0f;
        dst[4] = 0.0f;
        dst[5] = 0.0f;
    }

    void transform_multiply(float* dst, const float* src)
    {
        const float t0 = dst[0] * src[0] + dst[1] * src[2];
        const float t2 = dst[2] * src[0] + dst[3] * src[2];
        const float t4 = dst[4] * src[0] + dst[5] * src[2] + src[4];
        dst[1] = dst[0] * src[1] + dst[1] * src[3];
        dst[3] = dst[2] * src[1] + dst[3] * src[3];
        dst[5] = dst[4] * src[1] + dst[5] * src[3] + src[5];
        dst[0] = t0;
        dst[2] = t2;
        dst[4] = t4;
    }

    void transform_premultiply(float* dst, const float* src)
    {
        float s2[6];
        memcpy(s2, src, sizeof(float) * 6);
        transform_multiply(s2, dst);
        memcpy(dst, s2, sizeof(float) * 6);
    }

    int transform_inverse(float* inv, const float* t)
    {
        const double det = static_cast<double>(t[0]) * t[3] - static_cast<double>(t[2]) * t[1];
        if (det > -1e-6 && det < 1e-6)
        {
            transform_identity(inv);
            return 0;
        }

        const double invdet = 1.0 / det;
        inv[0] = static_cast<float>(t[3] * invdet);
        inv[2] = static_cast<float>(-t[2] * invdet);
        inv[4] = static_cast<float>(
            (static_cast<double>(t[2]) * t[5] - static_cast<double>(t[3]) * t[4]) * invdet);
        inv[1] = static_cast<float>(-t[1] * invdet);
        inv[3] = static_cast<float>(t[0] * invdet);
        inv[5] = static_cast<float>(
            (static_cast<double>(t[1]) * t[4] - static_cast<double>(t[0]) * t[5]) * invdet);

        return 1;
    }

    void transform_point(float* dstx, float* dsty, const float* xform, const float srcx,
                         const float srcy)
    {
        *dstx = srcx * xform[0] + srcy * xform[2] + xform[4];
        *dsty = srcx * xform[1] + srcy * xform[3] + xform[5];
    }

    float deg_to_rad(const float deg)
    {
        return deg / 180.0f * std::numbers::pi_v<f32>;
    }

    float rad_to_deg(const float rad)
    {
        return rad / std::numbers::pi_v<f32> * 180.0f;
    }

    static void nvg_set_paint_color(NVGpaint* p, const NVGcolor color)
    {
        memset(p, 0, sizeof(*p));
        transform_identity(p->xform);
        p->radius = 0.0f;
        p->feather = 1.0f;
        p->innerColor = color;
        p->outerColor = color;
    }

    // State handling
    void save(NVGcontext* ctx)
    {
        if (ctx->nstates >= NVG_MAX_STATES)
            return;
        if (ctx->nstates > 0)
            memcpy(&ctx->states[ctx->nstates], &ctx->states[ctx->nstates - 1], sizeof(NVGstate));
        ctx->nstates++;
    }

    void restore(NVGcontext* ctx)
    {
        if (ctx->nstates <= 1)
            return;
        ctx->nstates--;
    }

    void reset(NVGcontext* ctx)
    {
        NVGstate* state = nvg_getState(ctx);
        memset(state, 0, sizeof(*state));

        nvg_set_paint_color(&state->fill, RGBA(255, 255, 255, 255));
        nvg_set_paint_color(&state->stroke, RGBA(0, 0, 0, 255));
        state->composite_operation = nvg_compositeOperationState(NVG_SOURCE_OVER);
        state->shapeAntiAlias = 1;
        state->strokeWidth = 1.0f;
        state->miterLimit = 10.0f;
        state->lineCap = NVG_BUTT;
        state->lineJoin = NVG_MITER;
        state->alpha = 1.0f;
        transform_identity(state->xform);

        state->scissor.extent[0] = -1.0f;
        state->scissor.extent[1] = -1.0f;

        state->fontSize = 16.0f;
        state->letterSpacing = 0.0f;
        state->lineHeight = 1.0f;
        state->fontBlur = 0.0f;
        state->textAlign = NVG_ALIGN_LEFT | NVG_ALIGN_BASELINE;
        state->fontId = 0;
    }

    // State setting
    void shape_anti_alias(NVGcontext* ctx, const int enabled)
    {
        NVGstate* state = nvg_getState(ctx);
        state->shapeAntiAlias = enabled;
    }

    void stroke_width(NVGcontext* ctx, const float width)
    {
        NVGstate* state = nvg_getState(ctx);
        state->strokeWidth = width;
    }

    void miter_limit(NVGcontext* ctx, const float limit)
    {
        NVGstate* state = nvg_getState(ctx);
        state->miterLimit = limit;
    }

    void line_cap(NVGcontext* ctx, const int cap)
    {
        NVGstate* state = nvg_getState(ctx);
        state->lineCap = cap;
    }

    void line_join(NVGcontext* ctx, const int join)
    {
        NVGstate* state = nvg_getState(ctx);
        state->lineJoin = join;
    }

    void global_alpha(NVGcontext* ctx, const float alpha)
    {
        NVGstate* state = nvg_getState(ctx);
        state->alpha = alpha;
    }

    void transform(NVGcontext* ctx, const float a, const float b, const float c, const float d,
                   const float e, const float f)
    {
        NVGstate* state = nvg_getState(ctx);
        const float t[6] = { a, b, c, d, e, f };
        transform_premultiply(state->xform, t);
    }

    void reset_transform(NVGcontext* ctx)
    {
        NVGstate* state = nvg_getState(ctx);
        transform_identity(state->xform);
    }

    void translate(NVGcontext* ctx, const float x, const float y)
    {
        NVGstate* state = nvg_getState(ctx);
        float t[6];
        transform_translate(t, x, y);
        transform_premultiply(state->xform, t);
    }

    void translate(NVGcontext* ctx, ds::vector2<f32>&& local_offset)
    {
        float t[6] = { 0 };
        NVGstate* state{ nvg_getState(ctx) };
        transform_translate(t, local_offset.x, local_offset.y);
        transform_premultiply(state->xform, t);
    }

    void rotate(NVGcontext* ctx, const float angle)
    {
        NVGstate* state = nvg_getState(ctx);
        float t[6];
        transform_rotate(t, angle);
        transform_premultiply(state->xform, t);
    }

    void skew_x(NVGcontext* ctx, const float angle)
    {
        NVGstate* state = nvg_getState(ctx);
        float t[6];
        transform_skew_x(t, angle);
        transform_premultiply(state->xform, t);
    }

    void skew_y(NVGcontext* ctx, const float angle)
    {
        NVGstate* state = nvg_getState(ctx);
        float t[6];
        transform_skew_y(t, angle);
        transform_premultiply(state->xform, t);
    }

    void scale(NVGcontext* ctx, const float x, const float y)
    {
        NVGstate* state = nvg_getState(ctx);
        float t[6];
        transform_scale(t, x, y);
        transform_premultiply(state->xform, t);
    }

    void current_transform(NVGcontext* ctx, float* xform)
    {
        const NVGstate* state = nvg_getState(ctx);
        if (xform == nullptr)
            return;
        memcpy(xform, state->xform, sizeof(float) * 6);
    }

    void stroke_color(NVGcontext* ctx, const NVGcolor color)
    {
        NVGstate* state = nvg_getState(ctx);
        nvg_set_paint_color(&state->stroke, color);
    }

    void stroke_paint(NVGcontext* ctx, const NVGpaint& paint)
    {
        NVGstate* state = nvg_getState(ctx);
        state->stroke = paint;
        transform_multiply(state->stroke.xform, state->xform);
    }

    void fill_color(NVGcontext* ctx, const NVGcolor color)
    {
        NVGstate* state = nvg_getState(ctx);
        nvg_set_paint_color(&state->fill, color);
    }

    void fill_paint(NVGcontext* ctx, const NVGpaint& paint)
    {
        NVGstate* state = nvg_getState(ctx);
        state->fill = paint;
        transform_multiply(state->fill.xform, state->xform);
    }

#ifndef NVG_NO_STB
    int create_image(const NVGcontext* ctx, const char* filename, const int imageFlags)
    {
        int w, h, n;
        stbi_set_unpremultiply_on_load(1);
        stbi_convert_iphone_png_to_rgb(1);
        unsigned char* img = stbi_load(filename, &w, &h, &n, 4);
        if (img == nullptr)
            //		printf("Failed to load %s - %s\n", filename, stbi_failure_reason());
            return 0;
        const int image = create_image_rgba(ctx, w, h, imageFlags, img);
        stbi_image_free(img);
        return image;
    }

    int create_image_mem(const NVGcontext* ctx, const int imageFlags, const unsigned char* data,
                         const int ndata)
    {
        int w, h, n;
        unsigned char* img = stbi_load_from_memory(data, ndata, &w, &h, &n, 4);
        if (img == nullptr)
            //		printf("Failed to load %s - %s\n", filename, stbi_failure_reason());
            return 0;
        const int image = create_image_rgba(ctx, w, h, imageFlags, img);
        stbi_image_free(img);
        return image;
    }
#endif

    int create_image_rgba(const NVGcontext* ctx, const int w, const int h, const int imageFlags,
                          const unsigned char* data)
    {
        return ctx->params.renderCreateTexture(ctx->params.userPtr, NVG_TEXTURE_RGBA, w, h,
                                               imageFlags, data);
    }

    int create_image_alpha(const NVGcontext* ctx, const int w, const int h, const int imageFlags,
                           const unsigned char* data)
    {
        return ctx->params.renderCreateTexture(ctx->params.userPtr, NVG_TEXTURE_ALPHA, w, h,
                                               imageFlags, data);
    }

    void update_image(const NVGcontext* ctx, const int image, const unsigned char* data)
    {
        int w, h;
        ctx->params.renderGetTextureSize(ctx->params.userPtr, image, &w, &h);
        ctx->params.renderUpdateTexture(ctx->params.userPtr, image, 0, 0, w, h, data);
    }

    void image_size(const NVGcontext* ctx, const int image, int* w, int* h)
    {
        ctx->params.renderGetTextureSize(ctx->params.userPtr, image, w, h);
    }

    void delete_image(const NVGcontext* ctx, const int image)
    {
        ctx->params.renderDeleteTexture(ctx->params.userPtr, image);
    }

    NVGpaint linear_gradient(NVGcontext* ctx, const float sx, const float sy, const float ex,
                             const float ey, const NVGcolor icol, const NVGcolor ocol)
    {
        NVGpaint p;
        constexpr float large = 1e5;
        NVG_NOTUSED(ctx);
        memset(&p, 0, sizeof(p));

        // Calculate transform aligned to the line
        float dx = ex - sx;
        float dy = ey - sy;
        const float d = sqrtf(dx * dx + dy * dy);
        if (d > 0.0001f)
        {
            dx /= d;
            dy /= d;
        }
        else
        {
            dx = 0;
            dy = 1;
        }

        p.xform[0] = dy;
        p.xform[1] = -dx;
        p.xform[2] = dx;
        p.xform[3] = dy;
        p.xform[4] = sx - dx * large;
        p.xform[5] = sy - dy * large;

        p.extent[0] = large;
        p.extent[1] = large + d * 0.5f;

        p.radius = 0.0f;

        p.feather = nvg_maxf(1.0f, d);

        p.innerColor = icol;
        p.outerColor = ocol;

        return p;
    }

    NVGpaint radial_gradient(NVGcontext* ctx, const float cx, const float cy, const float inr,
                             const float outr, const NVGcolor icol, const NVGcolor ocol)
    {
        NVGpaint p;
        const float r = (inr + outr) * 0.5f;
        const float f = (outr - inr);
        NVG_NOTUSED(ctx);
        memset(&p, 0, sizeof(p));

        transform_identity(p.xform);
        p.xform[4] = cx;
        p.xform[5] = cy;

        p.extent[0] = r;
        p.extent[1] = r;

        p.radius = r;

        p.feather = nvg_maxf(1.0f, f);

        p.innerColor = icol;
        p.outerColor = ocol;

        return p;
    }

    NVGpaint box_gradient(NVGcontext* ctx, const float x, const float y, const float w,
                          const float h, const float r, const float f, const NVGcolor icol,
                          const NVGcolor ocol)
    {
        NVGpaint p;
        NVG_NOTUSED(ctx);
        memset(&p, 0, sizeof(p));

        transform_identity(p.xform);
        p.xform[4] = x + w * 0.5f;
        p.xform[5] = y + h * 0.5f;

        p.extent[0] = w * 0.5f;
        p.extent[1] = h * 0.5f;

        p.radius = r;

        p.feather = nvg_maxf(1.0f, f);

        p.innerColor = icol;
        p.outerColor = ocol;

        return p;
    }

    NVGpaint image_pattern(NVGcontext* ctx, const float cx, const float cy, const float w,
                           const float h, const float angle, const int image, const float alpha)
    {
        NVGpaint p;
        NVG_NOTUSED(ctx);
        memset(&p, 0, sizeof(p));

        transform_rotate(p.xform, angle);
        p.xform[4] = cx;
        p.xform[5] = cy;

        p.extent[0] = w;
        p.extent[1] = h;

        p.image = image;

        p.innerColor = p.outerColor = RGBAf(1, 1, 1, alpha);

        return p;
    }

    // Scissoring
    void scissor(NVGcontext* ctx, const float x, const float y, float w, float h)
    {
        NVGstate* state = nvg_getState(ctx);

        w = nvg_maxf(0.0f, w);
        h = nvg_maxf(0.0f, h);

        transform_identity(state->scissor.xform);
        state->scissor.xform[4] = x + w * 0.5f;
        state->scissor.xform[5] = y + h * 0.5f;
        transform_multiply(state->scissor.xform, state->xform);

        state->scissor.extent[0] = w * 0.5f;
        state->scissor.extent[1] = h * 0.5f;
    }

    void nvg_isectRects(float* dst, const float ax, const float ay, const float aw, const float ah,
                        const float bx, const float by, const float bw, const float bh)
    {
        const float minx = nvg_maxf(ax, bx);
        const float miny = nvg_maxf(ay, by);
        const float maxx = nvg_minf(ax + aw, bx + bw);
        const float maxy = nvg_minf(ay + ah, by + bh);
        dst[0] = minx;
        dst[1] = miny;
        dst[2] = nvg_maxf(0.0f, maxx - minx);
        dst[3] = nvg_maxf(0.0f, maxy - miny);
    }

    void intersect_scissor(NVGcontext* ctx, const float x, const float y, const float w,
                           const float h)
    {
        const NVGstate* state = nvg_getState(ctx);
        float pxform[6], invxorm[6];
        float rect[4];

        // If no previous scissor has been set, set the scissor as current scissor.
        if (state->scissor.extent[0] < 0)
        {
            scissor(ctx, x, y, w, h);
            return;
        }

        // Transform the current scissor rect into current transform space.
        // If there is difference in rotation, this will be approximation.
        memcpy(pxform, state->scissor.xform, sizeof(float) * 6);
        const float ex = state->scissor.extent[0];
        const float ey = state->scissor.extent[1];
        transform_inverse(invxorm, state->xform);
        transform_multiply(pxform, invxorm);
        const float tex = ex * nvg_absf(pxform[0]) + ey * nvg_absf(pxform[2]);
        const float tey = ex * nvg_absf(pxform[1]) + ey * nvg_absf(pxform[3]);

        // Intersect rects.
        nvg_isectRects(rect, pxform[4] - tex, pxform[5] - tey, tex * 2, tey * 2, x, y, w, h);

        scissor(ctx, rect[0], rect[1], rect[2], rect[3]);
    }

    void reset_scissor(NVGcontext* ctx)
    {
        NVGstate* state = nvg_getState(ctx);
        memset(state->scissor.xform, 0, sizeof(state->scissor.xform));
        state->scissor.extent[0] = -1.0f;
        state->scissor.extent[1] = -1.0f;
    }

    // Global composite operation.
    void GlobalCompositeOperation(NVGcontext* ctx, const int op)
    {
        NVGstate* state = nvg_getState(ctx);
        state->composite_operation = nvg_compositeOperationState(op);
    }

    void GlobalCompositeBlendFunc(NVGcontext* ctx, const int sfactor, const int dfactor)
    {
        GlobalCompositeBlendFuncSeparate(ctx, sfactor, dfactor, sfactor, dfactor);
    }

    void GlobalCompositeBlendFuncSeparate(NVGcontext* ctx, const int srcRGB, const int dstRGB,
                                          const int srcAlpha, const int dstAlpha)
    {
        NVGcompositeOperationState op{};
        op.srcRGB = srcRGB;
        op.dstRGB = dstRGB;
        op.srcAlpha = srcAlpha;
        op.dstAlpha = dstAlpha;

        NVGstate* state = nvg_getState(ctx);
        state->composite_operation = op;
    }

    int nvg_ptEquals(const float x1, const float y1, const float x2, const float y2, const float tol)
    {
        const float dx = x2 - x1;
        const float dy = y2 - y1;
        return dx * dx + dy * dy < tol * tol;
    }

    float nvg_distPtSeg(const float x, const float y, const float px, const float py,
                        const float qx, const float qy)
    {
        const float pqx = qx - px;
        const float pqy = qy - py;
        float dx = x - px;
        float dy = y - py;
        const float d = pqx * pqx + pqy * pqy;
        float t = pqx * dx + pqy * dy;
        if (d > 0)
            t /= d;
        if (t < 0)
            t = 0;
        else if (t > 1)
            t = 1;
        dx = px + t * pqx - x;
        dy = py + t * pqy - y;
        return dx * dx + dy * dy;
    }

    void nvg_appendCommands(NVGcontext* ctx, float* vals, const int nvals)
    {
        const NVGstate* state = nvg_getState(ctx);

        if (ctx->ncommands + nvals > ctx->ccommands)
        {
            const int ccommands = ctx->ncommands + nvals + ctx->ccommands / 2;
            const auto commands = static_cast<float*>(
                realloc(ctx->commands, sizeof(float) * ccommands));
            if (commands == nullptr)
                return;
            ctx->commands = commands;
            ctx->ccommands = ccommands;
        }

        const int val = vals[0];
        if (val != static_cast<float>(Close) && val != static_cast<float>(Winding))
        {
            ctx->commandx = vals[nvals - 2];
            ctx->commandy = vals[nvals - 1];
        }

        // transform commands
        int i = 0;
        while (i < nvals)
        {
            const float cmd{ vals[i] };
            switch (NVGcommands((int)cmd))
            {
                case NVGcommands::MoveTo:
                    transform_point(&vals[i + 1], &vals[i + 2], state->xform, vals[i + 1],
                                    vals[i + 2]);
                    i += 3;
                    break;

                case NVGcommands::LineTo:
                    transform_point(&vals[i + 1], &vals[i + 2], state->xform, vals[i + 1],
                                    vals[i + 2]);
                    i += 3;
                    break;

                case NVGcommands::Bezierto:
                    transform_point(&vals[i + 1], &vals[i + 2], state->xform, vals[i + 1],
                                    vals[i + 2]);
                    transform_point(&vals[i + 3], &vals[i + 4], state->xform, vals[i + 3],
                                    vals[i + 4]);
                    transform_point(&vals[i + 5], &vals[i + 6], state->xform, vals[i + 5],
                                    vals[i + 6]);
                    i += 7;
                    break;

                case NVGcommands::Close:
                    i++;
                    break;

                case NVGcommands::Winding:
                    i += 2;
                    break;

                default:
                    i++;
            }
        }

        memcpy(&ctx->commands[ctx->ncommands], vals, nvals * sizeof(float));

        ctx->ncommands += nvals;
    }

    static void nvg_clearPathCache(const NVGcontext* ctx)
    {
        ctx->cache->npoints = 0;
        ctx->cache->npaths = 0;
    }

    static NVGpath* nvg_lastPath(const NVGcontext* ctx)
    {
        if (ctx->cache->npaths > 0)
            return &ctx->cache->paths[ctx->cache->npaths - 1];
        return nullptr;
    }

    static void nvg_addPath(const NVGcontext* ctx)
    {
        if (ctx->cache->npaths + 1 > ctx->cache->cpaths)
        {
            const int cpaths = ctx->cache->npaths + 1 + ctx->cache->cpaths / 2;
            const auto paths = static_cast<NVGpath*>(
                realloc(ctx->cache->paths, sizeof(NVGpath) * cpaths));
            if (paths == nullptr)
                return;
            ctx->cache->paths = paths;
            ctx->cache->cpaths = cpaths;
        }
        NVGpath* path = &ctx->cache->paths[ctx->cache->npaths];
        memset(path, 0, sizeof(*path));
        path->first = ctx->cache->npoints;
        path->winding = NVG_CCW;

        ctx->cache->npaths++;
    }

    static NVGpoint* nvg_lastPoint(const NVGcontext* ctx)
    {
        if (ctx->cache->npoints > 0)
            return &ctx->cache->points[ctx->cache->npoints - 1];
        return nullptr;
    }

    static void nvg_addPoint(const NVGcontext* ctx, const float x, const float y, const int flags)
    {
        NVGpath* path = nvg_lastPath(ctx);
        NVGpoint* pt;
        if (path == nullptr)
            return;

        if (path->count > 0 && ctx->cache->npoints > 0)
        {
            pt = nvg_lastPoint(ctx);
            if (nvg_ptEquals(pt->x, pt->y, x, y, ctx->distTol))
            {
                pt->flags |= flags;
                return;
            }
        }

        if (ctx->cache->npoints + 1 > ctx->cache->cpoints)
        {
            const int cpoints = ctx->cache->npoints + 1 + ctx->cache->cpoints / 2;
            const auto points = static_cast<NVGpoint*>(
                realloc(ctx->cache->points, sizeof(NVGpoint) * cpoints));
            if (points == nullptr)
                return;
            ctx->cache->points = points;
            ctx->cache->cpoints = cpoints;
        }

        pt = &ctx->cache->points[ctx->cache->npoints];
        memset(pt, 0, sizeof(*pt));
        pt->x = x;
        pt->y = y;
        pt->flags = static_cast<unsigned char>(flags);

        ctx->cache->npoints++;
        path->count++;
    }

    static void nvg_closePath(const NVGcontext* ctx)
    {
        NVGpath* path = nvg_lastPath(ctx);
        if (path == nullptr)
            return;
        path->closed = 1;
    }

    static void nvg_pathWinding(const NVGcontext* ctx, const int winding)
    {
        NVGpath* path = nvg_lastPath(ctx);
        if (path == nullptr)
            return;
        path->winding = winding;
    }

    static float nvg_getAverageScale(const float* t)
    {
        const float sx = sqrtf(t[0] * t[0] + t[2] * t[2]);
        const float sy = sqrtf(t[1] * t[1] + t[3] * t[3]);
        return (sx + sy) * 0.5f;
    }

    static NVGvertex* nvg_allocTempVerts(const NVGcontext* ctx, const int nverts)
    {
        if (nverts > ctx->cache->cverts)
        {
            const int cverts = (nverts + 0xff) & ~0xff;  // Round up to prevent allocations when
            // things change just slightly.
            const auto verts = static_cast<NVGvertex*>(
                realloc(ctx->cache->verts, sizeof(NVGvertex) * cverts));
            if (verts == nullptr)
                return nullptr;
            ctx->cache->verts = verts;
            ctx->cache->cverts = cverts;
        }

        return ctx->cache->verts;
    }

    static float nvg_triarea2(const float ax, const float ay, const float bx, const float by,
                              const float cx, const float cy)
    {
        const float abx = bx - ax;
        const float aby = by - ay;
        const float acx = cx - ax;
        const float acy = cy - ay;
        return acx * aby - abx * acy;
    }

    static float nvg_polyArea(NVGpoint* pts, const int npts)
    {
        float area = 0;
        for (int i = 2; i < npts; i++)
        {
            const NVGpoint* a = &pts[0];
            const NVGpoint* b = &pts[i - 1];
            const NVGpoint* c = &pts[i];
            area += nvg_triarea2(a->x, a->y, b->x, b->y, c->x, c->y);
        }
        return area * 0.5f;
    }

    static void nvg_polyReverse(NVGpoint* pts, const int npts)
    {
        int i = 0, j = npts - 1;
        while (i < j)
        {
            const NVGpoint tmp = pts[i];
            pts[i] = pts[j];
            pts[j] = tmp;
            i++;
            j--;
        }
    }

    static void nvg_vset(NVGvertex* vtx, const float x, const float y, const float u, const float v)
    {
        vtx->x = x;
        vtx->y = y;
        vtx->u = u;
        vtx->v = v;
    }

    static void nvg_tesselateBezier(NVGcontext* ctx, const float x1, const float y1, const float x2,
                                    const float y2, const float x3, const float y3, const float x4,
                                    const float y4, const int level, const int type)
    {
        if (level > 10)
            return;

        const float x12 = (x1 + x2) * 0.5f;
        const float y12 = (y1 + y2) * 0.5f;
        const float x23 = (x2 + x3) * 0.5f;
        const float y23 = (y2 + y3) * 0.5f;
        const float x34 = (x3 + x4) * 0.5f;
        const float y34 = (y3 + y4) * 0.5f;
        const float x123 = (x12 + x23) * 0.5f;
        const float y123 = (y12 + y23) * 0.5f;

        const float dx = x4 - x1;
        const float dy = y4 - y1;
        const float d2 = nvg_absf((x2 - x4) * dy - (y2 - y4) * dx);
        const float d3 = nvg_absf((x3 - x4) * dy - (y3 - y4) * dx);

        if ((d2 + d3) * (d2 + d3) < ctx->tessTol * (dx * dx + dy * dy))
        {
            nvg_addPoint(ctx, x4, y4, type);
            return;
        }

        /*	if (nvg__absf(x1+x3-x2-x2) + nvg__absf(y1+y3-y2-y2) + nvg__absf(x2+x4-x3-x3) +
           nvg__absf(y2+y4-y3-y3) < ctx->tessTol) { nvg__addPoint(ctx, x4, y4, type); return;
            }*/

        const float x234 = (x23 + x34) * 0.5f;
        const float y234 = (y23 + y34) * 0.5f;
        const float x1234 = (x123 + x234) * 0.5f;
        const float y1234 = (y123 + y234) * 0.5f;

        nvg_tesselateBezier(ctx, x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1, 0);
        nvg_tesselateBezier(ctx, x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1, type);
    }

    static void nvg_flattenPaths(NVGcontext* ctx)
    {
        NVGpathCache* cache = ctx->cache;
        //	NVGstate* state = nvg__getState(ctx);
        NVGpoint* last;
        NVGpoint* pts;
        float* p;

        if (cache->npaths > 0)
            return;

        // Flatten
        int i = 0;
        while (i < ctx->ncommands)
        {
            const auto cmd = NVGcommands(ctx->commands[i]);
            switch (cmd)
            {
                case NVGcommands::MoveTo:
                    nvg_addPath(ctx);
                    p = &ctx->commands[i + 1];
                    nvg_addPoint(ctx, p[0], p[1], NVG_PT_CORNER);
                    i += 3;
                    break;
                case NVGcommands::LineTo:
                    p = &ctx->commands[i + 1];
                    nvg_addPoint(ctx, p[0], p[1], NVG_PT_CORNER);
                    i += 3;
                    break;
                case NVGcommands::Bezierto:
                    last = nvg_lastPoint(ctx);
                    if (last != nullptr)
                    {
                        const float* cp1 = &ctx->commands[i + 1];
                        const float* cp2 = &ctx->commands[i + 3];
                        p = &ctx->commands[i + 5];
                        nvg_tesselateBezier(ctx, last->x, last->y, cp1[0], cp1[1], cp2[0], cp2[1],
                                            p[0], p[1], 0, NVG_PT_CORNER);
                    }
                    i += 7;
                    break;
                case NVGcommands::Close:
                    nvg_closePath(ctx);
                    i++;
                    break;
                case NVGcommands::Winding:
                    nvg_pathWinding(ctx, static_cast<int>(ctx->commands[i + 1]));
                    i += 2;
                    break;
                default:
                    i++;
            }
        }

        cache->bounds[0] = cache->bounds[1] = 1e6f;
        cache->bounds[2] = cache->bounds[3] = -1e6f;

        // Calculate the direction and length of line segments.
        for (int j = 0; j < cache->npaths; j++)
        {
            NVGpath* path = &cache->paths[j];
            pts = &cache->points[path->first];

            // If the first and last points are the same, remove the last, mark as closed path.
            NVGpoint* p0 = &pts[path->count - 1];
            NVGpoint* p1 = &pts[0];
            if (nvg_ptEquals(p0->x, p0->y, p1->x, p1->y, ctx->distTol))
            {
                path->count--;
                p0 = &pts[path->count - 1];
                path->closed = 1;
            }

            // Enforce winding.
            if (path->count > 2)
            {
                const float area = nvg_polyArea(pts, path->count);
                if (path->winding == NVG_CCW && area < 0.0f)
                    nvg_polyReverse(pts, path->count);
                if (path->winding == NVG_CW && area > 0.0f)
                    nvg_polyReverse(pts, path->count);
            }

            for (i = 0; i < path->count; i++)
            {
                // Calculate segment direction and length
                p0->dx = p1->x - p0->x;
                p0->dy = p1->y - p0->y;
                p0->len = nvg_normalize(&p0->dx, &p0->dy);
                // Update bounds
                cache->bounds[0] = nvg_minf(cache->bounds[0], p0->x);
                cache->bounds[1] = nvg_minf(cache->bounds[1], p0->y);
                cache->bounds[2] = nvg_maxf(cache->bounds[2], p0->x);
                cache->bounds[3] = nvg_maxf(cache->bounds[3], p0->y);
                // Advance
                p0 = p1++;
            }
        }
    }

    static int nvg_curveDivs(const float r, const float arc, const float tol)
    {
        const float da = acosf(r / (r + tol)) * 2.0f;
        return nvg_maxi(2, static_cast<int>(ceilf(arc / da)));
    }

    static void nvg_chooseBevel(const int bevel, const NVGpoint* p0, const NVGpoint* p1,
                                const float w, float* x0, float* y0, float* x1, float* y1)
    {
        if (bevel)
        {
            *x0 = p1->x + p0->dy * w;
            *y0 = p1->y - p0->dx * w;
            *x1 = p1->x + p1->dy * w;
            *y1 = p1->y - p1->dx * w;
        }
        else
        {
            *x0 = p1->x + p1->dmx * w;
            *y0 = p1->y + p1->dmy * w;
            *x1 = p1->x + p1->dmx * w;
            *y1 = p1->y + p1->dmy * w;
        }
    }

    static NVGvertex* nvg_roundJoin(NVGvertex* dst, const NVGpoint* p0, const NVGpoint* p1,
                                    const float lw, const float rw, const float lu, const float ru,
                                    const int ncap, float fringe)
    {
        int i, n;
        const float dlx0 = p0->dy;
        const float dly0 = -p0->dx;
        const float dlx1 = p1->dy;
        const float dly1 = -p1->dx;
        NVG_NOTUSED(fringe);

        if (p1->flags & NVG_PT_LEFT)
        {
            float lx0, ly0, lx1, ly1;
            nvg_chooseBevel(p1->flags & NVG_PR_INNERBEVEL, p0, p1, lw, &lx0, &ly0, &lx1, &ly1);
            const float a0 = atan2f(-dly0, -dlx0);
            float a1 = atan2f(-dly1, -dlx1);
            if (a1 > a0)
                a1 -= std::numbers::pi_v<f32> * 2;

            nvg_vset(dst, lx0, ly0, lu, 1);
            dst++;
            nvg_vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
            dst++;

            n = nvg_clampi(static_cast<int>(ceilf(((a0 - a1) / std::numbers::pi_v<f32>)*ncap)), 2,
                           ncap);
            for (i = 0; i < n; i++)
            {
                const float u = i / static_cast<float>(n - 1);
                const float a = a0 + u * (a1 - a0);
                const float rx = p1->x + cosf(a) * rw;
                const float ry = p1->y + sinf(a) * rw;
                nvg_vset(dst, p1->x, p1->y, 0.5f, 1);
                dst++;
                nvg_vset(dst, rx, ry, ru, 1);
                dst++;
            }

            nvg_vset(dst, lx1, ly1, lu, 1);
            dst++;
            nvg_vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
            dst++;
        }
        else
        {
            float rx0, ry0, rx1, ry1;
            nvg_chooseBevel(p1->flags & NVG_PR_INNERBEVEL, p0, p1, -rw, &rx0, &ry0, &rx1, &ry1);
            const float a0 = atan2f(dly0, dlx0);
            float a1 = atan2f(dly1, dlx1);
            if (a1 < a0)
                a1 += std::numbers::pi_v<f32> * 2;

            nvg_vset(dst, p1->x + dlx0 * rw, p1->y + dly0 * rw, lu, 1);
            dst++;
            nvg_vset(dst, rx0, ry0, ru, 1);
            dst++;

            n = nvg_clampi(static_cast<int>(ceilf(((a1 - a0) / std::numbers::pi_v<f32>)*ncap)), 2,
                           ncap);
            for (i = 0; i < n; i++)
            {
                const float u = i / static_cast<float>(n - 1);
                const float a = a0 + u * (a1 - a0);
                const float lx = p1->x + cosf(a) * lw;
                const float ly = p1->y + sinf(a) * lw;
                nvg_vset(dst, lx, ly, lu, 1);
                dst++;
                nvg_vset(dst, p1->x, p1->y, 0.5f, 1);
                dst++;
            }

            nvg_vset(dst, p1->x + dlx1 * rw, p1->y + dly1 * rw, lu, 1);
            dst++;
            nvg_vset(dst, rx1, ry1, ru, 1);
            dst++;
        }
        return dst;
    }

    static NVGvertex* nvg_bevelJoin(NVGvertex* dst, const NVGpoint* p0, const NVGpoint* p1,
                                    const float lw, const float rw, const float lu, const float ru,
                                    float fringe)
    {
        float rx0, ry0, rx1, ry1;
        float lx0, ly0, lx1, ly1;
        const float dlx0 = p0->dy;
        const float dly0 = -p0->dx;
        const float dlx1 = p1->dy;
        const float dly1 = -p1->dx;
        NVG_NOTUSED(fringe);

        if (p1->flags & NVG_PT_LEFT)
        {
            nvg_chooseBevel(p1->flags & NVG_PR_INNERBEVEL, p0, p1, lw, &lx0, &ly0, &lx1, &ly1);

            nvg_vset(dst, lx0, ly0, lu, 1);
            dst++;
            nvg_vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
            dst++;

            if (p1->flags & NVG_PT_BEVEL)
            {
                nvg_vset(dst, lx0, ly0, lu, 1);
                dst++;
                nvg_vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
                dst++;

                nvg_vset(dst, lx1, ly1, lu, 1);
                dst++;
                nvg_vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
                dst++;
            }
            else
            {
                rx0 = p1->x - p1->dmx * rw;
                ry0 = p1->y - p1->dmy * rw;

                nvg_vset(dst, p1->x, p1->y, 0.5f, 1);
                dst++;
                nvg_vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
                dst++;

                nvg_vset(dst, rx0, ry0, ru, 1);
                dst++;
                nvg_vset(dst, rx0, ry0, ru, 1);
                dst++;

                nvg_vset(dst, p1->x, p1->y, 0.5f, 1);
                dst++;
                nvg_vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
                dst++;
            }

            nvg_vset(dst, lx1, ly1, lu, 1);
            dst++;
            nvg_vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
            dst++;
        }
        else
        {
            nvg_chooseBevel(p1->flags & NVG_PR_INNERBEVEL, p0, p1, -rw, &rx0, &ry0, &rx1, &ry1);

            nvg_vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1);
            dst++;
            nvg_vset(dst, rx0, ry0, ru, 1);
            dst++;

            if (p1->flags & NVG_PT_BEVEL)
            {
                nvg_vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1);
                dst++;
                nvg_vset(dst, rx0, ry0, ru, 1);
                dst++;

                nvg_vset(dst, p1->x + dlx1 * lw, p1->y + dly1 * lw, lu, 1);
                dst++;
                nvg_vset(dst, rx1, ry1, ru, 1);
                dst++;
            }
            else
            {
                lx0 = p1->x + p1->dmx * lw;
                ly0 = p1->y + p1->dmy * lw;

                nvg_vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1);
                dst++;
                nvg_vset(dst, p1->x, p1->y, 0.5f, 1);
                dst++;

                nvg_vset(dst, lx0, ly0, lu, 1);
                dst++;
                nvg_vset(dst, lx0, ly0, lu, 1);
                dst++;

                nvg_vset(dst, p1->x + dlx1 * lw, p1->y + dly1 * lw, lu, 1);
                dst++;
                nvg_vset(dst, p1->x, p1->y, 0.5f, 1);
                dst++;
            }

            nvg_vset(dst, p1->x + dlx1 * lw, p1->y + dly1 * lw, lu, 1);
            dst++;
            nvg_vset(dst, rx1, ry1, ru, 1);
            dst++;
        }

        return dst;
    }

    static NVGvertex* nvg_buttCapStart(NVGvertex* dst, const NVGpoint* p, const float dx,
                                       const float dy, const float w, const float d, const float aa,
                                       const float u0, const float u1)
    {
        const float px = p->x - dx * d;
        const float py = p->y - dy * d;
        const float dlx = dy;
        const float dly = -dx;
        nvg_vset(dst, px + dlx * w - dx * aa, py + dly * w - dy * aa, u0, 0);
        dst++;
        nvg_vset(dst, px - dlx * w - dx * aa, py - dly * w - dy * aa, u1, 0);
        dst++;
        nvg_vset(dst, px + dlx * w, py + dly * w, u0, 1);
        dst++;
        nvg_vset(dst, px - dlx * w, py - dly * w, u1, 1);
        dst++;
        return dst;
    }

    static NVGvertex* nvg_buttCapEnd(NVGvertex* dst, const NVGpoint* p, const float dx,
                                     const float dy, const float w, const float d, const float aa,
                                     const float u0, const float u1)
    {
        const float px = p->x + dx * d;
        const float py = p->y + dy * d;
        const float dlx = dy;
        const float dly = -dx;
        nvg_vset(dst, px + dlx * w, py + dly * w, u0, 1);
        dst++;
        nvg_vset(dst, px - dlx * w, py - dly * w, u1, 1);
        dst++;
        nvg_vset(dst, px + dlx * w + dx * aa, py + dly * w + dy * aa, u0, 0);
        dst++;
        nvg_vset(dst, px - dlx * w + dx * aa, py - dly * w + dy * aa, u1, 0);
        dst++;
        return dst;
    }

    static NVGvertex* nvg_roundCapStart(NVGvertex* dst, const NVGpoint* p, const float dx,
                                        const float dy, const float w, const int ncap, float aa,
                                        const float u0, const float u1)
    {
        const float px = p->x;
        const float py = p->y;
        const float dlx = dy;
        const float dly = -dx;
        NVG_NOTUSED(aa);
        for (int i = 0; i < ncap; i++)
        {
            const float a = i / static_cast<float>(ncap - 1) * std::numbers::pi_v<f32>;
            float ax = cosf(a) * w, ay = sinf(a) * w;
            nvg_vset(dst, px - dlx * ax - dx * ay, py - dly * ax - dy * ay, u0, 1);
            dst++;
            nvg_vset(dst, px, py, 0.5f, 1);
            dst++;
        }
        nvg_vset(dst, px + dlx * w, py + dly * w, u0, 1);
        dst++;
        nvg_vset(dst, px - dlx * w, py - dly * w, u1, 1);
        dst++;
        return dst;
    }

    static NVGvertex* nvg_roundCapEnd(NVGvertex* dst, const NVGpoint* p, const float dx,
                                      const float dy, const float w, const int ncap, float aa,
                                      const float u0, const float u1)
    {
        const float px = p->x;
        const float py = p->y;
        const float dlx = dy;
        const float dly = -dx;
        NVG_NOTUSED(aa);
        nvg_vset(dst, px + dlx * w, py + dly * w, u0, 1);
        dst++;
        nvg_vset(dst, px - dlx * w, py - dly * w, u1, 1);
        dst++;
        for (int i = 0; i < ncap; i++)
        {
            const float a = i / static_cast<float>(ncap - 1) * std::numbers::pi_v<f32>;
            float ax = cosf(a) * w, ay = sinf(a) * w;
            nvg_vset(dst, px, py, 0.5f, 1);
            dst++;
            nvg_vset(dst, px - dlx * ax + dx * ay, py - dly * ax + dy * ay, u0, 1);
            dst++;
        }
        return dst;
    }

    static void nvg_calculateJoins(const NVGcontext* ctx, const float w, const int lineJoin,
                                   const float miterLimit)
    {
        const NVGpathCache* cache = ctx->cache;
        float iw = 0.0f;

        if (w > 0.0f)
            iw = 1.0f / w;

        // Calculate which joins needs extra vertices to append, and gather vertex count.
        for (int i = 0; i < cache->npaths; i++)
        {
            NVGpath* path = &cache->paths[i];
            NVGpoint* pts = &cache->points[path->first];
            const NVGpoint* p0 = &pts[path->count - 1];
            NVGpoint* p1 = &pts[0];
            int nleft = 0;

            path->nbevel = 0;

            for (int j = 0; j < path->count; j++)
            {
                const float dlx0 = p0->dy;
                const float dly0 = -p0->dx;
                const float dlx1 = p1->dy;
                const float dly1 = -p1->dx;
                // Calculate extrusions
                p1->dmx = (dlx0 + dlx1) * 0.5f;
                p1->dmy = (dly0 + dly1) * 0.5f;
                const float dmr2 = p1->dmx * p1->dmx + p1->dmy * p1->dmy;
                if (dmr2 > 0.000001f)
                {
                    float scale = 1.0f / dmr2;
                    if (scale > 600.0f)
                        scale = 600.0f;
                    p1->dmx *= scale;
                    p1->dmy *= scale;
                }

                // Clear flags, but keep the corner.
                p1->flags = (p1->flags & NVG_PT_CORNER) ? NVG_PT_CORNER : 0;

                // Keep track of left turns.
                const float cross = p1->dx * p0->dy - p0->dx * p1->dy;
                if (cross > 0.0f)
                {
                    nleft++;
                    p1->flags |= NVG_PT_LEFT;
                }

                // Calculate if we should use bevel or miter for inner join.
                const float limit = nvg_maxf(1.01f, nvg_minf(p0->len, p1->len) * iw);
                if ((dmr2 * limit * limit) < 1.0f)
                    p1->flags |= NVG_PR_INNERBEVEL;

                // Check to see if the corner needs to be beveled.
                if (p1->flags & NVG_PT_CORNER)
                    if ((dmr2 * miterLimit * miterLimit) < 1.0f || lineJoin == NVG_BEVEL ||
                        lineJoin == NVG_ROUND)
                        p1->flags |= NVG_PT_BEVEL;

                if ((p1->flags & (NVG_PT_BEVEL | NVG_PR_INNERBEVEL)) != 0)
                    path->nbevel++;

                p0 = p1++;
            }

            path->convex = (nleft == path->count) ? 1 : 0;
        }
    }

    static int nvg_expandStroke(const NVGcontext* ctx, float w, const float fringe,
                                const int lineCap, const int lineJoin, const float miterLimit)
    {
        const NVGpathCache* cache = ctx->cache;
        int i;
        const float aa = fringe;  // ctx->fringeWidth;
        float u0 = 0.0f, u1 = 1.0f;
        const int ncap = nvg_curveDivs(w, std::numbers::pi_v<f32>, ctx->tessTol);  // Calculate
        // divisions per
        // half circle.

        w += aa * 0.5f;

        // Disable the gradient used for antialiasing when antialiasing is not used.
        if (aa == 0.0f)
        {
            u0 = 0.5f;
            u1 = 0.5f;
        }

        nvg_calculateJoins(ctx, w, lineJoin, miterLimit);

        // Calculate max vertex usage.
        int cverts = 0;
        for (i = 0; i < cache->npaths; i++)
        {
            const NVGpath* path = &cache->paths[i];
            const int loop = (path->closed == 0) ? 0 : 1;
            if (lineJoin == NVG_ROUND)
                cverts += (path->count + path->nbevel * (ncap + 2) + 1) * 2;  // plus one for loop
            else
                cverts += (path->count + path->nbevel * 5 + 1) * 2;  // plus one for loop
            if (loop == 0)
            {
                // space for caps
                if (lineCap == NVG_ROUND)
                    cverts += (ncap * 2 + 2) * 2;
                else
                    cverts += (3 + 3) * 2;
            }
        }

        NVGvertex* verts = nvg_allocTempVerts(ctx, cverts);
        if (verts == nullptr)
            return 0;

        for (i = 0; i < cache->npaths; i++)
        {
            NVGpath* path = &cache->paths[i];
            NVGpoint* pts = &cache->points[path->first];
            NVGpoint* p0;
            NVGpoint* p1;
            int s, e;
            float dx, dy;

            path->fill = nullptr;
            path->nfill = 0;

            // Calculate fringe or stroke
            const int loop = (path->closed == 0) ? 0 : 1;
            NVGvertex* dst = verts;
            path->stroke = dst;

            if (loop)
            {
                // Looping
                p0 = &pts[path->count - 1];
                p1 = &pts[0];
                s = 0;
                e = path->count;
            }
            else
            {
                // Add cap
                p0 = &pts[0];
                p1 = &pts[1];
                s = 1;
                e = path->count - 1;
            }

            if (loop == 0)
            {
                // Add cap
                dx = p1->x - p0->x;
                dy = p1->y - p0->y;
                nvg_normalize(&dx, &dy);
                if (lineCap == NVG_BUTT)
                    dst = nvg_buttCapStart(dst, p0, dx, dy, w, -aa * 0.5f, aa, u0, u1);
                else if (lineCap == NVG_BUTT || lineCap == NVG_SQUARE)
                    dst = nvg_buttCapStart(dst, p0, dx, dy, w, w - aa, aa, u0, u1);
                else if (lineCap == NVG_ROUND)
                    dst = nvg_roundCapStart(dst, p0, dx, dy, w, ncap, aa, u0, u1);
            }

            for (int j = s; j < e; ++j)
            {
                if ((p1->flags & (NVG_PT_BEVEL | NVG_PR_INNERBEVEL)) != 0)
                {
                    if (lineJoin == NVG_ROUND)
                        dst = nvg_roundJoin(dst, p0, p1, w, w, u0, u1, ncap, aa);
                    else
                        dst = nvg_bevelJoin(dst, p0, p1, w, w, u0, u1, aa);
                }
                else
                {
                    nvg_vset(dst, p1->x + (p1->dmx * w), p1->y + (p1->dmy * w), u0, 1);
                    dst++;
                    nvg_vset(dst, p1->x - (p1->dmx * w), p1->y - (p1->dmy * w), u1, 1);
                    dst++;
                }
                p0 = p1++;
            }

            if (loop)
            {
                // Loop it
                nvg_vset(dst, verts[0].x, verts[0].y, u0, 1);
                dst++;
                nvg_vset(dst, verts[1].x, verts[1].y, u1, 1);
                dst++;
            }
            else
            {
                // Add cap
                dx = p1->x - p0->x;
                dy = p1->y - p0->y;
                nvg_normalize(&dx, &dy);
                if (lineCap == NVG_BUTT)
                    dst = nvg_buttCapEnd(dst, p1, dx, dy, w, -aa * 0.5f, aa, u0, u1);
                else if (lineCap == NVG_BUTT || lineCap == NVG_SQUARE)
                    dst = nvg_buttCapEnd(dst, p1, dx, dy, w, w - aa, aa, u0, u1);
                else if (lineCap == NVG_ROUND)
                    dst = nvg_roundCapEnd(dst, p1, dx, dy, w, ncap, aa, u0, u1);
            }

            path->nstroke = static_cast<int>(dst - verts);

            verts = dst;
        }

        return 1;
    }

    static int nvg_expandFill(const NVGcontext* ctx, const float w, const int lineJoin,
                              const float miterLimit)
    {
        const NVGpathCache* cache = ctx->cache;
        int i, j;
        const float aa = ctx->fringeWidth;
        const int fringe = w > 0.0f;

        nvg_calculateJoins(ctx, w, lineJoin, miterLimit);

        // Calculate max vertex usage.
        int cverts = 0;
        for (i = 0; i < cache->npaths; i++)
        {
            const NVGpath* path = &cache->paths[i];
            cverts += path->count + path->nbevel + 1;
            if (fringe)
                cverts += (path->count + path->nbevel * 5 + 1) * 2;  // plus one for loop
        }

        NVGvertex* verts = nvg_allocTempVerts(ctx, cverts);
        if (verts == nullptr)
            return 0;

        const auto convex = (cache->npaths == 1) && (cache->paths[0].convex);

        for (i = 0; i < cache->npaths; i++)
        {
            NVGpath* path = &cache->paths[i];
            NVGpoint* pts = &cache->points[path->first];
            NVGpoint* p0;
            NVGpoint* p1;

            // Calculate shape vertices.
            const float woff = 0.5f * aa;
            NVGvertex* dst = verts;
            path->fill = dst;

            if (fringe)
            {
                // Looping
                p0 = &pts[path->count - 1];
                p1 = &pts[0];
                for (j = 0; j < path->count; ++j)
                {
                    if (p1->flags & NVG_PT_BEVEL)
                    {
                        const float dlx0 = p0->dy;
                        const float dly0 = -p0->dx;
                        const float dlx1 = p1->dy;
                        const float dly1 = -p1->dx;
                        if (p1->flags & NVG_PT_LEFT)
                        {
                            const float lx = p1->x + p1->dmx * woff;
                            const float ly = p1->y + p1->dmy * woff;
                            nvg_vset(dst, lx, ly, 0.5f, 1);
                            dst++;
                        }
                        else
                        {
                            const float lx0 = p1->x + dlx0 * woff;
                            const float ly0 = p1->y + dly0 * woff;
                            const float lx1 = p1->x + dlx1 * woff;
                            const float ly1 = p1->y + dly1 * woff;
                            nvg_vset(dst, lx0, ly0, 0.5f, 1);
                            dst++;
                            nvg_vset(dst, lx1, ly1, 0.5f, 1);
                            dst++;
                        }
                    }
                    else
                    {
                        nvg_vset(dst, p1->x + (p1->dmx * woff), p1->y + (p1->dmy * woff), 0.5f, 1);
                        dst++;
                    }
                    p0 = p1++;
                }
            }
            else
                for (j = 0; j < path->count; ++j)
                {
                    nvg_vset(dst, pts[j].x, pts[j].y, 0.5f, 1);
                    dst++;
                }

            path->nfill = static_cast<int>(dst - verts);
            verts = dst;

            // Calculate fringe
            if (fringe)
            {
                float lw = w + woff;
                const float rw = w - woff;
                float lu = 0;
                const float ru = 1;
                dst = verts;
                path->stroke = dst;

                // Create only half a fringe for convex shapes so that
                // the shape can be rendered without stenciling.
                if (convex)
                {
                    lw = woff;  // This should generate the same vertex as fill inset above.
                    lu = 0.5f;  // Set outline fade at middle.
                }

                // Looping
                p0 = &pts[path->count - 1];
                p1 = &pts[0];

                for (j = 0; j < path->count; ++j)
                {
                    if ((p1->flags & (NVG_PT_BEVEL | NVG_PR_INNERBEVEL)) != 0)
                        dst = nvg_bevelJoin(dst, p0, p1, lw, rw, lu, ru, ctx->fringeWidth);
                    else
                    {
                        nvg_vset(dst, p1->x + (p1->dmx * lw), p1->y + (p1->dmy * lw), lu, 1);
                        dst++;
                        nvg_vset(dst, p1->x - (p1->dmx * rw), p1->y - (p1->dmy * rw), ru, 1);
                        dst++;
                    }
                    p0 = p1++;
                }

                // Loop it
                nvg_vset(dst, verts[0].x, verts[0].y, lu, 1);
                dst++;
                nvg_vset(dst, verts[1].x, verts[1].y, ru, 1);
                dst++;

                path->nstroke = static_cast<int>(dst - verts);
                verts = dst;
            }
            else
            {
                path->stroke = nullptr;
                path->nstroke = 0;
            }
        }

        return 1;
    }

    // Draw
    void begin_path(NVGcontext* ctx)
    {
        ctx->ncommands = 0;
        nvg_clearPathCache(ctx);
    }

    void move_to(NVGcontext* ctx, const float x, const float y)
    {
        auto vals = std::array{ (float)MoveTo, x, y };
        nvg_appendCommands(ctx, vals.data(), static_cast<int>(vals.size()));
    }

    void line_to(NVGcontext* ctx, const float x, const float y)
    {
        auto vals = std::array{ (float)LineTo, x, y };
        nvg_appendCommands(ctx, vals.data(), static_cast<int>(vals.size()));
    }

    void bezier_to(NVGcontext* ctx, const float c1_x, const float c1_y, const float c2_x,
                   const float c2_y, const float x, const float y)
    {
        auto vals = std::array{ (float)Bezierto, x, y };
        nvg_appendCommands(ctx, vals.data(), static_cast<int>(vals.size()));
    }

    void quad_to(NVGcontext* ctx, const float cx, const float cy, const float x, const float y)
    {
        const float x0 = ctx->commandx;
        const float y0 = ctx->commandy;
        auto vals = std::array{ (float)Bezierto,
                                x0 + 2.0f / 3.0f * (cx - x0),
                                y0 + 2.0f / 3.0f * (cy - y0),
                                x + 2.0f / 3.0f * (cx - x),
                                y + 2.0f / 3.0f * (cy - y),
                                x,
                                y };
        nvg_appendCommands(ctx, vals.data(), static_cast<int>(vals.size()));
    }

    void arc_to(NVGcontext* ctx, const float x1, const float y1, const float x2, const float y2,
                const float radius)
    {
        const float x0 = ctx->commandx;
        const float y0 = ctx->commandy;
        float dx0, dy0, dx1, dy1, cx, cy, a0, a1;
        int dir;

        if (ctx->ncommands == 0)
            return;

        // Handle degenerate cases.
        if (nvg_ptEquals(x0, y0, x1, y1, ctx->distTol) ||
            nvg_ptEquals(x1, y1, x2, y2, ctx->distTol) ||
            nvg_distPtSeg(x1, y1, x0, y0, x2, y2) < ctx->distTol * ctx->distTol ||
            radius < ctx->distTol)
        {
            line_to(ctx, x1, y1);
            return;
        }

        // Calculate tangential circle to lines (x0,y0)-(x1,y1) and (x1,y1)-(x2,y2).
        dx0 = x0 - x1;
        dy0 = y0 - y1;
        dx1 = x2 - x1;
        dy1 = y2 - y1;
        nvg_normalize(&dx0, &dy0);
        nvg_normalize(&dx1, &dy1);
        const float a = nvg_acosf(dx0 * dx1 + dy0 * dy1);
        const float d = radius / nvg_tanf(a / 2.0f);

        //	printf("a=%f° d=%f\n", a/std::numbers::pi_v<f32>*180.0f, d);

        if (d > 10000.0f)
        {
            line_to(ctx, x1, y1);
            return;
        }

        if (nvg_cross(dx0, dy0, dx1, dy1) > 0.0f)
        {
            cx = x1 + dx0 * d + dy0 * radius;
            cy = y1 + dy0 * d + -dx0 * radius;
            a0 = nvg_atan2f(dx0, -dy0);
            a1 = nvg_atan2f(-dx1, dy1);
            dir = NVG_CW;
            //		printf("CW c=(%f, %f) a0=%f° a1=%f°\n", cx, cy,
            // a0/std::numbers::pi_v<f32>*180.0f,
            // a1/std::numbers::pi_v<f32>*180.0f);
        }
        else
        {
            cx = x1 + dx0 * d + -dy0 * radius;
            cy = y1 + dy0 * d + dx0 * radius;
            a0 = nvg_atan2f(-dx0, dy0);
            a1 = nvg_atan2f(dx1, -dy1);
            dir = NVG_CCW;
            //		printf("CCW c=(%f, %f) a0=%f° a1=%f°\n", cx, cy,
            // a0/std::numbers::pi_v<f32>*180.0f,
            // a1/std::numbers::pi_v<f32>*180.0f);
        }

        arc(ctx, cx, cy, radius, a0, a1, dir);
    }

    void close_path(NVGcontext* ctx)
    {
        float vals[]{ static_cast<float>(NVGcommands::Close) };
        nvg_appendCommands(ctx, vals, std::size(vals));
    }

    void path_winding(NVGcontext* ctx, const int dir)
    {
        float vals[] = { static_cast<float>(NVGcommands::Winding), static_cast<float>(dir) };
        nvg_appendCommands(ctx, vals, std::size(vals));
    }

    void barc(NVGcontext* ctx, const float cx, const float cy, const float r, const float a0,
              const float a1, const int dir, const int join)
    {
        float px = 0.0f;
        float py = 0.0f;
        float ptanx = 0.0f;
        float ptany = 0.0f;
        float vals[3 + 5 * 7 + 100] = { 0.0f };
        const int move = join && ctx->ncommands > 0 ? NVGcommands::LineTo : NVGcommands::MoveTo;

        // Clamp angles
        float da = a1 - a0;
        if (dir == NVG_CW)
        {
            if (nvg_absf(da) >= std::numbers::pi_v<f32> * 2)
                da = std::numbers::pi_v<f32> * 2;
            else
                while (da < 0.0f)
                    da += std::numbers::pi_v<f32> * 2;
        }
        else
        {
            if (nvg_absf(da) >= std::numbers::pi_v<f32> * 2)
                da = -std::numbers::pi_v<f32> * 2;
            else
                while (da > 0.0f)
                    da -= std::numbers::pi_v<f32> * 2;
        }

        // Split arc into max 90 degree segments.
        const int ndivs = nvg_maxi(
            1,
            nvg_mini(static_cast<int>(nvg_absf(da) / (std::numbers::pi_v<f32> * 0.5f) + 0.5f), 5));
        const float hda = (da / static_cast<float>(ndivs)) / 2.0f;
        float kappa = nvg_absf(4.0f / 3.0f * (1.0f - nvg_cosf(hda)) / nvg_sinf(hda));

        if (dir == NVG_CCW)
            kappa = -kappa;

        int nvals = 0;
        for (int i = 0; i <= ndivs; i++)
        {
            const float a = a0 + da * (static_cast<float>(i) / static_cast<float>(ndivs));
            const float dx = nvg_cosf(a);
            const float dy = nvg_sinf(a);
            const float x = cx + dx * r;
            const float y = cy + dy * r;
            const float tanx = -dy * r * kappa;
            const float tany = dx * r * kappa;

            if (i == 0)
            {
                vals[nvals++] = static_cast<float>(move);
                vals[nvals++] = x;
                vals[nvals++] = y;
            }
            else
            {
                vals[nvals++] = static_cast<float>(Bezierto);
                vals[nvals++] = px + ptanx;
                vals[nvals++] = py + ptany;
                vals[nvals++] = x - tanx;
                vals[nvals++] = y - tany;
                vals[nvals++] = x;
                vals[nvals++] = y;
            }

            px = x;
            py = y;
            ptanx = tanx;
            ptany = tany;
        }

        nvg_appendCommands(ctx, vals, nvals);
    }

    void arc(NVGcontext* ctx, const float cx, const float cy, const float r, const float a0,
             const float a1, const int dir)
    {
        barc(ctx, cx, cy, r, a0, a1, dir, 1);
    }

    void rect(NVGcontext* ctx, const float x, const float y, const float w, const float h)
    {
        auto vals = std::array{ static_cast<f32>(MoveTo), x,     y,
                                static_cast<f32>(LineTo), x,     y + h,
                                static_cast<f32>(LineTo), x + w, y + h,
                                static_cast<f32>(LineTo), x + w, y,
                                static_cast<f32>(Close) };
        nvg_appendCommands(ctx, vals.data(), std::size(vals));
    }

    void rounded_rect(NVGcontext* ctx, const float x, const float y, const float w, const float h,
                      const float r)
    {
        rounded_rect_varying(ctx, x, y, w, h, r, r, r, r);
    }

    void rounded_rect_varying(NVGcontext* ctx, const float x, const float y, const float w,
                              const float h, const float rad_top_left, const float rad_top_right,
                              const float rad_bottom_right, const float rad_bottom_left)
    {
        if (rad_top_left < 0.1f && rad_top_right < 0.1f && rad_bottom_right < 0.1f &&
            rad_bottom_left < 0.1f)
        {
            rect(ctx, x, y, w, h);
            return;
        }

        const float halfw = nvg_absf(w) * 0.5f;
        const float halfh = nvg_absf(h) * 0.5f;
        const float rx_bl = nvg_minf(rad_bottom_left, halfw) * nvg_signf(w);
        const float ry_bl = nvg_minf(rad_bottom_left, halfh) * nvg_signf(h);
        const float rx_br = nvg_minf(rad_bottom_right, halfw) * nvg_signf(w);
        const float ry_br = nvg_minf(rad_bottom_right, halfh) * nvg_signf(h);
        const float rx_tr = nvg_minf(rad_top_right, halfw) * nvg_signf(w);
        const float ry_tr = nvg_minf(rad_top_right, halfh) * nvg_signf(h);
        const float rx_tl = nvg_minf(rad_top_left, halfw) * nvg_signf(w);
        const float ry_tl = nvg_minf(rad_top_left, halfh) * nvg_signf(h);

        auto vals = std::array{ static_cast<float>(NVGcommands::MoveTo),
                                x,
                                y + ry_tl,
                                static_cast<float>(NVGcommands::LineTo),
                                x,
                                y + h - ry_bl,
                                static_cast<float>(NVGcommands::Bezierto),
                                x,
                                y + h - ry_bl * (1 - NVG_KAPPA90),
                                x + rx_bl * (1 - NVG_KAPPA90),
                                y + h,
                                x + rx_bl,
                                y + h,
                                static_cast<float>(NVGcommands::LineTo),
                                x + w - rx_br,
                                y + h,
                                static_cast<float>(NVGcommands::Bezierto),
                                x + w - rx_br * (1 - NVG_KAPPA90),
                                y + h,
                                x + w,
                                y + h - ry_br * (1 - NVG_KAPPA90),
                                x + w,
                                y + h - ry_br,
                                static_cast<float>(NVGcommands::LineTo),
                                x + w,
                                y + ry_tr,
                                static_cast<float>(NVGcommands::Bezierto),
                                x + w,
                                y + ry_tr * (1 - NVG_KAPPA90),
                                x + w - rx_tr * (1 - NVG_KAPPA90),
                                y,
                                x + w - rx_tr,
                                y,
                                static_cast<float>(NVGcommands::LineTo),
                                x + rx_tl,
                                y,
                                static_cast<float>(NVGcommands::Bezierto),
                                x + rx_tl * (1 - NVG_KAPPA90),
                                y,
                                x,
                                y + ry_tl * (1 - NVG_KAPPA90),
                                x,
                                y + ry_tl,
                                static_cast<float>(NVGcommands::Close) };
        nvg_appendCommands(ctx, vals.data(), vals.size());
    }

    void ellipse(NVGcontext* ctx, const float cx, const float cy, const float rx, const float ry)
    {
        auto vals = std::array{ static_cast<float>(NVGcommands::MoveTo),
                                cx - rx,
                                cy,
                                static_cast<float>(NVGcommands::Bezierto),
                                cx - rx,
                                cy + ry * NVG_KAPPA90,
                                cx - rx * NVG_KAPPA90,
                                cy + ry,
                                cx,
                                cy + ry,
                                static_cast<float>(NVGcommands::Bezierto),
                                cx + rx * NVG_KAPPA90,
                                cy + ry,
                                cx + rx,
                                cy + ry * NVG_KAPPA90,
                                cx + rx,
                                cy,
                                static_cast<float>(NVGcommands::Bezierto),
                                cx + rx,
                                cy - ry * NVG_KAPPA90,
                                cx + rx * NVG_KAPPA90,
                                cy - ry,
                                cx,
                                cy - ry,
                                static_cast<float>(NVGcommands::Bezierto),
                                cx - rx * NVG_KAPPA90,
                                cy - ry,
                                cx - rx,
                                cy - ry * NVG_KAPPA90,
                                cx - rx,
                                cy,
                                static_cast<float>(NVGcommands::Close) };
        nvg_appendCommands(ctx, vals.data(), std::size(vals));
    }

    void circle(NVGcontext* ctx, const float cx, const float cy, const float r)
    {
        ellipse(ctx, cx, cy, r, r);
    }

    void debug_dump_path_cache(const NVGcontext* ctx)
    {
        int j;

        std::println("Dumping {} cached paths", ctx->cache->npaths);
        for (int i = 0; i < ctx->cache->npaths; i++)
        {
            const NVGpath* path = &ctx->cache->paths[i];
            std::println(" - Path {}", i);
            if (path->nfill)
            {
                std::println("   - fill: {}", path->nfill);
                for (j = 0; j < path->nfill; j++)
                    std::println("{:f}\t{:f}", path->fill[j].x, path->fill[j].y);
            }
            if (path->nstroke)
            {
                std::println("   - stroke: {}", path->nstroke);
                for (j = 0; j < path->nstroke; j++)
                    std::println("{:f}\t{:f}", path->stroke[j].x, path->stroke[j].y);
            }
        }
    }

    void fill(NVGcontext* ctx)
    {
        NVGstate* state = nvg_getState(ctx);
        NVGpaint fill_paint = state->fill;

        nvg_flattenPaths(ctx);
        if (ctx->params.edgeAntiAlias && state->shapeAntiAlias)
            nvg_expandFill(ctx, ctx->fringeWidth, NVG_MITER, 2.4f);
        else
            nvg_expandFill(ctx, 0.0f, NVG_MITER, 2.4f);

        // Apply global alpha
        fill_paint.innerColor.a *= state->alpha;
        fill_paint.outerColor.a *= state->alpha;

        ctx->params.renderFill(ctx->params.userPtr, &fill_paint, state->composite_operation,
                               &state->scissor, ctx->fringeWidth, ctx->cache->bounds,
                               ctx->cache->paths, ctx->cache->npaths);

        // Count triangles
        for (int i = 0; i < ctx->cache->npaths; i++)
        {
            const NVGpath* path = &ctx->cache->paths[i];
            ctx->fillTriCount += path->nfill - 2;
            ctx->fillTriCount += path->nstroke - 2;
            ctx->drawCallCount += 2;
        }
    }

    void stroke(NVGcontext* ctx)
    {
        NVGstate* state = nvg_getState(ctx);
        const float scale = nvg_getAverageScale(state->xform);
        float stroke_width = nvg_clampf(state->strokeWidth * scale, 0.0f, 200.0f);
        NVGpaint stroke_paint = state->stroke;

        if (stroke_width < ctx->fringeWidth)
        {
            // If the stroke width is less than pixel size, use alpha to emulate coverage.
            // Since coverage is area, scale by alpha*alpha.
            const float alpha = nvg_clampf(stroke_width / ctx->fringeWidth, 0.0f, 1.0f);
            stroke_paint.innerColor.a *= alpha * alpha;
            stroke_paint.outerColor.a *= alpha * alpha;
            stroke_width = ctx->fringeWidth;
        }

        // Apply global alpha
        stroke_paint.innerColor.a *= state->alpha;
        stroke_paint.outerColor.a *= state->alpha;

        nvg_flattenPaths(ctx);

        if (ctx->params.edgeAntiAlias && state->shapeAntiAlias)
            nvg_expandStroke(ctx, stroke_width * 0.5f, ctx->fringeWidth, state->lineCap,
                             state->lineJoin, state->miterLimit);
        else
            nvg_expandStroke(ctx, stroke_width * 0.5f, 0.0f, state->lineCap, state->lineJoin,
                             state->miterLimit);

        ctx->params.renderStroke(ctx->params.userPtr, &stroke_paint, state->composite_operation,
                                 &state->scissor, ctx->fringeWidth, stroke_width, ctx->cache->paths,
                                 ctx->cache->npaths);

        // Count triangles
        for (int i = 0; i < ctx->cache->npaths; i++)
        {
            const NVGpath* path = &ctx->cache->paths[i];
            ctx->strokeTriCount += path->nstroke - 2;
            ctx->drawCallCount++;
        }
    }

    // Add fonts
    int create_font(const NVGcontext* ctx, const char* name, const char* filename)
    {
        return fonsAddFont(ctx->fs, name, filename, 0);
    }

    int create_font_at_index(const NVGcontext* ctx, const char* name, const char* filename,
                             const int font_index)
    {
        return fonsAddFont(ctx->fs, name, filename, font_index);
    }

    int create_font_mem(const NVGcontext* ctx, const char* name, unsigned char* data,
                        const int ndata, const int free_data)
    {
        return fonsAddFontMem(ctx->fs, name, data, ndata, free_data, 0);
    }

    i32 create_font_mem(const NVGcontext* ctx, const std::string_view& name,
                        const std::basic_string_view<u8>& font_data) noexcept
    {
        constexpr static i32 font_index{ 0 };
        constexpr static i32 dealloc_data{ false };

        return fonsAddFontMem(ctx->fs, name.data(), (unsigned char*)font_data.data(),
                              static_cast<i32>(font_data.size()), dealloc_data, font_index);
    }

    int create_font_mem_at_index(const NVGcontext* ctx, const char* name, unsigned char* data,
                                 const int ndata, const int free_data, const int font_index)
    {
        return fonsAddFontMem(ctx->fs, name, data, ndata, free_data, font_index);
    }

    int find_font(const NVGcontext* ctx, const char* name)
    {
        if (name == nullptr)
            return -1;
        return fonsGetFontByName(ctx->fs, name);
    }

    int add_fallback_font_id(const NVGcontext* ctx, const int base_font, const int fallback_font)
    {
        if (base_font == -1 || fallback_font == -1)
            return 0;
        return fonsAddFallbackFont(ctx->fs, base_font, fallback_font);
    }

    int add_fallback_font(const NVGcontext* ctx, const char* base_font, const char* fallback_font)
    {
        return add_fallback_font_id(ctx, find_font(ctx, base_font), find_font(ctx, fallback_font));
    }

    void reset_fallback_fonts_id(const NVGcontext* ctx, const int base_font)
    {
        fonsResetFallbackFont(ctx->fs, base_font);
    }

    void reset_fallback_fonts(const NVGcontext* ctx, const char* base_font)
    {
        reset_fallback_fonts_id(ctx, find_font(ctx, base_font));
    }

    // State setting
    void font_size(NVGcontext* ctx, const float size)
    {
        NVGstate* state = nvg_getState(ctx);
        state->fontSize = size;
    }

    void font_blur(NVGcontext* ctx, const float blur)
    {
        NVGstate* state = nvg_getState(ctx);
        state->fontBlur = blur;
    }

    void text_letter_spacing(NVGcontext* ctx, const float spacing)
    {
        NVGstate* state = nvg_getState(ctx);
        state->letterSpacing = spacing;
    }

    void text_line_height(NVGcontext* ctx, const float line_height)
    {
        NVGstate* state = nvg_getState(ctx);
        state->lineHeight = line_height;
    }

    void text_align(NVGcontext* ctx, const int align)
    {
        NVGstate* state = nvg_getState(ctx);
        state->textAlign = align;
    }

    void font_face_id(NVGcontext* ctx, const int font)
    {
        NVGstate* state = nvg_getState(ctx);
        state->fontId = font;
    }

    void font_face(NVGcontext* ctx, const char* font)
    {
        NVGstate* state = nvg_getState(ctx);
        state->fontId = fonsGetFontByName(ctx->fs, font);
    }

    void font_face(NVGcontext* ctx, const std::string_view& font)
    {
        NVGstate* state{ nvg_getState(ctx) };
        state->fontId = fonsGetFontByName(ctx->fs, font.data());
    }

    static float nvg_quantize(const float a, const float d)
    {
        return static_cast<int>(a / d + 0.5f) * d;
    }

    static float nvg_getFontScale(const NVGstate* state)
    {
        return nvg_minf(nvg_quantize(nvg_getAverageScale(state->xform), 0.01f), 4.0f);
    }

    static void nvg_flushTextTexture(const NVGcontext* ctx)
    {
        int dirty[4] = { 0 };

        if (fonsValidateTexture(ctx->fs, dirty))
        {
            const int fontImage = ctx->fontImages[ctx->fontImageIdx];
            // Update texture
            if (fontImage != 0)
            {
                int iw, ih;
                const unsigned char* data = fonsGetTextureData(ctx->fs, &iw, &ih);
                const int x = dirty[0];
                const int y = dirty[1];
                const int w = dirty[2] - dirty[0];
                const int h = dirty[3] - dirty[1];
                ctx->params.renderUpdateTexture(ctx->params.userPtr, fontImage, x, y, w, h, data);
            }
        }
    }

    static int nvg_allocTextAtlas(NVGcontext* ctx)
    {
        int iw, ih;
        nvg_flushTextTexture(ctx);
        if (ctx->fontImageIdx >= NVG_MAX_FONTIMAGES - 1)
            return 0;
        // if next fontImage already have a texture
        if (ctx->fontImages[ctx->fontImageIdx + 1] != 0)
            image_size(ctx, ctx->fontImages[ctx->fontImageIdx + 1], &iw, &ih);
        else
        {
            // calculate the new font image size and create it.
            image_size(ctx, ctx->fontImages[ctx->fontImageIdx], &iw, &ih);
            if (iw > ih)
                ih *= 2;
            else
                iw *= 2;
            if (iw > NVG_MAX_FONTIMAGE_SIZE || ih > NVG_MAX_FONTIMAGE_SIZE)
                iw = ih = NVG_MAX_FONTIMAGE_SIZE;
            ctx->fontImages[ctx->fontImageIdx + 1] = ctx->params.renderCreateTexture(
                ctx->params.userPtr, NVG_TEXTURE_ALPHA, iw, ih, 0, nullptr);
        }
        ++ctx->fontImageIdx;
        fonsResetAtlas(ctx->fs, iw, ih);
        return 1;
    }

    static void nvg_renderText(NVGcontext* ctx, const NVGvertex* verts, const int nverts)
    {
        NVGstate* state = nvg_getState(ctx);
        NVGpaint paint = state->fill;

        // Render triangles.
        paint.image = ctx->fontImages[ctx->fontImageIdx];

        // Apply global alpha
        paint.innerColor.a *= state->alpha;
        paint.outerColor.a *= state->alpha;

        ctx->params.renderTriangles(ctx->params.userPtr, &paint, state->composite_operation,
                                    &state->scissor, verts, nverts, ctx->fringeWidth);

        ctx->drawCallCount++;
        ctx->textTriCount += nverts / 3;
    }

    static int nvg_isTransformFlipped(const float* xform)
    {
        const float det = xform[0] * xform[3] - xform[2] * xform[1];
        return det < 0;
    }

    float text(NVGcontext* ctx, float x, float y, const char* string, const char* end /*= nullptr*/)
    {
        NVGstate* state = nvg_getState(ctx);
        FONStextIter iter, prev_iter;
        FONSquad q;
        NVGvertex* verts;
        float scale = nvg_getFontScale(state) * ctx->devicePxRatio;
        float invscale = 1.0f / scale;
        int cverts = 0;
        int nverts = 0;
        int is_flipped = nvg_isTransformFlipped(state->xform);

        if (end == nullptr)
            end = string + strlen(string);

        if (state->fontId == FONS_INVALID)
            return x;

        fonsSetSize(ctx->fs, state->fontSize * scale);
        fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
        fonsSetBlur(ctx->fs, state->fontBlur * scale);
        fonsSetAlign(ctx->fs, state->textAlign);
        fonsSetFont(ctx->fs, state->fontId);

        cverts = nvg_maxi(2, static_cast<int>(end - string)) * 6;  // conservative estimate.
        verts = nvg_allocTempVerts(ctx, cverts);
        if (verts == nullptr)
            return x;

        fonsTextIterInit(ctx->fs, &iter, x * scale, y * scale, string, end,
                         FONS_GLYPH_BITMAP_REQUIRED);
        prev_iter = iter;
        while (fonsTextIterNext(ctx->fs, &iter, &q))
        {
            float c[4 * 2] = { 0 };
            if (iter.prevGlyphIndex == -1)
            {
                // can not retrieve glyph?
                if (nverts != 0)
                {
                    nvg_renderText(ctx, verts, nverts);
                    nverts = 0;
                }
                if (!nvg_allocTextAtlas(ctx))
                    break;  // no memory :(
                iter = prev_iter;
                fonsTextIterNext(ctx->fs, &iter, &q);  // try again
                if (iter.prevGlyphIndex == -1)         // still can not find glyph?
                    break;
            }
            prev_iter = iter;
            if (is_flipped)
            {
                float tmp;

                tmp = q.y0;
                q.y0 = q.y1;
                q.y1 = tmp;
                tmp = q.t0;
                q.t0 = q.t1;
                q.t1 = tmp;
            }
            // Transform corners.
            transform_point(&c[0], &c[1], state->xform, q.x0 * invscale, q.y0 * invscale);
            transform_point(&c[2], &c[3], state->xform, q.x1 * invscale, q.y0 * invscale);
            transform_point(&c[4], &c[5], state->xform, q.x1 * invscale, q.y1 * invscale);
            transform_point(&c[6], &c[7], state->xform, q.x0 * invscale, q.y1 * invscale);
            // Create triangles
            if (nverts + 6 <= cverts)
            {
                nvg_vset(&verts[nverts], c[0], c[1], q.s0, q.t0);
                nverts++;
                nvg_vset(&verts[nverts], c[4], c[5], q.s1, q.t1);
                nverts++;
                nvg_vset(&verts[nverts], c[2], c[3], q.s1, q.t0);
                nverts++;
                nvg_vset(&verts[nverts], c[0], c[1], q.s0, q.t0);
                nverts++;
                nvg_vset(&verts[nverts], c[6], c[7], q.s0, q.t1);
                nverts++;
                nvg_vset(&verts[nverts], c[4], c[5], q.s1, q.t1);
                nverts++;
            }
        }

        // TODO: add back-end bit to do this just once per frame.
        nvg_flushTextTexture(ctx);
        nvg_renderText(ctx, verts, nverts);

        return iter.nextx / scale;
    }

    void text_box(NVGcontext* ctx, const float x, float y, const float breakRowWidth,
                  const char* string, const char* end /*= nullptr*/)
    {
        NVGstate* state = nvg_getState(ctx);
        NVGtextRow rows[2];
        int nrows = 0;
        const int old_align = state->textAlign;
        const int haling = state->textAlign & (NVG_ALIGN_LEFT | NVG_ALIGN_CENTER | NVG_ALIGN_RIGHT);
        const int valign = state->textAlign & (NVG_ALIGN_TOP | NVG_ALIGN_MIDDLE | NVG_ALIGN_BOTTOM |
                                               NVG_ALIGN_BASELINE);
        float lineh = 0;

        if (state->fontId == FONS_INVALID)
            return;

        text_metrics(ctx, nullptr, nullptr, &lineh);

        state->textAlign = NVG_ALIGN_LEFT | valign;

        while ((nrows = text_break_lines(ctx, string, end, breakRowWidth, rows, 2)))
        {
            for (int i = 0; i < nrows; i++)
            {
                const NVGtextRow* row = &rows[i];
                if (haling & NVG_ALIGN_LEFT)
                    text(ctx, x, y, row->start, row->end);
                else if (haling & NVG_ALIGN_CENTER)
                    text(ctx, x + breakRowWidth * 0.5f - row->width * 0.5f, y, row->start, row->end);
                else if (haling & NVG_ALIGN_RIGHT)
                    text(ctx, x + breakRowWidth - row->width, y, row->start, row->end);
                y += lineh * state->lineHeight;
            }
            string = rows[nrows - 1].next;
        }

        state->textAlign = old_align;
    }

    int text_glyph_positions(NVGcontext* ctx, float x, float y, const char* string, const char* end,
                             NVGglyphPosition* positions, int max_positions)
    {
        NVGstate* state = nvg_getState(ctx);
        float scale = nvg_getFontScale(state) * ctx->devicePxRatio;
        float invscale = 1.0f / scale;
        FONStextIter iter, prev_iter;
        FONSquad q;
        int npos = 0;

        if (state->fontId == FONS_INVALID)
            return 0;

        if (end == nullptr)
            end = string + strlen(string);

        if (string == end)
            return 0;

        fonsSetSize(ctx->fs, state->fontSize * scale);
        fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
        fonsSetBlur(ctx->fs, state->fontBlur * scale);
        fonsSetAlign(ctx->fs, state->textAlign);
        fonsSetFont(ctx->fs, state->fontId);

        fonsTextIterInit(ctx->fs, &iter, x * scale, y * scale, string, end,
                         FONS_GLYPH_BITMAP_OPTIONAL);
        prev_iter = iter;
        while (fonsTextIterNext(ctx->fs, &iter, &q))
        {
            if (iter.prevGlyphIndex < 0 && nvg_allocTextAtlas(ctx))
            {
                // can not retrieve glyph?
                iter = prev_iter;
                fonsTextIterNext(ctx->fs, &iter, &q);  // try again
            }
            prev_iter = iter;
            positions[npos].str = iter.str;
            positions[npos].x = iter.x * invscale;
            positions[npos].minx = nvg_minf(iter.x, q.x0) * invscale;
            positions[npos].maxx = nvg_maxf(iter.nextx, q.x1) * invscale;
            npos++;
            if (npos >= max_positions)
                break;
        }

        return npos;
    }

    enum NVGcodepointType {
        Space,
        Newline,
        Char,
        CJK_Char,
    };

    int text_break_lines(NVGcontext* ctx, const char* string, const char* end,
                         float break_row_width, NVGtextRow* rows, int max_rows)
    {
        NVGstate* state = nvg_getState(ctx);
        float scale = nvg_getFontScale(state) * ctx->devicePxRatio;
        float invscale = 1.0f / scale;
        FONStextIter iter;
        FONStextIter prev_iter;
        FONSquad q;
        int nrows = 0;
        float row_start_x = 0;
        float row_width = 0;
        float row_min_x = 0;
        float row_max_x = 0;
        const char* row_start = nullptr;
        const char* row_end = nullptr;
        const char* word_start = nullptr;
        float word_start_x = 0;
        float word_min_x = 0;
        const char* break_end = nullptr;
        float break_width = 0;
        float break_max_x = 0;
        int type = Space;
        int ptype = Space;
        unsigned int pcodepoint = 0;

        if (max_rows == 0)
            return 0;
        if (state->fontId == FONS_INVALID)
            return 0;

        if (end == nullptr)
            end = string + strlen(string);

        if (string == end)
            return 0;

        fonsSetSize(ctx->fs, state->fontSize * scale);
        fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
        fonsSetBlur(ctx->fs, state->fontBlur * scale);
        fonsSetAlign(ctx->fs, state->textAlign);
        fonsSetFont(ctx->fs, state->fontId);

        break_row_width *= scale;

        fonsTextIterInit(ctx->fs, &iter, 0, 0, string, end, FONS_GLYPH_BITMAP_OPTIONAL);
        prev_iter = iter;
        while (fonsTextIterNext(ctx->fs, &iter, &q))
        {
            if (iter.prevGlyphIndex < 0 && nvg_allocTextAtlas(ctx))
            {
                // can not retrieve glyph?
                iter = prev_iter;
                fonsTextIterNext(ctx->fs, &iter, &q);  // try again
            }
            prev_iter = iter;
            switch (iter.codepoint)
            {
                case 9:       // \t
                case 11:      // \v
                case 12:      // \f
                case 32:      // space
                case 0x00a0:  // NBSP
                    type = Space;
                    break;
                case 10:  // \n
                    type = pcodepoint == 13 ? Space : Newline;
                    break;
                case 13:  // \r
                    type = pcodepoint == 10 ? Space : Newline;
                    break;
                case 0x0085:  // NEL
                    type = Newline;
                    break;
                default:
                    if ((iter.codepoint >= 0x4E00 && iter.codepoint <= 0x9FFF) ||
                        (iter.codepoint >= 0x3000 && iter.codepoint <= 0x30FF) ||
                        (iter.codepoint >= 0xFF00 && iter.codepoint <= 0xFFEF) ||
                        (iter.codepoint >= 0x1100 && iter.codepoint <= 0x11FF) ||
                        (iter.codepoint >= 0x3130 && iter.codepoint <= 0x318F) ||
                        (iter.codepoint >= 0xAC00 && iter.codepoint <= 0xD7AF))
                        type = CJK_Char;
                    else
                        type = Char;
                    break;
            }

            if (type == Newline)
            {
                // Always handle new lines.
                rows[nrows].start = row_start != nullptr ? row_start : iter.str;
                rows[nrows].end = row_end != nullptr ? row_end : iter.str;
                rows[nrows].width = row_width * invscale;
                rows[nrows].minx = row_min_x * invscale;
                rows[nrows].maxx = row_max_x * invscale;
                rows[nrows].next = iter.next;
                nrows++;
                if (nrows >= max_rows)
                    return nrows;
                // Set null break point
                break_end = row_start;
                break_width = 0.0;
                break_max_x = 0.0;
                // Indicate to skip the white space at the beginning of the row.
                row_start = nullptr;
                row_end = nullptr;
                row_width = 0;
                row_min_x = row_max_x = 0;
            }
            else if (row_start == nullptr)
            {
                // Skip white space until the beginning of the line
                if (type == Char || type == CJK_Char)
                {
                    // The current char is the row so far
                    row_start_x = iter.x;
                    row_start = iter.str;
                    row_end = iter.next;
                    row_width = iter.nextx - row_start_x;
                    row_min_x = q.x0 - row_start_x;
                    row_max_x = q.x1 - row_start_x;
                    word_start = iter.str;
                    word_start_x = iter.x;
                    word_min_x = q.x0 - row_start_x;
                    // Set null break point
                    break_end = row_start;
                    break_width = 0.0;
                    break_max_x = 0.0;
                }
            }
            else
            {
                float nextWidth = iter.nextx - row_start_x;

                // track last non-white space character
                if (type == Char || type == CJK_Char)
                {
                    row_end = iter.next;
                    row_width = iter.nextx - row_start_x;
                    row_max_x = q.x1 - row_start_x;
                }
                // track last end of a word
                if (((ptype == Char || ptype == CJK_Char) && type == Space) || type == CJK_Char)
                {
                    break_end = iter.str;
                    break_width = row_width;
                    break_max_x = row_max_x;
                }
                // track last beginning of a word
                if ((ptype == Space && (type == Char || type == CJK_Char)) || type == CJK_Char)
                {
                    word_start = iter.str;
                    word_start_x = iter.x;
                    word_min_x = q.x0;
                }

                // Break to new line when a character is beyond break width.
                if ((type == Char || type == CJK_Char) && nextWidth > break_row_width)
                {
                    // The run length is too long, need to break to new line.
                    if (break_end == row_start)
                    {
                        // The current word is longer than the row length, just break it from here.
                        rows[nrows].start = row_start;
                        rows[nrows].end = iter.str;
                        rows[nrows].width = row_width * invscale;
                        rows[nrows].minx = row_min_x * invscale;
                        rows[nrows].maxx = row_max_x * invscale;
                        rows[nrows].next = iter.str;
                        nrows++;
                        if (nrows >= max_rows)
                            return nrows;
                        row_start_x = iter.x;
                        row_start = iter.str;
                        row_end = iter.next;
                        row_width = iter.nextx - row_start_x;
                        row_min_x = q.x0 - row_start_x;
                        row_max_x = q.x1 - row_start_x;
                        word_start = iter.str;
                        word_start_x = iter.x;
                        word_min_x = q.x0 - row_start_x;
                    }
                    else
                    {
                        // Break the line from the end of the last word, and start new line from the
                        // beginning of the new.
                        rows[nrows].start = row_start;
                        rows[nrows].end = break_end;
                        rows[nrows].width = break_width * invscale;
                        rows[nrows].minx = row_min_x * invscale;
                        rows[nrows].maxx = break_max_x * invscale;
                        rows[nrows].next = word_start;
                        nrows++;
                        if (nrows >= max_rows)
                            return nrows;
                        // Update row
                        row_start_x = word_start_x;
                        row_start = word_start;
                        row_end = iter.next;
                        row_width = iter.nextx - row_start_x;
                        row_min_x = word_min_x - row_start_x;
                        row_max_x = q.x1 - row_start_x;
                    }
                    // Set null break point
                    break_end = row_start;
                    break_width = 0.0;
                    break_max_x = 0.0;
                }
            }

            pcodepoint = iter.codepoint;
            ptype = type;
        }

        // Break the line from the end of the last word, and start new line from the beginning of
        // the new.
        if (row_start != nullptr)
        {
            rows[nrows].start = row_start;
            rows[nrows].end = row_end;
            rows[nrows].width = row_width * invscale;
            rows[nrows].minx = row_min_x * invscale;
            rows[nrows].maxx = row_max_x * invscale;
            rows[nrows].next = end;
            nrows++;
        }

        return nrows;
    }

    float text_bounds(NVGcontext* ctx, const float x, const float y, const char* string,
                      const char* end /*= nullptr*/, float* bounds /*= nullptr*/)
    {
        const NVGstate* state = nvg_getState(ctx);
        const float scale = nvg_getFontScale(state) * ctx->devicePxRatio;
        const float invscale = 1.0f / scale;

        if (state->fontId == FONS_INVALID)
            return 0;

        fonsSetSize(ctx->fs, state->fontSize * scale);
        fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
        fonsSetBlur(ctx->fs, state->fontBlur * scale);
        fonsSetAlign(ctx->fs, state->textAlign);
        fonsSetFont(ctx->fs, state->fontId);

        const float width = fonsTextBounds(ctx->fs, x * scale, y * scale, string, end, bounds);
        if (bounds != nullptr)
        {
            // Use line bounds for height.
            fonsLineBounds(ctx->fs, y * scale, &bounds[1], &bounds[3]);
            bounds[0] *= invscale;
            bounds[1] *= invscale;
            bounds[2] *= invscale;
            bounds[3] *= invscale;
        }
        return width * invscale;
    }

    void text_box_bounds(NVGcontext* ctx, const float x, float y, const float breakRowWidth,
                         const char* string, const char* end, float* bounds)
    {
        NVGstate* state = nvg_getState(ctx);
        NVGtextRow rows[2];
        const float scale = nvg_getFontScale(state) * ctx->devicePxRatio;
        const float invscale = 1.0f / scale;
        int nrows = 0;
        const int oldAlign = state->textAlign;
        const int haling = state->textAlign & (NVG_ALIGN_LEFT | NVG_ALIGN_CENTER | NVG_ALIGN_RIGHT);
        const int valign = state->textAlign & (NVG_ALIGN_TOP | NVG_ALIGN_MIDDLE | NVG_ALIGN_BOTTOM |
                                               NVG_ALIGN_BASELINE);
        float lineh = 0, rminy = 0, rmaxy = 0;
        float maxx, maxy;

        if (state->fontId == FONS_INVALID)
        {
            if (bounds != nullptr)
                bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0f;
            return;
        }

        text_metrics(ctx, nullptr, nullptr, &lineh);

        state->textAlign = NVG_ALIGN_LEFT | valign;

        float minx = maxx = x;
        float miny = maxy = y;

        fonsSetSize(ctx->fs, state->fontSize * scale);
        fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
        fonsSetBlur(ctx->fs, state->fontBlur * scale);
        fonsSetAlign(ctx->fs, state->textAlign);
        fonsSetFont(ctx->fs, state->fontId);
        fonsLineBounds(ctx->fs, 0, &rminy, &rmaxy);
        rminy *= invscale;
        rmaxy *= invscale;

        while ((nrows = text_break_lines(ctx, string, end, breakRowWidth, rows, 2)))
        {
            for (int i = 0; i < nrows; i++)
            {
                const NVGtextRow* row = &rows[i];
                float dx = 0;

                // Horizontal bounds
                if (haling & NVG_ALIGN_LEFT)
                    dx = 0;
                else if (haling & NVG_ALIGN_CENTER)
                    dx = breakRowWidth * 0.5f - row->width * 0.5f;
                else if (haling & NVG_ALIGN_RIGHT)
                    dx = breakRowWidth - row->width;
                const float rminx = x + row->minx + dx;
                const float rmaxx = x + row->maxx + dx;
                minx = nvg_minf(minx, rminx);
                maxx = nvg_maxf(maxx, rmaxx);

                // Vertical bounds.
                miny = nvg_minf(miny, y + rminy);
                maxy = nvg_maxf(maxy, y + rmaxy);

                y += lineh * state->lineHeight;
            }

            string = rows[nrows - 1].next;
        }

        state->textAlign = oldAlign;

        if (bounds != nullptr)
        {
            bounds[0] = minx;
            bounds[1] = miny;
            bounds[2] = maxx;
            bounds[3] = maxy;
        }
    }

    void text_metrics(NVGcontext* ctx, float* ascender, float* descender, float* lineh)
    {
        const NVGstate* state = nvg_getState(ctx);
        const float scale = nvg_getFontScale(state) * ctx->devicePxRatio;
        const float invscale = 1.0f / scale;

        if (state->fontId == FONS_INVALID)
            return;

        fonsSetSize(ctx->fs, state->fontSize * scale);
        fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
        fonsSetBlur(ctx->fs, state->fontBlur * scale);
        fonsSetAlign(ctx->fs, state->textAlign);
        fonsSetFont(ctx->fs, state->fontId);

        fonsVertMetrics(ctx->fs, ascender, descender, lineh);
        if (ascender != nullptr)
            *ascender *= invscale;
        if (descender != nullptr)
            *descender *= invscale;
        if (lineh != nullptr)
            *lineh *= invscale;
    }
}
