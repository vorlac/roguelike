#include <array>
#include <cstdlib>
#include <numbers>
#include <print>

#include "ds/color.hpp"
#include "ds/rect.hpp"
#include "graphics/stb/stb_image.hpp"
#include "graphics/vg/nanovg.hpp"

#ifdef _MSC_VER
  #pragma warning(disable : 4100)  // unreferenced formal parameter
  #pragma warning(disable : 4706)  // assignment within conditional expression
  #pragma warning(disable : 4267)
#endif

namespace rl::nvg {

    enum {
        NvgInitCommandsSize = 256,
        NvgInitPointsSize = 128,
        NvgInitPathsSize = 16,
        NvgInitVertsSize = 256
    };

    enum NVGcodepointType {
        Space,
        Newline,
        Char,
        CJK_Char,
    };

    enum NVGcommands {
        MoveTo = 0,
        LineTo = 1,
        Bezierto = 2,
        Close = 3,
        Winding = 4,
    };

    enum NVGpointFlags {
        NvgPtCorner = 0x01,
        NvgPtLeft = 0x02,
        NvgPtBevel = 0x04,
        NvgPrInnerbevel = 0x08,
    };

    // struct NVGcontext
    //{
    //     NVGparams params;
    //     float* commands;
    //     int32_t ccommands;
    //     int32_t ncommands;
    //     float commandx;
    //     float commandy;
    //     NVGstate states[NVG_MAX_STATES];
    //     int32_t nstates;
    //     NVGpathCache* cache;
    //     float tess_tol;
    //     float dist_tol;
    //     float fringe_width;
    //     float device_px_ratio;
    //     FONScontext* fs;
    //     int32_t font_images[NvgMaxFontimages];
    //     int32_t font_image_idx;
    //     int32_t draw_call_count;
    //     int32_t fill_tri_count;
    //     int32_t stroke_tri_count;
    //     int32_t text_tri_count;
    // };

    namespace detail {
        static float nvg_sqrtf(const float a)
        {
            return std::sqrtf(a);
        }

        static float nvg_modf(const float a, const float b)
        {
            return std::fmodf(a, b);
        }

        static float nvg_sinf(const float a)
        {
            return std::sinf(a);
        }

        static float nvg_cosf(const float a)
        {
            return std::cosf(a);
        }

        static float nvg_tanf(const float a)
        {
            return std::tanf(a);
        }

        // ReSharper disable once CppInconsistentNaming
        static float nvg_atan2f(const float a, const float b)
        {
            return std::atan2f(a, b);
        }

        static float nvg_acosf(const float a)
        {
            return std::acosf(a);
        }

        template <typename T>
        constexpr static auto nvg_min(const T a, const T b)
        {
            return a < b ? a : b;
        }

        template <typename T>
        constexpr static auto nvg_max(const T a, const T b)
        {
            return a > b ? a : b;
        }

        template <typename T>
        constexpr static int32_t nvg_clampi(const T a, const T mn, const T mx)
        {
            return a < mn ? mn : (a > mx ? mx : a);
        }

        constexpr static float nvg_absf(const float a)
        {
            return a >= 0.0f ? a : -a;
        }

        constexpr static float nvg_signf(const float a)
        {
            return a >= 0.0f ? 1.0f : -1.0f;
        }

        constexpr static float nvg_clampf(const float a, const float mn, const float mx)
        {
            return a < mn ? mn : (a > mx ? mx : a);
        }

        constexpr static float nvg_cross(const float dx0, const float dy0, const float dx1,
                                         const float dy1)
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

        static void nvg_delete_path_cache(NVGpathCache* c)
        {
            if (c == nullptr)
                return;
            if (c->points != nullptr)
                std::free(c->points);
            if (c->paths != nullptr)
                std::free(c->paths);
            if (c->verts != nullptr)
                std::free(c->verts);

            std::free(c);
        }

        static NVGpathCache* nvg_alloc_path_cache()
        {
            const auto c = static_cast<NVGpathCache*>(std::malloc(sizeof(NVGpathCache)));
            if (c != nullptr)
            {
                std::memset(c, 0, sizeof(NVGpathCache));
                c->points = static_cast<NVGpoint*>(
                    std::malloc(sizeof(NVGpoint) * NvgInitPointsSize));

                if (c->points != nullptr)
                {
                    c->npoints = 0;
                    c->cpoints = NvgInitPointsSize;
                    c->paths = static_cast<NVGpath*>(
                        std::malloc(sizeof(NVGpath) * NvgInitPathsSize));

                    if (c->paths != nullptr)
                    {
                        c->npaths = 0;
                        c->cpaths = NvgInitPathsSize;
                        c->verts = static_cast<NVGvertex*>(
                            std::malloc(sizeof(NVGvertex) * NvgInitVertsSize));

                        if (c->verts != nullptr)
                        {
                            c->nverts = 0;
                            c->cverts = NvgInitVertsSize;
                            return c;
                        }
                    }
                }
            }

            // error
            nvg_delete_path_cache(c);
            return nullptr;
        }

        constexpr static void nvg_set_device_pixel_ratio(NVGcontext* ctx, const float ratio)
        {
            ctx->tess_tol = 0.25f / ratio;
            ctx->dist_tol = 0.01f / ratio;
            ctx->fringe_width = 1.0f / ratio;
            ctx->device_px_ratio = ratio;
        }

        constexpr static NVGcompositeOperationState nvg_composite_operation_state(const int32_t op)
        {
            int32_t sfactor;
            int32_t dfactor;

            if (op == NVGcompositeOperation::NVGSourceOver)
            {
                sfactor = NVGOne;
                dfactor = NVGOneMinusSrcAlpha;
            }
            else if (op == NVGcompositeOperation::NVGSourceIn)
            {
                sfactor = NVGDstAlpha;
                dfactor = NVGZero;
            }
            else if (op == NVGcompositeOperation::NVGSourceOut)
            {
                sfactor = NVGOneMinusDstAlpha;
                dfactor = NVGZero;
            }
            else if (op == NVGcompositeOperation::NVGAtop)
            {
                sfactor = NVGDstAlpha;
                dfactor = NVGOneMinusSrcAlpha;
            }
            else if (op == NVGcompositeOperation::NVGDestinationOver)
            {
                sfactor = NVGOneMinusDstAlpha;
                dfactor = NVGOne;
            }
            else if (op == NVGcompositeOperation::NVGDestinationIn)
            {
                sfactor = NVGZero;
                dfactor = NVGSrcAlpha;
            }
            else if (op == NVGcompositeOperation::NVGDestinationOut)
            {
                sfactor = NVGZero;
                dfactor = NVGOneMinusSrcAlpha;
            }
            else if (op == NVGcompositeOperation::NVGDestinationAtop)
            {
                sfactor = NVGOneMinusDstAlpha;
                dfactor = NVGSrcAlpha;
            }
            else if (op == NVGcompositeOperation::NVGLighter)
            {
                sfactor = NVGOne;
                dfactor = NVGOne;
            }
            else if (op == NVGcompositeOperation::NVGCopy)
            {
                sfactor = NVGOne;
                dfactor = NVGZero;
            }
            else if (op == NVGcompositeOperation::NVGXor)
            {
                sfactor = NVGOneMinusDstAlpha;
                dfactor = NVGOneMinusSrcAlpha;
            }
            else
            {
                sfactor = NVGOne;
                dfactor = NVGZero;
            }

            const NVGcompositeOperationState state{
                .src_rgb = sfactor,
                .dst_rgb = dfactor,
                .src_alpha = sfactor,
                .dst_alpha = dfactor,
            };

            return state;
        }

        static NVGstate* nvg_get_state(NVGcontext* ctx)
        {
            return &ctx->states[ctx->nstates - 1];
        }

        static void nvg_clear_path_cache(const NVGcontext* ctx)
        {
            ctx->cache->npoints = 0;
            ctx->cache->npaths = 0;
        }

        static NVGpath* nvg_last_path(const NVGcontext* ctx)
        {
            if (ctx->cache->npaths > 0)
                return &ctx->cache->paths[ctx->cache->npaths - 1];
            return nullptr;
        }

        static void nvg_add_path(const NVGcontext* ctx)
        {
            if (ctx->cache->npaths + 1 > ctx->cache->cpaths)
            {
                const int32_t cpaths = ctx->cache->npaths + 1 + (ctx->cache->cpaths / 2);
                const auto paths = static_cast<NVGpath*>(
                    realloc(ctx->cache->paths, sizeof(NVGpath) * static_cast<uint64_t>(cpaths)));
                if (paths == nullptr)
                    return;
                ctx->cache->paths = paths;
                ctx->cache->cpaths = cpaths;
            }
            NVGpath* path = &ctx->cache->paths[ctx->cache->npaths];
            memset(path, 0, sizeof(*path));
            path->first = ctx->cache->npoints;
            path->winding = NVGccw;

            ctx->cache->npaths++;
        }

        static NVGpoint* nvg_last_point(const NVGcontext* ctx)
        {
            if (ctx->cache->npoints > 0)
                return &ctx->cache->points[ctx->cache->npoints - 1];
            return nullptr;
        }

        static int32_t nvg_pt_equals(const float x1, const float y1, const float x2, const float y2,
                                     const float tol)
        {
            const float dx = x2 - x1;
            const float dy = y2 - y1;
            return dx * dx + dy * dy < tol * tol;
        }

        static void nvg_add_point(const NVGcontext* ctx, const float x, const float y,
                                  const int32_t flags)
        {
            NVGpath* path = nvg_last_path(ctx);
            NVGpoint* pt;
            if (path == nullptr)
                return;

            if (path->count > 0 && ctx->cache->npoints > 0)
            {
                pt = nvg_last_point(ctx);
                if (nvg_pt_equals(pt->x, pt->y, x, y, ctx->dist_tol))
                {
                    pt->flags |= flags;
                    return;
                }
            }

            if (ctx->cache->npoints + 1 > ctx->cache->cpoints)
            {
                const int32_t cpoints = ctx->cache->npoints + 1 + ctx->cache->cpoints / 2;
                const auto points = static_cast<NVGpoint*>(
                    realloc(ctx->cache->points, sizeof(NVGpoint) * static_cast<uint64_t>(cpoints)));
                if (points == nullptr)
                    return;
                ctx->cache->points = points;
                ctx->cache->cpoints = cpoints;
            }

            pt = &ctx->cache->points[ctx->cache->npoints];
            memset(pt, 0, sizeof(*pt));
            pt->x = x;
            pt->y = y;
            pt->flags = static_cast<uint8_t>(flags);

            ctx->cache->npoints++;
            path->count++;
        }

        static void nvg_close_path(const NVGcontext* ctx)
        {
            NVGpath* path = nvg_last_path(ctx);
            if (path == nullptr)
                return;
            path->closed = 1;
        }

        static void nvg_path_winding(const NVGcontext* ctx, const int32_t winding)
        {
            NVGpath* path = nvg_last_path(ctx);
            if (path == nullptr)
                return;
            path->winding = winding;
        }

        static float nvg_get_average_scale(const float* t)
        {
            const float sx = sqrtf(t[0] * t[0] + t[2] * t[2]);
            const float sy = sqrtf(t[1] * t[1] + t[3] * t[3]);
            return (sx + sy) * 0.5f;
        }

        static NVGvertex* nvg_alloc_temp_verts(const NVGcontext* ctx, const int32_t nverts)
        {
            if (nverts > ctx->cache->cverts)
            {
                const int32_t cverts = (nverts + 0xff) & ~0xff;  // Round up to prevent allocations
                                                                 // when
                // things change just slightly.
                const auto verts = static_cast<NVGvertex*>(std::realloc(
                    ctx->cache->verts, sizeof(NVGvertex) * static_cast<uint64_t>(cverts)));
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

        static float nvg_poly_area(const NVGpoint* pts, const int32_t npts)
        {
            float area = 0;
            for (int32_t i = 2; i < npts; i++)
            {
                const NVGpoint* a = &pts[0];
                const NVGpoint* b = &pts[i - 1];
                const NVGpoint* c = &pts[i];
                area += nvg_triarea2(a->x, a->y, b->x, b->y, c->x, c->y);
            }
            return area * 0.5f;
        }

        static void nvg_poly_reverse(NVGpoint* pts, const int32_t npts)
        {
            int32_t i = 0, j = npts - 1;
            while (i < j)
            {
                const NVGpoint tmp = pts[i];
                pts[i] = pts[j];
                pts[j] = tmp;
                i++;
                j--;
            }
        }

        static void nvg_vset(NVGvertex* vtx, const float x, const float y, const float u,
                             const float v)
        {
            vtx->x = x;
            vtx->y = y;
            vtx->u = u;
            vtx->v = v;
        }

        static void nvg_tesselate_bezier(NVGcontext* ctx, const float x1, const float y1,
                                         const float x2, const float y2, const float x3,
                                         const float y3, const float x4, const float y4,
                                         const int32_t level, const int32_t type)
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

            if ((d2 + d3) * (d2 + d3) < ctx->tess_tol * (dx * dx + dy * dy))
            {
                nvg_add_point(ctx, x4, y4, type);
                return;
            }

            /*	if (nvg__absf(x1+x3-x2-x2) + nvg__absf(y1+y3-y2-y2) + nvg__absf(x2+x4-x3-x3) +
               nvg__absf(y2+y4-y3-y3) < ctx->tessTol) { nvg__addPoint(ctx, x4, y4, type); return;
                }*/

            const float x234 = (x23 + x34) * 0.5f;
            const float y234 = (y23 + y34) * 0.5f;
            const float x1234 = (x123 + x234) * 0.5f;
            const float y1234 = (y123 + y234) * 0.5f;

            nvg_tesselate_bezier(ctx, x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1, 0);
            nvg_tesselate_bezier(ctx, x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1, type);
        }

        static void nvg_flatten_paths(NVGcontext* ctx)
        {
            NVGpathCache* cache = ctx->cache;
            //	NVGstate* state = nvg__getState(ctx);
            NVGpoint* last;
            NVGpoint* pts;
            float* p;

            if (cache->npaths > 0)
                return;

            // Flatten
            int32_t i = 0;
            while (i < ctx->ncommands)
            {
                const auto cmd = static_cast<NVGcommands>(ctx->commands[i]);
                switch (cmd)
                {
                    case MoveTo:
                        nvg_add_path(ctx);
                        p = &ctx->commands[i + 1];
                        nvg_add_point(ctx, p[0], p[1], NvgPtCorner);
                        i += 3;
                        break;
                    case LineTo:
                        p = &ctx->commands[i + 1];
                        nvg_add_point(ctx, p[0], p[1], NvgPtCorner);
                        i += 3;
                        break;
                    case Bezierto:
                        last = nvg_last_point(ctx);
                        if (last != nullptr)
                        {
                            const float* cp1 = &ctx->commands[i + 1];
                            const float* cp2 = &ctx->commands[i + 3];
                            p = &ctx->commands[i + 5];
                            nvg_tesselate_bezier(ctx, last->x, last->y, cp1[0], cp1[1], cp2[0],
                                                 cp2[1], p[0], p[1], 0, NvgPtCorner);
                        }
                        i += 7;
                        break;
                    case Close:
                        nvg_close_path(ctx);
                        i++;
                        break;
                    case Winding:
                        nvg_path_winding(ctx, static_cast<int32_t>(ctx->commands[i + 1]));
                        i += 2;
                        break;
                    default:
                        i++;
                }
            }

            cache->bounds[0] = cache->bounds[1] = 1e6f;
            cache->bounds[2] = cache->bounds[3] = -1e6f;

            // Calculate the direction and length of line segments.
            for (int32_t j = 0; j < cache->npaths; j++)
            {
                NVGpath* path = &cache->paths[j];
                pts = &cache->points[path->first];

                // If the first and last points are the same, remove the last, mark as closed path.
                NVGpoint* p0 = &pts[path->count - 1];
                NVGpoint* p1 = &pts[0];
                if (nvg_pt_equals(p0->x, p0->y, p1->x, p1->y, ctx->dist_tol))
                {
                    path->count--;
                    p0 = &pts[path->count - 1];
                    path->closed = 1;
                }

                // Enforce winding.
                if (path->count > 2)
                {
                    const float area = nvg_poly_area(pts, path->count);
                    if (path->winding == NVGccw && area < 0.0f)
                        nvg_poly_reverse(pts, path->count);
                    if (path->winding == NVGcw && area > 0.0f)
                        nvg_poly_reverse(pts, path->count);
                }

                for (i = 0; i < path->count; i++)
                {
                    // Calculate segment direction and length
                    p0->dx = p1->x - p0->x;
                    p0->dy = p1->y - p0->y;
                    p0->len = nvg_normalize(&p0->dx, &p0->dy);
                    // Update bounds
                    cache->bounds[0] = nvg_min(cache->bounds[0], p0->x);
                    cache->bounds[1] = nvg_min(cache->bounds[1], p0->y);
                    cache->bounds[2] = nvg_max(cache->bounds[2], p0->x);
                    cache->bounds[3] = nvg_max(cache->bounds[3], p0->y);
                    // Advance
                    p0 = p1++;
                }
            }
        }

        static int32_t nvg_curve_divs(const float r, const float arc, const float tol)
        {
            const float da = acosf(r / (r + tol)) * 2.0f;
            return nvg_max(2, static_cast<int32_t>(ceilf(arc / da)));
        }

        static void nvg_choose_bevel(const int32_t bevel, const NVGpoint* p0, const NVGpoint* p1,
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

        static NVGvertex* nvg_round_join(NVGvertex* dst, const NVGpoint* p0, const NVGpoint* p1,
                                         const float lw, const float rw, const float lu,
                                         const float ru, const int32_t ncap, float fringe)
        {
            int32_t i, n;
            const float dlx0 = p0->dy;
            const float dly0 = -p0->dx;
            const float dlx1 = p1->dy;
            const float dly1 = -p1->dx;

            if (p1->flags & NvgPtLeft)
            {
                float lx0, ly0, lx1, ly1;
                nvg_choose_bevel(p1->flags & NvgPrInnerbevel, p0, p1, lw, &lx0, &ly0, &lx1, &ly1);
                const float a0 = atan2f(-dly0, -dlx0);
                float a1 = atan2f(-dly1, -dlx1);
                if (a1 > a0)
                    a1 -= std::numbers::pi_v<f32> * 2;

                nvg_vset(dst, lx0, ly0, lu, 1);
                dst++;
                nvg_vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
                dst++;

                n = nvg_clampi(static_cast<int32_t>(ceilf(
                                   (a0 - a1) / std::numbers::pi_v<f32> * static_cast<float>(ncap))),
                               2, ncap);

                for (i = 0; i < n; i++)
                {
                    const float u = static_cast<float>(i) / (static_cast<float>(n) - 1.0f);
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
                nvg_choose_bevel(p1->flags & NvgPrInnerbevel, p0, p1, -rw, &rx0, &ry0, &rx1, &ry1);
                const float a0 = atan2f(dly0, dlx0);
                float a1 = atan2f(dly1, dlx1);
                if (a1 < a0)
                    a1 += std::numbers::pi_v<f32> * 2;

                nvg_vset(dst, p1->x + dlx0 * rw, p1->y + dly0 * rw, lu, 1);
                dst++;
                nvg_vset(dst, rx0, ry0, ru, 1);
                dst++;

                n = std::clamp(static_cast<int32_t>(std::ceil(
                                   (a1 - a0) / std::numbers::pi_v<f32> * static_cast<float>(ncap))),
                               2, ncap);

                for (i = 0; i < n; i++)
                {
                    const float u = static_cast<float>(i) / (static_cast<float>(n) - 1.0f);
                    const float a = a0 + u * (a1 - a0);
                    const float lx = p1->x + std::cosf(a) * lw;
                    const float ly = p1->y + std::sinf(a) * lw;

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

        static NVGvertex* nvg_bevel_join(NVGvertex* dst, const NVGpoint* p0, const NVGpoint* p1,
                                         const float lw, const float rw, const float lu,
                                         const float ru, float fringe)
        {
            float rx0, ry0, rx1, ry1;
            float lx0, ly0, lx1, ly1;
            const float dlx0 = p0->dy;
            const float dly0 = -p0->dx;
            const float dlx1 = p1->dy;
            const float dly1 = -p1->dx;

            if (p1->flags & NvgPtLeft)
            {
                nvg_choose_bevel(p1->flags & NvgPrInnerbevel, p0, p1, lw, &lx0, &ly0, &lx1, &ly1);

                nvg_vset(dst, lx0, ly0, lu, 1);
                dst++;
                nvg_vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
                dst++;

                if (p1->flags & NvgPtBevel)
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
                nvg_choose_bevel(p1->flags & NvgPrInnerbevel, p0, p1, -rw, &rx0, &ry0, &rx1, &ry1);

                nvg_vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1);
                dst++;
                nvg_vset(dst, rx0, ry0, ru, 1);
                dst++;

                if (p1->flags & NvgPtBevel)
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

        static NVGvertex* nvg_butt_cap_start(NVGvertex* dst, const NVGpoint* p, const float dx,
                                             const float dy, const float w, const float d,
                                             const float aa, const float u0, const float u1)
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

        static NVGvertex* nvg_butt_cap_end(NVGvertex* dst, const NVGpoint* p, const float dx,
                                           const float dy, const float w, const float d,
                                           const float aa, const float u0, const float u1)
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

        static NVGvertex* nvg_round_cap_start(NVGvertex* dst, const NVGpoint* p, const float dx,
                                              const float dy, const float w, const int32_t ncap,
                                              float aa, const float u0, const float u1)
        {
            const float px = p->x;
            const float py = p->y;
            const float dlx = dy;
            const float dly = -dx;

            for (int32_t i = 0; i < ncap; i++)
            {
                const float a = static_cast<float>(i) / (static_cast<float>(ncap) - 1.0f) *
                                std::numbers::pi_v<f32>;
                const float ax = cosf(a) * w;
                const float ay = sinf(a) * w;

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

        static NVGvertex* nvg_round_cap_end(NVGvertex* dst, const NVGpoint* p, const float dx,
                                            const float dy, const float w, const int32_t ncap,
                                            float aa, const float u0, const float u1)
        {
            const float px = p->x;
            const float py = p->y;
            const float dlx = dy;
            const float dly = -dx;
            nvg_vset(dst, px + dlx * w, py + dly * w, u0, 1);
            dst++;
            nvg_vset(dst, px - dlx * w, py - dly * w, u1, 1);
            dst++;
            for (int32_t i = 0; i < ncap; i++)
            {
                const float a = static_cast<float>(i) / static_cast<float>(ncap - 1) *
                                std::numbers::pi_v<f32>;
                const float ax = cosf(a) * w, ay = sinf(a) * w;
                nvg_vset(dst, px, py, 0.5f, 1);
                dst++;
                nvg_vset(dst, px - dlx * ax + dx * ay, py - dly * ax + dy * ay, u0, 1);
                dst++;
            }
            return dst;
        }

        static void nvg_calculate_joins(const NVGcontext* ctx, const float w,
                                        const int32_t line_join, const float miter_limit)
        {
            const NVGpathCache* cache = ctx->cache;
            float iw = 0.0f;

            if (w > 0.0f)
                iw = 1.0f / w;

            // Calculate which joins needs extra vertices to append, and gather vertex count.
            for (int32_t i = 0; i < cache->npaths; i++)
            {
                NVGpath* path = &cache->paths[i];
                NVGpoint* pts = &cache->points[path->first];
                const NVGpoint* p0 = &pts[path->count - 1];
                NVGpoint* p1 = &pts[0];
                int32_t nleft = 0;

                path->nbevel = 0;

                for (int32_t j = 0; j < path->count; j++)
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
                    p1->flags = (p1->flags & NvgPtCorner) ? NvgPtCorner : 0;

                    // Keep track of left turns.
                    const float cross = p1->dx * p0->dy - p0->dx * p1->dy;
                    if (cross > 0.0f)
                    {
                        nleft++;
                        p1->flags |= NvgPtLeft;
                    }

                    // Calculate if we should use bevel or miter for inner join.
                    const float limit = nvg_max(1.01f, nvg_min(p0->len, p1->len) * iw);
                    if ((dmr2 * limit * limit) < 1.0f)
                        p1->flags |= NvgPrInnerbevel;

                    // Check to see if the corner needs to be beveled.
                    if (p1->flags & NvgPtCorner)
                        if ((dmr2 * miter_limit * miter_limit) < 1.0f || line_join == NVGBevel ||
                            line_join == NVGRound)
                            p1->flags |= NvgPtBevel;

                    if ((p1->flags & (NvgPtBevel | NvgPrInnerbevel)) != 0)
                        path->nbevel++;

                    p0 = p1++;
                }

                path->convex = (nleft == path->count) ? 1 : 0;
            }
        }

        static int32_t nvg_expand_stroke(const NVGcontext* ctx, float w, const float fringe,
                                         const int32_t line_cap, const int32_t line_join,
                                         const float miter_limit)
        {
            const NVGpathCache* cache = ctx->cache;
            int32_t i;
            const float aa = fringe;  // ctx->fringeWidth;
            float u0 = 0.0f, u1 = 1.0f;
            const int32_t ncap = nvg_curve_divs(w, std::numbers::pi_v<f32>,
                                                ctx->tess_tol);  // Calculate

            // divisions per
            // half circle.
            w += aa * 0.5f;

            // Disable the gradient used for antialiasing when antialiasing is not used.
            if (aa == 0.0f)
            {
                u0 = 0.5f;
                u1 = 0.5f;
            }

            nvg_calculate_joins(ctx, w, line_join, miter_limit);

            // Calculate max vertex usage.
            int32_t cverts = 0;
            for (i = 0; i < cache->npaths; i++)
            {
                const NVGpath* path = &cache->paths[i];
                const int32_t loop = (path->closed == 0) ? 0 : 1;
                if (line_join == NVGRound)
                    cverts += (path->count + path->nbevel * (ncap + 2) + 1) * 2;  // plus one for
                                                                                  // loop
                else
                    cverts += (path->count + path->nbevel * 5 + 1) * 2;  // plus one for loop
                if (loop == 0)
                {
                    // space for caps
                    if (line_cap == NVGRound)
                        cverts += (ncap * 2 + 2) * 2;
                    else
                        cverts += (3 + 3) * 2;
                }
            }

            NVGvertex* verts = nvg_alloc_temp_verts(ctx, cverts);
            if (verts == nullptr)
                return 0;

            for (i = 0; i < cache->npaths; i++)
            {
                NVGpath* path = &cache->paths[i];
                NVGpoint* pts = &cache->points[path->first];
                NVGpoint* p0;
                NVGpoint* p1;
                int32_t s, e;
                float dx, dy;

                path->fill = nullptr;
                path->nfill = 0;

                // Calculate fringe or stroke
                const int32_t loop = (path->closed == 0) ? 0 : 1;
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
                    if (line_cap == NVGButt)
                        dst = nvg_butt_cap_start(dst, p0, dx, dy, w, -aa * 0.5f, aa, u0, u1);
                    else if (line_cap == NVGButt || line_cap == NVGSquare)
                        dst = nvg_butt_cap_start(dst, p0, dx, dy, w, w - aa, aa, u0, u1);
                    else if (line_cap == NVGRound)
                        dst = nvg_round_cap_start(dst, p0, dx, dy, w, ncap, aa, u0, u1);
                }

                for (int32_t j = s; j < e; ++j)
                {
                    if ((p1->flags & (NvgPtBevel | NvgPrInnerbevel)) != 0)
                    {
                        if (line_join == NVGRound)
                            dst = nvg_round_join(dst, p0, p1, w, w, u0, u1, ncap, aa);
                        else
                            dst = nvg_bevel_join(dst, p0, p1, w, w, u0, u1, aa);
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
                    if (line_cap == NVGButt)
                        dst = nvg_butt_cap_end(dst, p1, dx, dy, w, -aa * 0.5f, aa, u0, u1);
                    else if (line_cap == NVGButt || line_cap == NVGSquare)
                        dst = nvg_butt_cap_end(dst, p1, dx, dy, w, w - aa, aa, u0, u1);
                    else if (line_cap == NVGRound)
                        dst = nvg_round_cap_end(dst, p1, dx, dy, w, ncap, aa, u0, u1);
                }

                path->nstroke = static_cast<int32_t>(dst - verts);

                verts = dst;
            }

            return 1;
        }

        static int32_t nvg_expand_fill(const NVGcontext* ctx, const float w, const int32_t lineJoin,
                                       const float miterLimit)
        {
            const NVGpathCache* cache = ctx->cache;
            int32_t i, j;
            const float aa = ctx->fringe_width;
            const int32_t fringe = w > 0.0f;

            nvg_calculate_joins(ctx, w, lineJoin, miterLimit);

            // Calculate max vertex usage.
            int32_t cverts = 0;
            for (i = 0; i < cache->npaths; i++)
            {
                const NVGpath* path = &cache->paths[i];
                cverts += path->count + path->nbevel + 1;
                if (fringe)
                    cverts += (path->count + path->nbevel * 5 + 1) * 2;  // plus one for loop
            }

            NVGvertex* verts = nvg_alloc_temp_verts(ctx, cverts);
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
                        if (p1->flags & NvgPtBevel)
                        {
                            const float dlx0 = p0->dy;
                            const float dly0 = -p0->dx;
                            const float dlx1 = p1->dy;
                            const float dly1 = -p1->dx;
                            if (p1->flags & NvgPtLeft)
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
                            nvg_vset(dst, p1->x + (p1->dmx * woff), p1->y + (p1->dmy * woff), 0.5f,
                                     1);
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

                path->nfill = static_cast<int32_t>(dst - verts);
                verts = dst;

                // Calculate fringe
                if (fringe)
                {
                    float lw = w + woff;
                    const float rw = w - woff;
                    float lu = 0;
                    constexpr float ru = 1;
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
                        if ((p1->flags & (NvgPtBevel | NvgPrInnerbevel)) != 0)
                            dst = nvg_bevel_join(dst, p0, p1, lw, rw, lu, ru, ctx->fringe_width);
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

                    path->nstroke = static_cast<int32_t>(dst - verts);
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

        static void nvg_set_paint_color(NVGpaint* p, const NVGcolor color)
        {
            memset(p, 0, sizeof(*p));

            transform_identity(p->xform);

            p->radius = 0.0f;
            p->feather = 1.0f;
            p->inner_color = color;
            p->outer_color = color;
        }

        constexpr static float nvg_hue(float h, const float m1, const float m2)
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

        static float nvg_quantize(const float a, const float d)
        {
            return (a / d + 0.5f) * d;
        }

        static float nvg_get_font_scale(const NVGstate* state)
        {
            return nvg_min(nvg_quantize(nvg_get_average_scale(state->xform), 0.01f), 4.0f);
        }

        static void nvg_flush_text_texture(const NVGcontext* ctx)
        {
            int32_t dirty[4] = { 0 };

            if (fons_validate_texture(ctx->fs, dirty))
            {
                const int32_t font_image = ctx->font_images[ctx->font_image_idx];
                // Update texture
                if (font_image != 0)
                {
                    int32_t iw, ih;
                    const uint8_t* data = fons_get_texture_data(ctx->fs, &iw, &ih);
                    const int32_t x = dirty[0];
                    const int32_t y = dirty[1];
                    const int32_t w = dirty[2] - dirty[0];
                    const int32_t h = dirty[3] - dirty[1];
                    ctx->params.render_update_texture(ctx->params.user_ptr, font_image, x, y, w, h,
                                                      data);
                }
            }
        }

        static int32_t nvg_alloc_text_atlas(NVGcontext* ctx)
        {
            nvg_flush_text_texture(ctx);
            if (ctx->font_image_idx >= NvgMaxFontimages - 1)
                return 0;

            float iw, ih;
            // if next fontImage already have a texture
            if (ctx->font_images[ctx->font_image_idx + 1] != 0)
                image_size(ctx, ctx->font_images[ctx->font_image_idx + 1], &iw, &ih);
            else
            {
                // calculate the new font image size and create it.
                image_size(ctx, ctx->font_images[ctx->font_image_idx], &iw, &ih);
                if (iw > ih)
                    ih *= 2;
                else
                    iw *= 2;

                if (iw > static_cast<float>(NvgMaxFontimageSize) ||
                    ih > static_cast<float>(NvgMaxFontimageSize))
                    iw = ih = NvgMaxFontimageSize;

                ctx->font_images[ctx->font_image_idx + 1] = ctx->params.render_create_texture(
                    ctx->params.user_ptr, NVGTextureAlpha, static_cast<int32_t>(iw),
                    static_cast<int32_t>(ih), 0, nullptr);
            }

            ++ctx->font_image_idx;
            fons_reset_atlas(ctx->fs, static_cast<int32_t>(iw), static_cast<int32_t>(ih));

            return 1;
        }

        static void nvg_render_text(NVGcontext* ctx, const NVGvertex* vertices,
                                    const int32_t vertex_count)
        {
            const NVGstate* state = detail::nvg_get_state(ctx);
            NVGpaint paint = state->fill;

            // Render triangles.
            paint.image = ctx->font_images[ctx->font_image_idx];

            // Apply global alpha
            paint.inner_color.a *= state->alpha;
            paint.outer_color.a *= state->alpha;

            ctx->params.render_triangles(ctx->params.user_ptr, &paint, state->composite_operation,
                                         &state->scissor, vertices, vertex_count,
                                         ctx->fringe_width);

            ctx->draw_call_count++;
            ctx->text_tri_count += vertex_count / 3;
        }

        static int32_t nvg_is_transform_flipped(const float* xform)
        {
            const float det = xform[0] * xform[3] - xform[2] * xform[1];
            return det < 0;
        }
    }

    NVGcontext* create_internal(const NVGparams* params)
    {
        FONSparams font_params;
        const auto ctx = static_cast<NVGcontext*>(malloc(sizeof(NVGcontext)));
        if (ctx != nullptr)
        {
            memset(ctx, 0, sizeof(NVGcontext));

            ctx->params = *params;
            for (int32_t& font_image : ctx->font_images)
                font_image = 0;

            ctx->commands = static_cast<float*>(malloc(sizeof(float) * NvgInitCommandsSize));
            if (ctx->commands != nullptr)
            {
                ctx->ncommands = 0;
                ctx->ccommands = NvgInitCommandsSize;

                ctx->cache = detail::nvg_alloc_path_cache();
                if (ctx->cache != nullptr)
                {
                    save(ctx);
                    reset(ctx);

                    detail::nvg_set_device_pixel_ratio(ctx, 1.0f);

                    if (ctx->params.render_create(ctx->params.user_ptr) != 0)
                    {
                        // Init font rendering
                        memset(&font_params, 0, sizeof(font_params));
                        font_params.width = NvgInitFontimageSize;
                        font_params.height = NvgInitFontimageSize;
                        font_params.flags = FonsZeroTopleft;
                        font_params.render_create = nullptr;
                        font_params.render_update = nullptr;
                        font_params.render_draw = nullptr;
                        font_params.render_delete = nullptr;
                        font_params.user_ptr = nullptr;

                        ctx->fs = fons_create_internal(&font_params);
                        if (ctx->fs != nullptr)
                        {
                            // Create font texture
                            ctx->font_images[0] = ctx->params.render_create_texture(
                                ctx->params.user_ptr, NVGTextureAlpha, font_params.width,
                                font_params.height, 0, nullptr);

                            if (ctx->font_images[0] != 0)
                            {
                                ctx->font_image_idx = 0;
                                return ctx;
                            }
                        }
                    }
                }
            }
        }

        // error
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
            detail::nvg_delete_path_cache(ctx->cache);
        if (ctx->fs)
            fons_delete_internal(ctx->fs);

        for (i32& font_image : ctx->font_images)
        {
            if (font_image != 0)
            {
                delete_image(ctx, font_image);
                font_image = 0;
            }
        }

        if (ctx->params.render_delete != nullptr)
            ctx->params.render_delete(ctx->params.user_ptr);

        free(ctx);
    }

    void begin_frame(NVGcontext* ctx, const float window_width, const float window_height,
                     const float device_pixel_ratio)
    {
        ctx->nstates = 0;

        save(ctx);
        reset(ctx);

        detail::nvg_set_device_pixel_ratio(ctx, device_pixel_ratio);
        ctx->params.render_viewport(ctx->params.user_ptr, window_width, window_height,
                                    device_pixel_ratio);

        ctx->draw_call_count = 0;
        ctx->fill_tri_count = 0;
        ctx->stroke_tri_count = 0;
        ctx->text_tri_count = 0;
    }

    void cancel_frame(const NVGcontext* ctx)
    {
        ctx->params.render_cancel(ctx->params.user_ptr);
    }

    void end_frame(NVGcontext* ctx)
    {
        ctx->params.render_flush(ctx->params.user_ptr);
        if (ctx->font_image_idx != 0)
        {
            const int32_t font_image = ctx->font_images[ctx->font_image_idx];
            ctx->font_images[ctx->font_image_idx] = 0;

            // delete images that smaller than current one
            if (font_image == 0)
                return;

            float iw{ 0.0f };
            float ih{ 0.0f };
            image_size(ctx, font_image, &iw, &ih);

            int32_t j = 0;
            for (int32_t i = 0; i < ctx->font_image_idx; i++)
            {
                if (ctx->font_images[i] != 0)
                {
                    const int32_t image = ctx->font_images[i];
                    ctx->font_images[i] = 0;

                    float nw, nh;
                    image_size(ctx, image, &nw, &nh);

                    if (nw < iw || nh < ih)
                        delete_image(ctx, image);
                    else
                        ctx->font_images[j++] = image;
                }
            }

            // make current font image to first
            ctx->font_images[j] = ctx->font_images[0];
            ctx->font_images[0] = font_image;
            ctx->font_image_idx = 0;
        }
    }

    NVGcolor trans_rgba(NVGcolor c0, const uint8_t a)
    {
        c0.a = static_cast<f32>(a) / 255.0f;
        return c0;
    }

    NVGcolor trans_rgba_f(NVGcolor c0, const float a)
    {
        c0.a = a;
        return c0;
    }

    NVGcolor lerp_rgba(const NVGcolor c0, const NVGcolor c1, float u)
    {
        NVGcolor cint = { { { 0 } } };

        u = detail::nvg_clampf(u, 0.0f, 1.0f);
        const float oneminu = 1.0f - u;
        for (int32_t i = 0; i < 4; i++)
            cint.rgba[i] = c0.rgba[i] * oneminu + c1.rgba[i] * u;

        return cint;
    }

    NVGcolor hsl(const float h, const float s, const float l)
    {
        return hsla(h, s, l, 255);
    }

    NVGcolor hsla(float h, float s, float l, const uint8_t a)
    {
        NVGcolor col;
        h = detail::nvg_modf(h, 1.0f);
        if (h < 0.0f)
            h += 1.0f;

        s = detail::nvg_clampf(s, 0.0f, 1.0f);
        l = detail::nvg_clampf(l, 0.0f, 1.0f);

        const float m2 = l <= 0.5f ? (l * (1 + s)) : (l + s - l * s);
        const float m1 = 2 * l - m2;

        col.r = detail::nvg_clampf(detail::nvg_hue(h + 1.0f / 3.0f, m1, m2), 0.0f, 1.0f);
        col.g = detail::nvg_clampf(detail::nvg_hue(h, m1, m2), 0.0f, 1.0f);
        col.b = detail::nvg_clampf(detail::nvg_hue(h - 1.0f / 3.0f, m1, m2), 0.0f, 1.0f);

        col.a = static_cast<f32>(a) / 255.0f;

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

    void transform_translate(float* t, const ds::vector2<f32>& translation)
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
        const float cs = detail::nvg_cosf(a);
        const float sn = detail::nvg_sinf(a);
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
        dst[2] = detail::nvg_tanf(a);
        dst[3] = 1.0f;
        dst[4] = 0.0f;
        dst[5] = 0.0f;
    }

    void transform_skew_y(float* dst, const float a)
    {
        dst[0] = 1.0f;
        dst[1] = detail::nvg_tanf(a);
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

    int32_t transform_inverse(float* dst, const float* src)
    {
        const double det = static_cast<double>(src[0]) * src[3] -
                           static_cast<double>(src[2]) * src[1];

        if (det > -1e-6 && det < 1e-6)
        {
            transform_identity(dst);
            return 0;
        }

        const double invdet = 1.0 / det;
        dst[0] = static_cast<float>(src[3] * invdet);
        dst[2] = static_cast<float>(-src[2] * invdet);
        dst[4] = static_cast<float>(
            (static_cast<double>(src[2]) * src[5] - static_cast<double>(src[3]) * src[4]) * invdet);
        dst[1] = static_cast<float>(-src[1] * invdet);
        dst[3] = static_cast<float>(src[0] * invdet);
        dst[5] = static_cast<float>(
            (static_cast<double>(src[1]) * src[4] - static_cast<double>(src[0]) * src[5]) * invdet);

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
        NVGstate* state = detail::nvg_get_state(ctx);
        memset(state, 0, sizeof(*state));

        detail::nvg_set_paint_color(&state->fill, rgba(255, 255, 255, 255));
        detail::nvg_set_paint_color(&state->stroke, rgba(0, 0, 0, 255));

        state->composite_operation = detail::nvg_composite_operation_state(NVGSourceOver);
        state->shape_anti_alias = 1;
        state->stroke_width = 1.0f;
        state->miter_limit = 10.0f;
        state->line_cap = NVGButt;
        state->line_join = NVGMiter;
        state->alpha = 1.0f;

        transform_identity(state->xform);

        state->scissor.extent[0] = -1.0f;
        state->scissor.extent[1] = -1.0f;

        state->font_size = 16.0f;
        state->letter_spacing = 0.0f;
        state->line_height = 1.0f;
        state->font_blur = 0.0f;
        state->text_align = NVGAlignLeft | NVGAlignBaseline;
        state->font_id = 0;
    }

    // State setting
    void shape_anti_alias(NVGcontext* ctx, const int32_t enabled)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->shape_anti_alias = enabled;
    }

    void stroke_width(NVGcontext* ctx, const float width)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->stroke_width = width;
    }

    void miter_limit(NVGcontext* ctx, const float limit)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->miter_limit = limit;
    }

    void line_cap(NVGcontext* ctx, const int32_t cap)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->line_cap = cap;
    }

    void line_join(NVGcontext* ctx, const int32_t join)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->line_join = join;
    }

    void global_alpha(NVGcontext* ctx, const float alpha)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->alpha = alpha;
    }

    void transform(NVGcontext* ctx, const float a, const float b, const float c, const float d,
                   const float e, const float f)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        const float t[6] = { a, b, c, d, e, f };
        transform_premultiply(state->xform, t);
    }

    void reset_transform(NVGcontext* ctx)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        transform_identity(state->xform);
    }

    void translate(NVGcontext* ctx, const float x, const float y)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        float t[6];
        transform_translate(t, x, y);
        transform_premultiply(state->xform, t);
    }

    void translate(NVGcontext* ctx, const ds::vector2<f32>& local_offset)
    {
        float t[6] = { 0 };
        NVGstate* state{ detail::nvg_get_state(ctx) };
        transform_translate(t, local_offset.x, local_offset.y);
        transform_premultiply(state->xform, t);
    }

    void rotate(NVGcontext* ctx, const float angle)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        float t[6];
        transform_rotate(t, angle);
        transform_premultiply(state->xform, t);
    }

    void skew_x(NVGcontext* ctx, const float angle)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        float t[6];
        transform_skew_x(t, angle);
        transform_premultiply(state->xform, t);
    }

    void skew_y(NVGcontext* ctx, const float angle)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        float t[6];
        transform_skew_y(t, angle);
        transform_premultiply(state->xform, t);
    }

    void scale(NVGcontext* ctx, const float x, const float y)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        float t[6];
        transform_scale(t, x, y);
        transform_premultiply(state->xform, t);
    }

    void current_transform(NVGcontext* ctx, float* xform)
    {
        const NVGstate* state = detail::nvg_get_state(ctx);
        if (xform == nullptr)
            return;
        memcpy(xform, state->xform, sizeof(float) * 6);
    }

    void stroke_color(NVGcontext* ctx, const NVGcolor color)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        detail::nvg_set_paint_color(&state->stroke, color);
    }

    void stroke_paint(NVGcontext* ctx, const NVGpaint& paint)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->stroke = paint;
        transform_multiply(state->stroke.xform, state->xform);
    }

    void fill_color(NVGcontext* ctx, const NVGcolor color)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        detail::nvg_set_paint_color(&state->fill, color);
    }

    void fill_paint(NVGcontext* ctx, const NVGpaint& paint)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->fill = paint;
        transform_multiply(state->fill.xform, state->xform);
    }

#ifndef NVG_NO_STB
    int32_t create_image(const NVGcontext* ctx, const char* filename, const int32_t image_flags)
    {
        int32_t w, h, n;
        stb::stbi_set_unpremultiply_on_load(1);
        stb::stbi_convert_iphone_png_to_rgb(1);
        uint8_t* img = stb::stbi_load(filename, &w, &h, &n, 4);
        if (img == nullptr)
            //		printf("Failed to load %s - %s\n", filename, stbi_failure_reason());
            return 0;
        const int32_t image = create_image_rgba(ctx, w, h, image_flags, img);
        stb::stbi_image_free(img);
        return image;
    }

    int32_t create_image_mem(const NVGcontext* ctx, const int32_t imageFlags, const uint8_t* data,
                             const int32_t ndata)
    {
        int32_t w, h, n;
        uint8_t* img = stb::stbi_load_from_memory(data, ndata, &w, &h, &n, 4);
        if (img == nullptr)
            //		printf("Failed to load %s - %s\n", filename, stbi_failure_reason());
            return 0;
        const int32_t image = create_image_rgba(ctx, w, h, imageFlags, img);
        stb::stbi_image_free(img);
        return image;
    }
#endif

    int32_t create_image_rgba(const NVGcontext* ctx, const int32_t w, const int32_t h,
                              const int32_t imageFlags, const uint8_t* data)
    {
        return ctx->params.render_create_texture(ctx->params.user_ptr, NVGTextureRgba, w, h,
                                                 imageFlags, data);
    }

    int32_t create_image_alpha(const NVGcontext* ctx, const int32_t w, const int32_t h,
                               const int32_t imageFlags, const uint8_t* data)
    {
        return ctx->params.render_create_texture(ctx->params.user_ptr, NVGTextureAlpha, w, h,
                                                 imageFlags, data);
    }

    void update_image(const NVGcontext* ctx, const int32_t image, const uint8_t* data)
    {
        float w{ 0.0f };
        float h{ 0.0f };
        ctx->params.render_get_texture_size(ctx->params.user_ptr, image, &w, &h);
        ctx->params.render_update_texture(ctx->params.user_ptr, image, 0, 0,
                                          static_cast<int32_t>(w), static_cast<int32_t>(h), data);
    }

    void image_size(const NVGcontext* ctx, const int32_t image, float* w, float* h)
    {
        ctx->params.render_get_texture_size(ctx->params.user_ptr, image, w, h);
    }

    void delete_image(const NVGcontext* ctx, const int32_t image)
    {
        ctx->params.render_delete_texture(ctx->params.user_ptr, image);
    }

    NVGpaint linear_gradient(NVGcontext* ctx, const float sx, const float sy, const float ex,
                             const float ey, const NVGcolor icol, const NVGcolor ocol)
    {
        NVGpaint p = {};
        constexpr float large = 1e5;

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

        p.feather = detail::nvg_max(1.0f, d);

        p.inner_color = icol;
        p.outer_color = ocol;

        return p;
    }

    NVGpaint radial_gradient(NVGcontext* ctx, const float cx, const float cy, const float inr,
                             const float outr, const NVGcolor icol, const NVGcolor ocol)
    {
        const float r = (inr + outr) * 0.5f;
        const float f = (outr - inr);
        NVGpaint p = {};

        transform_identity(p.xform);
        p.xform[4] = cx;
        p.xform[5] = cy;

        p.extent[0] = r;
        p.extent[1] = r;

        p.radius = r;

        p.feather = detail::nvg_max(1.0f, f);

        p.inner_color = icol;
        p.outer_color = ocol;

        return p;
    }

    NVGpaint box_gradient(NVGcontext* ctx, const float x, const float y, const float w,
                          const float h, const float r, const float f, const NVGcolor icol,
                          const NVGcolor ocol)
    {
        NVGpaint p = {};

        transform_identity(p.xform);

        p.xform[4] = x + w * 0.5f;
        p.xform[5] = y + h * 0.5f;
        p.extent[0] = w * 0.5f;
        p.extent[1] = h * 0.5f;

        p.radius = r;
        p.feather = detail::nvg_max(1.0f, f);
        p.inner_color = icol;
        p.outer_color = ocol;

        return p;
    }

    NVGpaint box_gradient(NVGcontext* ctx, const ds::rect<f32>& rect, const f32 corner_radius,
                          const f32 feather_blur, const ds::color<f32>& inner_color,
                          const ds::color<f32>& outer_gradient_color)
    {
        NVGpaint paint = {};
        transform_identity(paint.xform);

        paint.xform[4] = rect.pt.x + rect.size.width * 0.5f;
        paint.xform[5] = rect.pt.y + rect.size.height * 0.5f;
        paint.extent[0] = rect.size.width * 0.5f;
        paint.extent[1] = rect.size.height * 0.5f;

        paint.radius = corner_radius;
        paint.feather = detail::nvg_max(1.0f, feather_blur);
        paint.inner_color = inner_color.nvg();
        paint.outer_color = outer_gradient_color.nvg();

        return paint;
    }

    NVGpaint image_pattern(NVGcontext* ctx, const float cx, const float cy, const float w,
                           const float h, const float angle, const int32_t image, const float alpha)
    {
        NVGpaint p = {};

        transform_rotate(p.xform, angle);

        p.xform[4] = cx;
        p.xform[5] = cy;

        p.extent[0] = w;
        p.extent[1] = h;

        p.image = image;

        p.inner_color = p.outer_color = rgba_f(1, 1, 1, alpha);

        return p;
    }

    // Scissoring
    void scissor(NVGcontext* ctx, const float x, const float y, float w, float h)
    {
        NVGstate* state = detail::nvg_get_state(ctx);

        w = detail::nvg_max(0.0f, w);
        h = detail::nvg_max(0.0f, h);

        transform_identity(state->scissor.xform);
        state->scissor.xform[4] = x + w * 0.5f;
        state->scissor.xform[5] = y + h * 0.5f;
        transform_multiply(state->scissor.xform, state->xform);

        state->scissor.extent[0] = w * 0.5f;
        state->scissor.extent[1] = h * 0.5f;
    }

    static void nvg_isect_rects(float* dst, const float ax, const float ay, const float aw,
                                const float ah, const float bx, const float by, const float bw,
                                const float bh)
    {
        const float minx = detail::nvg_max(ax, bx);
        const float miny = detail::nvg_max(ay, by);
        const float maxx = detail::nvg_min(ax + aw, bx + bw);
        const float maxy = detail::nvg_min(ay + ah, by + bh);
        dst[0] = minx;
        dst[1] = miny;
        dst[2] = detail::nvg_max(0.0f, maxx - minx);
        dst[3] = detail::nvg_max(0.0f, maxy - miny);
    }

    void intersect_scissor(NVGcontext* ctx, const float x, const float y, const float w,
                           const float h)
    {
        const NVGstate* state = detail::nvg_get_state(ctx);
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
        const float tex = ex * detail::nvg_absf(pxform[0]) + ey * detail::nvg_absf(pxform[2]);
        const float tey = ex * detail::nvg_absf(pxform[1]) + ey * detail::nvg_absf(pxform[3]);

        // Intersect rects.
        nvg_isect_rects(rect, pxform[4] - tex, pxform[5] - tey, tex * 2, tey * 2, x, y, w, h);

        scissor(ctx, rect[0], rect[1], rect[2], rect[3]);
    }

    void reset_scissor(NVGcontext* ctx)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        memset(state->scissor.xform, 0, sizeof(state->scissor.xform));
        state->scissor.extent[0] = -1.0f;
        state->scissor.extent[1] = -1.0f;
    }

    // Global composite operation.
    void global_composite_operation(NVGcontext* ctx, const int32_t op)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->composite_operation = detail::nvg_composite_operation_state(op);
    }

    void global_composite_blend_func(NVGcontext* ctx, const int32_t sfactor, const int32_t dfactor)
    {
        global_composite_blend_func_separate(ctx, sfactor, dfactor, sfactor, dfactor);
    }

    void global_composite_blend_func_separate(NVGcontext* ctx, const int32_t src_rgb,
                                              const int32_t dst_rgb, const int32_t src_alpha,
                                              const int32_t dst_alpha)
    {
        NVGcompositeOperationState op;
        op.src_rgb = src_rgb;
        op.dst_rgb = dst_rgb;
        op.src_alpha = src_alpha;
        op.dst_alpha = dst_alpha;

        NVGstate* state = detail::nvg_get_state(ctx);
        state->composite_operation = op;
    }

    static float nvg_dist_pt_seg(const float x, const float y, const float px, const float py,
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

    static void nvg_append_commands(NVGcontext* ctx, float* vals, const int32_t nvals)
    {
        const NVGstate* state = detail::nvg_get_state(ctx);
        if (ctx->ncommands + nvals > ctx->ccommands)
        {
            const int32_t ccommands = ctx->ncommands + nvals + ctx->ccommands / 2;
            const auto commands = static_cast<float*>(
                realloc(ctx->commands, sizeof(float) * static_cast<uint64_t>(ccommands)));

            if (commands == nullptr)
                return;

            ctx->commands = commands;
            ctx->ccommands = ccommands;
        }

        const int32_t val = static_cast<int32_t>(vals[0]);
        if (val != Close && val != Winding)
        {
            ctx->commandx = vals[nvals - 2];
            ctx->commandy = vals[nvals - 1];
        }

        // transform commands
        int32_t i = 0;
        while (i < nvals)
        {
            const auto cmd{ static_cast<NVGcommands>(static_cast<int32_t>(vals[i])) };
            switch (cmd)
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

        std::memcpy(&ctx->commands[ctx->ncommands], vals,
                    static_cast<uint64_t>(nvals) * sizeof(float));

        ctx->ncommands += nvals;
    }

    // Draw
    void begin_path(NVGcontext* ctx)
    {
        ctx->ncommands = 0;
        detail::nvg_clear_path_cache(ctx);
    }

    void move_to(NVGcontext* ctx, const float x, const float y)
    {
        auto vals = std::array{ static_cast<float>(MoveTo), x, y };
        nvg_append_commands(ctx, vals.data(), static_cast<int32_t>(vals.size()));
    }

    void line_to(NVGcontext* ctx, const float x, const float y)
    {
        auto vals = std::array{ static_cast<float>(LineTo), x, y };
        nvg_append_commands(ctx, vals.data(), static_cast<int32_t>(vals.size()));
    }

    void bezier_to(NVGcontext* ctx, const float c1_x, const float c1_y, const float c2_x,
                   const float c2_y, const float x, const float y)
    {
        auto vals = std::array{ static_cast<float>(Bezierto), x, y };
        nvg_append_commands(ctx, vals.data(), static_cast<int32_t>(vals.size()));
    }

    void quad_to(NVGcontext* ctx, const float cx, const float cy, const float x, const float y)
    {
        const float x0 = ctx->commandx;
        const float y0 = ctx->commandy;
        auto vals = std::array{ static_cast<float>(Bezierto),
                                x0 + 2.0f / 3.0f * (cx - x0),
                                y0 + 2.0f / 3.0f * (cy - y0),
                                x + 2.0f / 3.0f * (cx - x),
                                y + 2.0f / 3.0f * (cy - y),
                                x,
                                y };
        nvg_append_commands(ctx, vals.data(), static_cast<int32_t>(vals.size()));
    }

    void arc_to(NVGcontext* ctx, const float x1, const float y1, const float x2, const float y2,
                const float radius)
    {
        const float x0 = ctx->commandx;
        const float y0 = ctx->commandy;
        float dx0, dy0, dx1, dy1, cx, cy, a0, a1;
        int32_t dir;

        if (ctx->ncommands == 0)
            return;

        // Handle degenerate cases.
        if (detail::nvg_pt_equals(x0, y0, x1, y1, ctx->dist_tol) ||
            detail::nvg_pt_equals(x1, y1, x2, y2, ctx->dist_tol) ||
            nvg_dist_pt_seg(x1, y1, x0, y0, x2, y2) < ctx->dist_tol * ctx->dist_tol ||
            radius < ctx->dist_tol)
        {
            line_to(ctx, x1, y1);
            return;
        }

        // Calculate tangential circle to lines (x0,y0)-(x1,y1) and (x1,y1)-(x2,y2).
        dx0 = x0 - x1;
        dy0 = y0 - y1;
        dx1 = x2 - x1;
        dy1 = y2 - y1;
        detail::nvg_normalize(&dx0, &dy0);
        detail::nvg_normalize(&dx1, &dy1);
        const float a = detail::nvg_acosf(dx0 * dx1 + dy0 * dy1);
        const float d = radius / detail::nvg_tanf(a / 2.0f);

        //	printf("a=%f° d=%f\n", a/std::numbers::pi_v<f32>*180.0f, d);

        if (d > 10000.0f)
        {
            line_to(ctx, x1, y1);
            return;
        }

        if (detail::nvg_cross(dx0, dy0, dx1, dy1) > 0.0f)
        {
            cx = x1 + dx0 * d + dy0 * radius;
            cy = y1 + dy0 * d + -dx0 * radius;
            a0 = detail::nvg_atan2f(dx0, -dy0);
            a1 = detail::nvg_atan2f(-dx1, dy1);
            dir = NVGcw;
            //		printf("CW c=(%f, %f) a0=%f° a1=%f°\n", cx, cy,
            // a0/std::numbers::pi_v<f32>*180.0f,
            // a1/std::numbers::pi_v<f32>*180.0f);
        }
        else
        {
            cx = x1 + dx0 * d + -dy0 * radius;
            cy = y1 + dy0 * d + dx0 * radius;
            a0 = detail::nvg_atan2f(-dx0, dy0);
            a1 = detail::nvg_atan2f(dx1, -dy1);
            dir = NVGccw;
            //		printf("CCW c=(%f, %f) a0=%f° a1=%f°\n", cx, cy,
            // a0/std::numbers::pi_v<f32>*180.0f,
            // a1/std::numbers::pi_v<f32>*180.0f);
        }

        arc(ctx, cx, cy, radius, a0, a1, dir);
    }

    void close_path(NVGcontext* ctx)
    {
        float vals[]{ static_cast<float>(Close) };
        nvg_append_commands(ctx, vals, std::size(vals));
    }

    void path_winding(NVGcontext* ctx, const int32_t dir)
    {
        float vals[] = { static_cast<float>(Winding), static_cast<float>(dir) };
        nvg_append_commands(ctx, vals, std::size(vals));
    }

    void barc(NVGcontext* ctx, const float cx, const float cy, const float r, const float a0,
              const float a1, const int32_t dir, const int32_t join)
    {
        float px = 0.0f;
        float py = 0.0f;
        float ptanx = 0.0f;
        float ptany = 0.0f;
        float vals[3 + 5 * 7 + 100] = { 0.0f };
        const int32_t move = join && ctx->ncommands > 0 ? LineTo : MoveTo;

        // Clamp angles
        float da = a1 - a0;
        if (dir == NVGcw)
        {
            if (detail::nvg_absf(da) >= std::numbers::pi_v<f32> * 2)
                da = std::numbers::pi_v<f32> * 2;
            else
                while (da < 0.0f)
                    da += std::numbers::pi_v<f32> * 2;
        }
        else
        {
            if (detail::nvg_absf(da) >= std::numbers::pi_v<f32> * 2)
                da = -std::numbers::pi_v<f32> * 2;
            else
                while (da > 0.0f)
                    da -= std::numbers::pi_v<f32> * 2;
        }

        // Split arc into max 90 degree segments.
        const int32_t ndivs = detail::nvg_max(
            1, detail::nvg_min(static_cast<int32_t>(std::round(
                                   detail::nvg_absf(da) / (std::numbers::pi_v<f32> * 0.5f))),
                               5));

        const float hda = (da / static_cast<float>(ndivs)) / 2.0f;
        float kappa = detail::nvg_absf(
            4.0f / 3.0f * (1.0f - detail::nvg_cosf(hda)) / detail::nvg_sinf(hda));

        if (dir == NVGccw)
            kappa = -kappa;

        int32_t nvals = 0;
        for (int32_t i = 0; i <= ndivs; i++)
        {
            const float a = a0 + da * (static_cast<float>(i) / static_cast<float>(ndivs));
            const float dx = detail::nvg_cosf(a);
            const float dy = detail::nvg_sinf(a);
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

        nvg_append_commands(ctx, vals, nvals);
    }

    void arc(NVGcontext* ctx, const float cx, const float cy, const float r, const float a0,
             const float a1, const int32_t dir)
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
        nvg_append_commands(ctx, vals.data(), std::size(vals));
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

        const float halfw = detail::nvg_absf(w) * 0.5f;
        const float halfh = detail::nvg_absf(h) * 0.5f;
        const float rx_bl = detail::nvg_min(rad_bottom_left, halfw) * detail::nvg_signf(w);
        const float ry_bl = detail::nvg_min(rad_bottom_left, halfh) * detail::nvg_signf(h);
        const float rx_br = detail::nvg_min(rad_bottom_right, halfw) * detail::nvg_signf(w);
        const float ry_br = detail::nvg_min(rad_bottom_right, halfh) * detail::nvg_signf(h);
        const float rx_tr = detail::nvg_min(rad_top_right, halfw) * detail::nvg_signf(w);
        const float ry_tr = detail::nvg_min(rad_top_right, halfh) * detail::nvg_signf(h);
        const float rx_tl = detail::nvg_min(rad_top_left, halfw) * detail::nvg_signf(w);
        const float ry_tl = detail::nvg_min(rad_top_left, halfh) * detail::nvg_signf(h);

        auto vals = std::array{
            static_cast<float>(MoveTo),
            x,
            y + ry_tl,
            static_cast<float>(LineTo),
            x,
            y + h - ry_bl,
            static_cast<float>(Bezierto),
            x,
            y + h - ry_bl * (1 - NVG_KAPPA90),
            x + rx_bl * (1 - NVG_KAPPA90),
            y + h,
            x + rx_bl,
            y + h,
            static_cast<float>(LineTo),
            x + w - rx_br,
            y + h,
            static_cast<float>(Bezierto),
            x + w - rx_br * (1 - NVG_KAPPA90),
            y + h,
            x + w,
            y + h - ry_br * (1 - NVG_KAPPA90),
            x + w,
            y + h - ry_br,
            static_cast<float>(LineTo),
            x + w,
            y + ry_tr,
            static_cast<float>(Bezierto),
            x + w,
            y + ry_tr * (1 - NVG_KAPPA90),
            x + w - rx_tr * (1 - NVG_KAPPA90),
            y,
            x + w - rx_tr,
            y,
            static_cast<float>(LineTo),
            x + rx_tl,
            y,
            static_cast<float>(Bezierto),
            x + rx_tl * (1 - NVG_KAPPA90),
            y,
            x,
            y + ry_tl * (1 - NVG_KAPPA90),
            x,
            y + ry_tl,
            static_cast<float>(Close),
        };

        nvg_append_commands(ctx, vals.data(), vals.size());
    }

    void ellipse(NVGcontext* ctx, const float cx, const float cy, const float rx, const float ry)
    {
        auto vals = std::array{
            static_cast<float>(MoveTo),
            cx - rx,
            cy,
            static_cast<float>(Bezierto),
            cx - rx,
            cy + ry * NVG_KAPPA90,
            cx - rx * NVG_KAPPA90,
            cy + ry,
            cx,
            cy + ry,
            static_cast<float>(Bezierto),
            cx + rx * NVG_KAPPA90,
            cy + ry,
            cx + rx,
            cy + ry * NVG_KAPPA90,
            cx + rx,
            cy,
            static_cast<float>(Bezierto),
            cx + rx,
            cy - ry * NVG_KAPPA90,
            cx + rx * NVG_KAPPA90,
            cy - ry,
            cx,
            cy - ry,
            static_cast<float>(Bezierto),
            cx - rx * NVG_KAPPA90,
            cy - ry,
            cx - rx,
            cy - ry * NVG_KAPPA90,
            cx - rx,
            cy,
            static_cast<float>(Close),
        };

        nvg_append_commands(ctx, vals.data(), std::size(vals));
    }

    void circle(NVGcontext* ctx, const float cx, const float cy, const float r)
    {
        ellipse(ctx, cx, cy, r, r);
    }

    void debug_dump_path_cache(const NVGcontext* ctx)
    {
        int32_t j;

        std::println("Dumping {} cached paths", ctx->cache->npaths);
        for (int32_t i = 0; i < ctx->cache->npaths; i++)
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
        const NVGstate* state = detail::nvg_get_state(ctx);
        NVGpaint fill_paint = state->fill;

        detail::nvg_flatten_paths(ctx);
        if (ctx->params.edge_anti_alias && state->shape_anti_alias)
            detail::nvg_expand_fill(ctx, ctx->fringe_width, NVGMiter, 2.4f);
        else
            detail::nvg_expand_fill(ctx, 0.0f, NVGMiter, 2.4f);

        // Apply global alpha
        fill_paint.inner_color.a *= state->alpha;
        fill_paint.outer_color.a *= state->alpha;

        ctx->params.render_fill(ctx->params.user_ptr, &fill_paint, state->composite_operation,
                                &state->scissor, ctx->fringe_width, ctx->cache->bounds,
                                ctx->cache->paths, ctx->cache->npaths);

        // Count triangles
        for (int32_t i = 0; i < ctx->cache->npaths; i++)
        {
            const NVGpath* path = &ctx->cache->paths[i];
            ctx->fill_tri_count += path->nfill - 2;
            ctx->fill_tri_count += path->nstroke - 2;
            ctx->draw_call_count += 2;
        }
    }

    void stroke(NVGcontext* ctx)
    {
        const NVGstate* state = detail::nvg_get_state(ctx);
        const float scale = detail::nvg_get_average_scale(state->xform);
        float stroke_width = detail::nvg_clampf(state->stroke_width * scale, 0.0f, 200.0f);
        NVGpaint stroke_paint = state->stroke;

        if (stroke_width < ctx->fringe_width)
        {
            // If the stroke width is less than pixel size, use alpha to emulate coverage.
            // Since coverage is area, scale by alpha*alpha.
            const float alpha = detail::nvg_clampf(stroke_width / ctx->fringe_width, 0.0f, 1.0f);
            stroke_paint.inner_color.a *= alpha * alpha;
            stroke_paint.outer_color.a *= alpha * alpha;
            stroke_width = ctx->fringe_width;
        }

        // Apply global alpha
        stroke_paint.inner_color.a *= state->alpha;
        stroke_paint.outer_color.a *= state->alpha;

        detail::nvg_flatten_paths(ctx);

        if (ctx->params.edge_anti_alias && state->shape_anti_alias)
            detail::nvg_expand_stroke(ctx, stroke_width * 0.5f, ctx->fringe_width, state->line_cap,
                                      state->line_join, state->miter_limit);
        else
            detail::nvg_expand_stroke(ctx, stroke_width * 0.5f, 0.0f, state->line_cap,
                                      state->line_join, state->miter_limit);

        ctx->params.render_stroke(ctx->params.user_ptr, &stroke_paint, state->composite_operation,
                                  &state->scissor, ctx->fringe_width, stroke_width,
                                  ctx->cache->paths, ctx->cache->npaths);

        // Count triangles
        for (int32_t i = 0; i < ctx->cache->npaths; i++)
        {
            const NVGpath* path = &ctx->cache->paths[i];
            ctx->stroke_tri_count += path->nstroke - 2;
            ctx->draw_call_count++;
        }
    }

    // Add fonts
    int32_t create_font(const NVGcontext* ctx, const char* name, const char* filename)
    {
        return fons_add_font(ctx->fs, name, filename, 0);
    }

    int32_t create_font_at_index(const NVGcontext* ctx, const char* name, const char* filename,
                                 const int32_t font_index)
    {
        return fons_add_font(ctx->fs, name, filename, font_index);
    }

    int32_t create_font_mem(const NVGcontext* ctx, const char* name, uint8_t* data,
                            const int32_t ndata, const int32_t free_data)
    {
        return fons_add_font_mem(ctx->fs, name, data, ndata, free_data, 0);
    }

    i32 create_font_mem(const NVGcontext* ctx, const std::string_view& name,
                        const std::basic_string_view<u8>& font_data) noexcept
    {
        constexpr static i32 font_index{ 0 };
        constexpr static i32 dealloc_data{ false };

        return fons_add_font_mem(ctx->fs, name.data(), const_cast<uint8_t*>(font_data.data()),
                                 static_cast<i32>(font_data.size()), dealloc_data, font_index);
    }

    int32_t create_font_mem_at_index(const NVGcontext* ctx, const char* name, uint8_t* data,
                                     const int32_t ndata, const int32_t free_data,
                                     const int32_t font_index)
    {
        return fons_add_font_mem(ctx->fs, name, data, ndata, free_data, font_index);
    }

    int32_t find_font(const NVGcontext* ctx, const char* name)
    {
        if (name == nullptr)
            return -1;
        return fons_get_font_by_name(ctx->fs, name);
    }

    int32_t add_fallback_font_id(const NVGcontext* ctx, const int32_t base_font,
                                 const int32_t fallback_font)
    {
        if (base_font == -1 || fallback_font == -1)
            return 0;
        return fons_add_fallback_font(ctx->fs, base_font, fallback_font);
    }

    int32_t add_fallback_font(const NVGcontext* ctx, const char* base_font,
                              const char* fallback_font)
    {
        return add_fallback_font_id(ctx, find_font(ctx, base_font), find_font(ctx, fallback_font));
    }

    void reset_fallback_fonts_id(const NVGcontext* ctx, const int32_t base_font)
    {
        fons_reset_fallback_font(ctx->fs, base_font);
    }

    void reset_fallback_fonts(const NVGcontext* ctx, const char* base_font)
    {
        reset_fallback_fonts_id(ctx, find_font(ctx, base_font));
    }

    // State setting
    void font_size(NVGcontext* ctx, const float size)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->font_size = size;
    }

    void font_blur(NVGcontext* ctx, const float blur)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->font_blur = blur;
    }

    void text_letter_spacing(NVGcontext* ctx, const float spacing)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->letter_spacing = spacing;
    }

    void text_line_height(NVGcontext* ctx, const float line_height)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->line_height = line_height;
    }

    void text_align(NVGcontext* ctx, const int32_t align)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->text_align = align;
    }

    void font_face_id(NVGcontext* ctx, const int32_t font)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->font_id = font;
    }

    void font_face(NVGcontext* ctx, const char* font)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        state->font_id = fons_get_font_by_name(ctx->fs, font);
    }

    void font_face(NVGcontext* ctx, const std::string_view& font)
    {
        NVGstate* state{ detail::nvg_get_state(ctx) };
        state->font_id = fons_get_font_by_name(ctx->fs, font.data());
    }

    float text(NVGcontext* ctx, float x, float y, const char* string, const char* end /*= nullptr*/)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        FONStextIter iter, prev_iter;
        FONSquad q;
        NVGvertex* verts;
        float scale = detail::nvg_get_font_scale(state) * ctx->device_px_ratio;
        float invscale = 1.0f / scale;
        int32_t cverts = 0;
        int32_t nverts = 0;
        int32_t is_flipped = detail::nvg_is_transform_flipped(state->xform);

        if (end == nullptr)
            end = string + strlen(string);

        if (state->font_id == FONS_INVALID)
            return x;

        fons_set_size(ctx->fs, state->font_size * scale);
        fons_set_spacing(ctx->fs, state->letter_spacing * scale);
        fons_set_blur(ctx->fs, state->font_blur * scale);
        fons_set_align(ctx->fs, state->text_align);
        fons_set_font(ctx->fs, state->font_id);

        cverts = detail::nvg_max(2, static_cast<int32_t>(end - string)) * 6;  // conservative
                                                                              // estimate.
        verts = detail::nvg_alloc_temp_verts(ctx, cverts);
        if (verts == nullptr)
            return x;

        fons_text_iter_init(ctx->fs, &iter, x * scale, y * scale, string, end,
                            FonsGlyphBitmapRequired);
        prev_iter = iter;
        while (fons_text_iter_next(ctx->fs, &iter, &q))
        {
            float c[4 * 2] = { 0 };
            if (iter.prev_glyph_index == -1)
            {
                // can not retrieve glyph?
                if (nverts != 0)
                {
                    detail::nvg_render_text(ctx, verts, nverts);
                    nverts = 0;
                }

                if (detail::nvg_alloc_text_atlas(ctx) == 0)
                    break;  // no memory :(

                iter = prev_iter;
                fons_text_iter_next(ctx->fs, &iter, &q);  // try again
                if (iter.prev_glyph_index == -1)          // still can not find glyph?
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
                detail::nvg_vset(&verts[nverts], c[0], c[1], q.s0, q.t0);
                nverts++;
                detail::nvg_vset(&verts[nverts], c[4], c[5], q.s1, q.t1);
                nverts++;
                detail::nvg_vset(&verts[nverts], c[2], c[3], q.s1, q.t0);
                nverts++;
                detail::nvg_vset(&verts[nverts], c[0], c[1], q.s0, q.t0);
                nverts++;
                detail::nvg_vset(&verts[nverts], c[6], c[7], q.s0, q.t1);
                nverts++;
                detail::nvg_vset(&verts[nverts], c[4], c[5], q.s1, q.t1);
                nverts++;
            }
        }

        // TODO: add back-end bit to do this just once per frame.
        detail::nvg_flush_text_texture(ctx);
        detail::nvg_render_text(ctx, verts, nverts);

        return iter.nextx / scale;
    }

    void text_box(NVGcontext* ctx, const float x, float y, const float break_row_width,
                  const char* string, const char* end /*= nullptr*/)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        NVGtextRow rows[2];
        const int32_t old_align = state->text_align;
        const int32_t haling = state->text_align & (NVGAlignLeft | NVGAlignCenter | NVGAlignRight);
        const int32_t valign = state->text_align &
                               (NVGAlignTop | NVGAlignMiddle | NVGAlignBottom | NVGAlignBaseline);
        float lineh = 0;

        if (state->font_id == FONS_INVALID)
            return;

        text_metrics(ctx, nullptr, nullptr, &lineh);

        state->text_align = NVGAlignLeft | valign;

        int32_t nrows;
        while ((nrows = text_break_lines(ctx, string, end, break_row_width, rows, 2)))
        {
            for (int32_t i = 0; i < nrows; i++)
            {
                const NVGtextRow* row = &rows[i];
                if (haling & NVGAlignLeft)
                    text(ctx, x, y, row->start, row->end);
                else if (haling & NVGAlignCenter)
                    text(ctx, x + break_row_width * 0.5f - row->width * 0.5f, y, row->start,
                         row->end);
                else if (haling & NVGAlignRight)
                    text(ctx, x + break_row_width - row->width, y, row->start, row->end);
                y += lineh * state->line_height;
            }
            string = rows[nrows - 1].next;
        }

        state->text_align = old_align;
    }

    int32_t text_glyph_positions(NVGcontext* ctx, float x, float y, const char* string,
                                 const char* end, NVGglyphPosition* positions,
                                 int32_t max_positions)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        float scale = detail::nvg_get_font_scale(state) * ctx->device_px_ratio;
        float invscale = 1.0f / scale;
        FONStextIter iter, prev_iter;
        FONSquad q;
        int32_t npos = 0;

        if (state->font_id == FONS_INVALID)
            return 0;

        if (end == nullptr)
            end = string + strlen(string);

        if (string == end)
            return 0;

        fons_set_size(ctx->fs, state->font_size * scale);
        fons_set_spacing(ctx->fs, state->letter_spacing * scale);
        fons_set_blur(ctx->fs, state->font_blur * scale);
        fons_set_align(ctx->fs, state->text_align);
        fons_set_font(ctx->fs, state->font_id);

        fons_text_iter_init(ctx->fs, &iter, x * scale, y * scale, string, end,
                            FonsGlyphBitmapOptional);
        prev_iter = iter;
        while (fons_text_iter_next(ctx->fs, &iter, &q))
        {
            if (iter.prev_glyph_index < 0 && detail::nvg_alloc_text_atlas(ctx))
            {
                // can not retrieve glyph?
                iter = prev_iter;
                fons_text_iter_next(ctx->fs, &iter, &q);  // try again
            }
            prev_iter = iter;
            positions[npos].str = iter.str;
            positions[npos].x = iter.x * invscale;
            positions[npos].minx = detail::nvg_min(iter.x, q.x0) * invscale;
            positions[npos].maxx = detail::nvg_max(iter.nextx, q.x1) * invscale;
            npos++;
            if (npos >= max_positions)
                break;
        }

        return npos;
    }

    int32_t text_break_lines(NVGcontext* ctx, const char* string, const char* end,
                             float break_row_width, NVGtextRow* rows, int32_t max_rows)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        float scale = detail::nvg_get_font_scale(state) * ctx->device_px_ratio;
        float invscale = 1.0f / scale;
        FONStextIter iter;
        FONStextIter prev_iter;
        FONSquad q;
        int32_t nrows = 0;
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
        int32_t type = Space;
        int32_t ptype = Space;
        uint32_t pcodepoint = 0;

        if (max_rows == 0)
            return 0;
        if (state->font_id == FONS_INVALID)
            return 0;

        if (end == nullptr)
            end = string + strlen(string);

        if (string == end)
            return 0;

        fons_set_size(ctx->fs, state->font_size * scale);
        fons_set_spacing(ctx->fs, state->letter_spacing * scale);
        fons_set_blur(ctx->fs, state->font_blur * scale);
        fons_set_align(ctx->fs, state->text_align);
        fons_set_font(ctx->fs, state->font_id);

        break_row_width *= scale;

        fons_text_iter_init(ctx->fs, &iter, 0, 0, string, end, FonsGlyphBitmapOptional);
        prev_iter = iter;
        while (fons_text_iter_next(ctx->fs, &iter, &q))
        {
            if (iter.prev_glyph_index < 0 && detail::nvg_alloc_text_atlas(ctx))
            {
                // can not retrieve glyph?
                iter = prev_iter;
                fons_text_iter_next(ctx->fs, &iter, &q);  // try again
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
                float next_width = iter.nextx - row_start_x;

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
                if ((type == Char || type == CJK_Char) && next_width > break_row_width)
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
        const NVGstate* state = detail::nvg_get_state(ctx);
        const float scale = detail::nvg_get_font_scale(state) * ctx->device_px_ratio;
        const float invscale = 1.0f / scale;

        if (state->font_id == FONS_INVALID)
            return 0;

        fons_set_size(ctx->fs, state->font_size * scale);
        fons_set_spacing(ctx->fs, state->letter_spacing * scale);
        fons_set_blur(ctx->fs, state->font_blur * scale);
        fons_set_align(ctx->fs, state->text_align);
        fons_set_font(ctx->fs, state->font_id);

        const float width = fons_text_bounds(ctx->fs, x * scale, y * scale, string, end, bounds);
        if (bounds != nullptr)
        {
            // Use line bounds for height.
            fons_line_bounds(ctx->fs, y * scale, &bounds[1], &bounds[3]);
            bounds[0] *= invscale;
            bounds[1] *= invscale;
            bounds[2] *= invscale;
            bounds[3] *= invscale;
        }
        return width * invscale;
    }

    void text_box_bounds(NVGcontext* ctx, const float x, float y, const float break_row_width,
                         const char* string, const char* end, float* bounds)
    {
        NVGstate* state = detail::nvg_get_state(ctx);
        NVGtextRow rows[2];
        const float scale = detail::nvg_get_font_scale(state) * ctx->device_px_ratio;
        const float invscale = 1.0f / scale;
        const int32_t old_align = state->text_align;
        const int32_t haling = state->text_align & (NVGAlignLeft | NVGAlignCenter | NVGAlignRight);
        const int32_t valign = state->text_align &
                               (NVGAlignTop | NVGAlignMiddle | NVGAlignBottom | NVGAlignBaseline);
        float lineh = 0, rminy = 0, rmaxy = 0;
        float maxx, maxy;

        if (state->font_id == FONS_INVALID)
        {
            if (bounds != nullptr)
                bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0f;
            return;
        }

        text_metrics(ctx, nullptr, nullptr, &lineh);

        state->text_align = NVGAlignLeft | valign;

        float minx = maxx = x;
        float miny = maxy = y;

        fons_set_size(ctx->fs, state->font_size * scale);
        fons_set_spacing(ctx->fs, state->letter_spacing * scale);
        fons_set_blur(ctx->fs, state->font_blur * scale);
        fons_set_align(ctx->fs, state->text_align);
        fons_set_font(ctx->fs, state->font_id);
        fons_line_bounds(ctx->fs, 0, &rminy, &rmaxy);
        rminy *= invscale;
        rmaxy *= invscale;

        int32_t nrows;
        while ((nrows = text_break_lines(ctx, string, end, break_row_width, rows, 2)))
        {
            for (int32_t i = 0; i < nrows; i++)
            {
                const NVGtextRow* row = &rows[i];
                float dx = 0;

                // Horizontal bounds
                if (haling & NVGAlignLeft)
                    dx = 0;
                else if (haling & NVGAlignCenter)
                    dx = break_row_width * 0.5f - row->width * 0.5f;
                else if (haling & NVGAlignRight)
                    dx = break_row_width - row->width;

                const float rminx = x + row->minx + dx;
                const float rmaxx = x + row->maxx + dx;

                minx = detail::nvg_min(minx, rminx);
                maxx = detail::nvg_max(maxx, rmaxx);

                // Vertical bounds.
                miny = detail::nvg_min(miny, y + rminy);
                maxy = detail::nvg_max(maxy, y + rmaxy);

                y += lineh * state->line_height;
            }

            string = rows[nrows - 1].next;
        }

        state->text_align = old_align;

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
        const NVGstate* state = detail::nvg_get_state(ctx);
        const float scale = detail::nvg_get_font_scale(state) * ctx->device_px_ratio;
        const float invscale = 1.0f / scale;

        if (state->font_id == FONS_INVALID)
            return;

        fons_set_size(ctx->fs, state->font_size * scale);
        fons_set_spacing(ctx->fs, state->letter_spacing * scale);
        fons_set_blur(ctx->fs, state->font_blur * scale);
        fons_set_align(ctx->fs, state->text_align);
        fons_set_font(ctx->fs, state->font_id);

        fons_vert_metrics(ctx->fs, ascender, descender, lineh);
        if (ascender != nullptr)
            *ascender *= invscale;
        if (descender != nullptr)
            *descender *= invscale;
        if (lineh != nullptr)
            *lineh *= invscale;
    }
}
