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

    enum InitSize {
        NvgInitCommandsSize = 256,
        NvgInitPointsSize = 128,
        NvgInitPathsSize = 16,
        NvgInitVertsSize = 256
    };

    enum NVGcodepointType {
        Space,
        Newline,
        Char,
        CJKChar,
    };

    enum class Commands {
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

    namespace {
        namespace detail {
            float sqrtf(const float a)
            {
                return std::sqrtf(a);
            }

            float modf(const float a, const float b)
            {
                return std::fmodf(a, b);
            }

            float sinf(const float a)
            {
                return std::sinf(a);
            }

            float cosf(const float a)
            {
                return std::cosf(a);
            }

            float tanf(const float a)
            {
                return std::tanf(a);
            }

            // ReSharper disable once CppInconsistentNaming
            float atan2f(const float a, const float b)
            {
                return std::atan2f(a, b);
            }

            float acosf(const float a)
            {
                return std::acosf(a);
            }

            template <typename T>
            constexpr auto min(const T a, const T b)
            {
                return a < b ? a : b;
            }

            template <typename T>
            constexpr auto max(const T a, const T b)
            {
                return a > b ? a : b;
            }

            template <typename T>
            constexpr int32_t clampi(const T a, const T mn, const T mx)
            {
                return a < mn ? mn : a > mx ? mx : a;
            }

            constexpr float absf(const float a)
            {
                return a >= 0.0f ? a : -a;
            }

            constexpr float signf(const float a)
            {
                return a >= 0.0f ? 1.0f : -1.0f;
            }

            constexpr float clampf(const float a, const float mn, const float mx)
            {
                return a < mn ? mn : a > mx ? mx : a;
            }

            constexpr float cross(const float dx0, const float dy0, const float dx1, const float dy1)
            {
                return dx1 * dy0 - dx0 * dy1;
            }

            float normalize(float* x, float* y)
            {
                const float d = sqrtf(*x * *x + *y * *y);
                if (d > 1e-6f)
                {
                    const float id = 1.0f / d;
                    *x *= id;
                    *y *= id;
                }
                return d;
            }

            void delete_path_cache(PathCache* c)
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

            PathCache* alloc_path_cache()
            {
                const auto c = static_cast<PathCache*>(std::malloc(sizeof(PathCache)));
                if (c != nullptr)
                {
                    std::memset(c, 0, sizeof(PathCache));
                    c->points = static_cast<Point*>(std::malloc(sizeof(Point) * NvgInitPointsSize));

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
                            c->verts = static_cast<Vertex*>(
                                std::malloc(sizeof(Vertex) * NvgInitVertsSize));

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
                detail::delete_path_cache(c);
                return nullptr;
            }

            constexpr void set_device_pixel_ratio(Context* ctx, const float ratio)
            {
                ctx->tess_tol = 0.25f / ratio;
                ctx->dist_tol = 0.01f / ratio;
                ctx->fringe_width = 1.0f / ratio;
                ctx->device_px_ratio = ratio;
            }

            constexpr CompositeOperationState composite_operation_state(const CompositeOperation op)
            {
                BlendFactor sfactor{};
                BlendFactor dfactor{};

                switch (op)
                {
                    default:
                        [[fallthrough]];
                    case CompositeOperation::Copy:
                    {
                        sfactor = BlendFactor::One;
                        dfactor = BlendFactor::Zero;
                        break;
                    }
                    case CompositeOperation::SourceOver:
                    {
                        sfactor = BlendFactor::One;
                        dfactor = BlendFactor::OneMinusSrcAlpha;
                        break;
                    }
                    case CompositeOperation::SourceIn:
                    {
                        sfactor = BlendFactor::DstAlpha;
                        dfactor = BlendFactor::Zero;
                        break;
                    }
                    case CompositeOperation::SourceOut:
                    {
                        sfactor = BlendFactor::OneMinusDstAlpha;
                        dfactor = BlendFactor::Zero;
                        break;
                    }
                    case CompositeOperation::Atop:
                    {
                        sfactor = BlendFactor::DstAlpha;
                        dfactor = BlendFactor::OneMinusSrcAlpha;
                        break;
                    }
                    case CompositeOperation::DestinationOver:
                    {
                        sfactor = BlendFactor::OneMinusDstAlpha;
                        dfactor = BlendFactor::One;
                        break;
                    }
                    case CompositeOperation::DestinationIn:
                    {
                        sfactor = BlendFactor::Zero;
                        dfactor = BlendFactor::SrcAlpha;
                        break;
                    }
                    case CompositeOperation::DestinationOut:
                    {
                        sfactor = BlendFactor::Zero;
                        dfactor = BlendFactor::OneMinusSrcAlpha;
                        break;
                    }
                    case CompositeOperation::DestinationAtop:
                    {
                        sfactor = BlendFactor::OneMinusDstAlpha;
                        dfactor = BlendFactor::SrcAlpha;
                        break;
                    }
                    case CompositeOperation::Lighter:
                    {
                        sfactor = BlendFactor::One;
                        dfactor = BlendFactor::One;
                        break;
                    }
                    case CompositeOperation::Xor:
                    {
                        sfactor = BlendFactor::OneMinusDstAlpha;
                        dfactor = BlendFactor::OneMinusSrcAlpha;
                        break;
                    }
                }

                return CompositeOperationState{
                    .src_rgb = sfactor,
                    .dst_rgb = dfactor,
                    .src_alpha = sfactor,
                    .dst_alpha = dfactor,
                };
            }

            State* get_state(Context* ctx)
            {
                return &ctx->states[ctx->nstates - 1];
            }

            void clear_path_cache(const Context* ctx)
            {
                ctx->cache->npoints = 0;
                ctx->cache->npaths = 0;
            }

            NVGpath* last_path(const Context* ctx)
            {
                if (ctx->cache->npaths > 0)
                    return &ctx->cache->paths[ctx->cache->npaths - 1];
                return nullptr;
            }

            void add_path(const Context* ctx)
            {
                if (ctx->cache->npaths + 1 > ctx->cache->cpaths)
                {
                    const int32_t cpaths = ctx->cache->npaths + 1 + ctx->cache->cpaths / 2;
                    const auto paths = static_cast<NVGpath*>(realloc(
                        ctx->cache->paths, sizeof(NVGpath) * static_cast<uint64_t>(cpaths)));

                    if (paths == nullptr)
                        return;

                    ctx->cache->paths = paths;
                    ctx->cache->cpaths = cpaths;
                }
                NVGpath* path{ &ctx->cache->paths[ctx->cache->npaths] };
                std::memset(path, 0, sizeof(*path));
                path->first = ctx->cache->npoints;
                path->winding = ShapeWinding::CounterClockwise;

                ctx->cache->npaths++;
            }

            Point* last_point(const Context* ctx)
            {
                if (ctx->cache->npoints > 0)
                    return &ctx->cache->points[ctx->cache->npoints - 1];
                return nullptr;
            }

            int32_t pt_equals(const float x1, const float y1, const float x2, const float y2,
                              const float tol)
            {
                const float dx = x2 - x1;
                const float dy = y2 - y1;
                return dx * dx + dy * dy < tol * tol;
            }

            void add_point(const Context* ctx, const float x, const float y, const int32_t flags)
            {
                NVGpath* path = last_path(ctx);
                Point* pt;
                if (path == nullptr)
                    return;

                if (path->count > 0 && ctx->cache->npoints > 0)
                {
                    pt = last_point(ctx);
                    if (pt_equals(pt->x, pt->y, x, y, ctx->dist_tol))
                    {
                        pt->flags |= flags;
                        return;
                    }
                }

                if (ctx->cache->npoints + 1 > ctx->cache->cpoints)
                {
                    const int32_t cpoints = ctx->cache->npoints + 1 + ctx->cache->cpoints / 2;
                    const auto points = static_cast<Point*>(realloc(
                        ctx->cache->points, sizeof(Point) * static_cast<uint64_t>(cpoints)));
                    if (points == nullptr)
                        return;
                    ctx->cache->points = points;
                    ctx->cache->cpoints = cpoints;
                }

                pt = &ctx->cache->points[ctx->cache->npoints];
                std::memset(pt, 0, sizeof(*pt));
                pt->x = x;
                pt->y = y;
                pt->flags = static_cast<uint8_t>(flags);

                ctx->cache->npoints++;
                path->count++;
            }

            void close_path_internal(const Context* ctx)
            {
                NVGpath* path{ detail::last_path(ctx) };
                if (path == nullptr)
                    return;
                path->closed = 1;
            }

            void path_winding_internal(const Context* ctx, const ShapeWinding winding)
            {
                NVGpath* path = detail::last_path(ctx);
                if (path == nullptr)
                    return;
                path->winding = winding;
            }

            float get_average_scale(const float* t)
            {
                const float sx = detail::sqrtf(t[0] * t[0] + t[2] * t[2]);
                const float sy = detail::sqrtf(t[1] * t[1] + t[3] * t[3]);
                return (sx + sy) * 0.5f;
            }

            Vertex* alloc_temp_verts(const Context* ctx, const int32_t nverts)
            {
                if (nverts > ctx->cache->cverts)
                {
                    const int32_t cverts = nverts + 0xff & ~0xff;  // Round up to prevent
                                                                   // allocations when
                    // things change just slightly.
                    const auto verts = static_cast<Vertex*>(std::realloc(
                        ctx->cache->verts, sizeof(Vertex) * static_cast<uint64_t>(cverts)));

                    if (verts == nullptr)
                        return nullptr;

                    ctx->cache->verts = verts;
                    ctx->cache->cverts = cverts;
                }

                return ctx->cache->verts;
            }

            float triarea2(const float ax, const float ay, const float bx, const float by,
                           const float cx, const float cy)
            {
                const float abx{ bx - ax };
                const float aby{ by - ay };
                const float acx{ cx - ax };
                const float acy{ cy - ay };
                return acx * aby - abx * acy;
            }

            float poly_area(const Point* pts, const int32_t npts)
            {
                float area = 0;
                for (int32_t i = 2; i < npts; ++i)
                {
                    const Point* a{ &pts[0] };
                    const Point* b{ &pts[i - 1] };
                    const Point* c{ &pts[i] };
                    area += detail::triarea2(a->x, a->y, b->x, b->y, c->x, c->y);
                }
                return area * 0.5f;
            }

            void poly_reverse(Point* pts, const int32_t npts)
            {
                int32_t i = 0, j = npts - 1;
                while (i < j)
                {
                    const Point tmp = pts[i];
                    pts[i] = pts[j];
                    pts[j] = tmp;
                    i++;
                    j--;
                }
            }

            void vset(Vertex* vtx, const float x, const float y, const float u, const float v)
            {
                vtx->x = x;
                vtx->y = y;
                vtx->u = u;
                vtx->v = v;
            }

            void tesselate_bezier(Context* ctx, const float x1, const float y1, const float x2,
                                  const float y2, const float x3, const float y3, const float x4,
                                  const float y4, const int32_t level, const int32_t type)
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
                const float d2 = absf((x2 - x4) * dy - (y2 - y4) * dx);
                const float d3 = absf((x3 - x4) * dy - (y3 - y4) * dx);

                if ((d2 + d3) * (d2 + d3) < ctx->tess_tol * (dx * dx + dy * dy))
                {
                    add_point(ctx, x4, y4, type);
                    return;
                }

                /*	if (_absf(x1+x3-x2-x2) + _absf(y1+y3-y2-y2) + _absf(x2+x4-x3-x3) +
                   _absf(y2+y4-y3-y3) < ctx->tessTol) { _addPoint(ctx, x4, y4, type); return;
                    }*/

                const float x234 = (x23 + x34) * 0.5f;
                const float y234 = (y23 + y34) * 0.5f;
                const float x1234 = (x123 + x234) * 0.5f;
                const float y1234 = (y123 + y234) * 0.5f;

                tesselate_bezier(ctx, x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1, 0);
                tesselate_bezier(ctx, x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1, type);
            }

            void flatten_paths(Context* ctx)
            {
                PathCache* cache = ctx->cache;
                //	State* state = _getState(ctx);
                const Point* last{ nullptr };
                Point* pts{ nullptr };
                float* p;

                if (cache->npaths > 0)
                    return;

                // Flatten
                int32_t i = 0;
                while (i < ctx->ncommands)
                {
                    const auto cmd = static_cast<Commands>(ctx->commands[i]);
                    switch (cmd)
                    {
                        case Commands::MoveTo:
                            add_path(ctx);
                            p = &ctx->commands[i + 1];
                            detail::add_point(ctx, p[0], p[1], NvgPtCorner);
                            i += 3;
                            break;
                        case Commands::LineTo:
                            p = &ctx->commands[i + 1];
                            detail::add_point(ctx, p[0], p[1], NvgPtCorner);
                            i += 3;
                            break;
                        case Commands::Bezierto:
                            last = detail::last_point(ctx);
                            if (last != nullptr)
                            {
                                const float* cp1 = &ctx->commands[i + 1];
                                const float* cp2 = &ctx->commands[i + 3];
                                p = &ctx->commands[i + 5];
                                detail::tesselate_bezier(ctx, last->x, last->y, cp1[0], cp1[1],
                                                         cp2[0], cp2[1], p[0], p[1], 0,
                                                         NvgPtCorner);
                            }
                            i += 7;
                            break;
                        case Commands::Close:
                            detail::close_path_internal(ctx);
                            i++;
                            break;
                        case Commands::Winding:
                            detail::path_winding_internal(
                                ctx, static_cast<ShapeWinding>(ctx->commands[i + 1]));
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
                    NVGpath* path{ &cache->paths[j] };
                    pts = &cache->points[path->first];

                    // If the first and last points are the same, remove the last, mark as closed
                    // path.
                    Point* p0{ &pts[path->count - 1] };
                    Point* p1{ &pts[0] };
                    if (detail::pt_equals(p0->x, p0->y, p1->x, p1->y, ctx->dist_tol))
                    {
                        path->count--;
                        p0 = &pts[path->count - 1];
                        path->closed = 1;
                    }

                    // Enforce winding.
                    if (path->count > 2)
                    {
                        const float area = detail::poly_area(pts, path->count);
                        if (path->winding == ShapeWinding::CounterClockwise && area < 0.0f)
                            detail::poly_reverse(pts, path->count);
                        if (path->winding == ShapeWinding::Clockwise && area > 0.0f)
                            detail::poly_reverse(pts, path->count);
                    }

                    for (i = 0; i < path->count; i++)
                    {
                        // Calculate segment direction and length
                        p0->dx = p1->x - p0->x;
                        p0->dy = p1->y - p0->y;
                        p0->len = detail::normalize(&p0->dx, &p0->dy);
                        // Update bounds
                        cache->bounds[0] = detail::min(cache->bounds[0], p0->x);
                        cache->bounds[1] = detail::min(cache->bounds[1], p0->y);
                        cache->bounds[2] = detail::max(cache->bounds[2], p0->x);
                        cache->bounds[3] = detail::max(cache->bounds[3], p0->y);
                        // Advance
                        p0 = p1++;
                    }
                }
            }

            int32_t curve_divs(const float r, const float arc, const float tol)
            {
                const float da = acosf(r / (r + tol)) * 2.0f;
                return detail::max(2, static_cast<int32_t>(std::ceil(arc / da)));
            }

            void choose_bevel(const int32_t bevel, const Point* p0, const Point* p1, const float w,
                              float* x0, float* y0, float* x1, float* y1)
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

            Vertex* round_join(Vertex* dst, const Point* p0, const Point* p1, const float lw,
                               const float rw, const float lu, const float ru, const int32_t ncap,
                               float)
            {
                const float dlx0{ p0->dy };
                const float dly0{ -p0->dx };
                const float dlx1{ p1->dy };
                const float dly1{ -p1->dx };

                if (p1->flags & NvgPtLeft)
                {
                    float lx0, ly0, lx1, ly1;
                    detail::choose_bevel(p1->flags & NvgPrInnerbevel, p0, p1, lw, &lx0, &ly0, &lx1,
                                         &ly1);

                    const float a0 = atan2f(-dly0, -dlx0);
                    float a1 = atan2f(-dly1, -dlx1);
                    if (a1 > a0)
                        a1 -= std::numbers::pi_v<f32> * 2;

                    detail::vset(dst, lx0, ly0, lu, 1);
                    dst++;

                    detail::vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
                    dst++;

                    const int32_t n{ std::clamp(
                        static_cast<int32_t>(
                            ceilf((a0 - a1) / std::numbers::pi_v<f32> * static_cast<float>(ncap))),
                        2, ncap) };

                    for (int32_t i = 0; i < n; i++)
                    {
                        const float u{ static_cast<float>(i) / (static_cast<float>(n) - 1.0f) };
                        const float a{ a0 + u * (a1 - a0) };
                        const float rx{ p1->x + cosf(a) * rw };
                        const float ry{ p1->y + sinf(a) * rw };

                        detail::vset(dst, p1->x, p1->y, 0.5f, 1);
                        dst++;
                        detail::vset(dst, rx, ry, ru, 1);
                        dst++;
                    }

                    detail::vset(dst, lx1, ly1, lu, 1);
                    dst++;

                    detail::vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
                    dst++;
                }
                else
                {
                    float rx0, ry0, rx1, ry1;
                    detail::choose_bevel(p1->flags & NvgPrInnerbevel, p0, p1, -rw, &rx0, &ry0, &rx1,
                                         &ry1);
                    const float a0 = detail::atan2f(dly0, dlx0);
                    float a1 = detail::atan2f(dly1, dlx1);
                    if (a1 < a0)
                        a1 += std::numbers::pi_v<f32> * 2;

                    detail::vset(dst, p1->x + dlx0 * rw, p1->y + dly0 * rw, lu, 1);
                    dst++;
                    detail::vset(dst, rx0, ry0, ru, 1);
                    dst++;

                    const int32_t n{ std::clamp(
                        static_cast<int32_t>(std::ceil(
                            (a1 - a0) / std::numbers::pi_v<f32> * static_cast<float>(ncap))),
                        2, ncap) };

                    for (int32_t i = 0; i < n; ++i)
                    {
                        const float u = static_cast<float>(i) / (static_cast<float>(n) - 1.0f);
                        const float a = a0 + u * (a1 - a0);
                        const float lx = p1->x + std::cosf(a) * lw;
                        const float ly = p1->y + std::sinf(a) * lw;

                        detail::vset(dst, lx, ly, lu, 1.0f);
                        dst++;

                        detail::vset(dst, p1->x, p1->y, 0.5f, 1);
                        dst++;
                    }

                    detail::vset(dst, p1->x + dlx1 * rw, p1->y + dly1 * rw, lu, 1);
                    dst++;

                    detail::vset(dst, rx1, ry1, ru, 1);
                    dst++;
                }
                return dst;
            }

            Vertex* bevel_join(Vertex* dst, const Point* p0, const Point* p1, const float lw,
                               const float rw, const float lu, const float ru, float fringe)
            {
                float rx0, ry0, rx1, ry1;
                float lx0, ly0, lx1, ly1;
                const float dlx0{ p0->dy };
                const float dly0{ -p0->dx };
                const float dlx1{ p1->dy };
                const float dly1{ -p1->dx };

                if (p1->flags & NvgPtLeft)
                {
                    detail::choose_bevel(p1->flags & NvgPrInnerbevel, p0, p1, lw, &lx0, &ly0, &lx1,
                                         &ly1);

                    detail::vset(dst, lx0, ly0, lu, 1);
                    dst++;
                    detail::vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
                    dst++;

                    if (p1->flags & NvgPtBevel)
                    {
                        detail::vset(dst, lx0, ly0, lu, 1);
                        dst++;
                        detail::vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
                        dst++;

                        detail::vset(dst, lx1, ly1, lu, 1);
                        dst++;
                        detail::vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
                        dst++;
                    }
                    else
                    {
                        rx0 = p1->x - p1->dmx * rw;
                        ry0 = p1->y - p1->dmy * rw;

                        detail::vset(dst, p1->x, p1->y, 0.5f, 1);
                        dst++;
                        detail::vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
                        dst++;

                        detail::vset(dst, rx0, ry0, ru, 1);
                        dst++;
                        detail::vset(dst, rx0, ry0, ru, 1);
                        dst++;

                        detail::vset(dst, p1->x, p1->y, 0.5f, 1);
                        dst++;
                        detail::vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
                        dst++;
                    }

                    detail::vset(dst, lx1, ly1, lu, 1);
                    dst++;
                    detail::vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
                    dst++;
                }
                else
                {
                    detail::choose_bevel(p1->flags & NvgPrInnerbevel, p0, p1, -rw, &rx0, &ry0, &rx1,
                                         &ry1);

                    detail::vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1);
                    dst++;
                    detail::vset(dst, rx0, ry0, ru, 1);
                    dst++;

                    if (p1->flags & NvgPtBevel)
                    {
                        detail::vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1);
                        dst++;
                        detail::vset(dst, rx0, ry0, ru, 1);
                        dst++;

                        detail::vset(dst, p1->x + dlx1 * lw, p1->y + dly1 * lw, lu, 1);
                        dst++;
                        detail::vset(dst, rx1, ry1, ru, 1);
                        dst++;
                    }
                    else
                    {
                        lx0 = p1->x + p1->dmx * lw;
                        ly0 = p1->y + p1->dmy * lw;

                        detail::vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1.0f);
                        dst++;
                        detail::vset(dst, p1->x, p1->y, 0.5f, 1.0f);
                        dst++;

                        detail::vset(dst, lx0, ly0, lu, 1.0f);
                        dst++;
                        detail::vset(dst, lx0, ly0, lu, 1.0f);
                        dst++;

                        detail::vset(dst, p1->x + dlx1 * lw, p1->y + dly1 * lw, lu, 1.0f);
                        dst++;
                        detail::vset(dst, p1->x, p1->y, 0.5f, 1.0f);
                        dst++;
                    }

                    detail::vset(dst, p1->x + dlx1 * lw, p1->y + dly1 * lw, lu, 1.0f);
                    dst++;
                    detail::vset(dst, rx1, ry1, ru, 1);
                    dst++;
                }

                return dst;
            }

            Vertex* butt_cap_start(Vertex* dst, const Point* p, const float dx, const float dy,
                                   const float w, const float d, const float aa, const float u0,
                                   const float u1)
            {
                const float px{ p->x - dx * d };
                const float py{ p->y - dy * d };
                const float dlx{ dy };
                const float dly{ -dx };

                detail::vset(dst, px + dlx * w - dx * aa, py + dly * w - dy * aa, u0, 0);
                dst++;
                detail::vset(dst, px - dlx * w - dx * aa, py - dly * w - dy * aa, u1, 0);
                dst++;

                detail::vset(dst, px + dlx * w, py + dly * w, u0, 1);
                dst++;
                detail::vset(dst, px - dlx * w, py - dly * w, u1, 1);
                dst++;

                return dst;
            }

            Vertex* butt_cap_end(Vertex* dst, const Point* p, const float dx, const float dy,
                                 const float w, const float d, const float aa, const float u0,
                                 const float u1)
            {
                const float px = p->x + dx * d;
                const float py = p->y + dy * d;
                const float dlx = dy;
                const float dly = -dx;

                detail::vset(dst, px + dlx * w, py + dly * w, u0, 1);
                dst++;
                detail::vset(dst, px - dlx * w, py - dly * w, u1, 1);
                dst++;
                detail::vset(dst, px + dlx * w + dx * aa, py + dly * w + dy * aa, u0, 0);
                dst++;
                detail::vset(dst, px - dlx * w + dx * aa, py - dly * w + dy * aa, u1, 0);
                dst++;

                return dst;
            }

            Vertex* round_cap_start(Vertex* dst, const Point* p, const float dx, const float dy,
                                    const float w, const int32_t ncap, float aa, const float u0,
                                    const float u1)
            {
                const float px{ p->x };
                const float py{ p->y };
                const float dlx{ dy };
                const float dly{ -dx };

                for (int32_t i = 0; i < ncap; ++i)
                {
                    const float a = static_cast<float>(i) / (static_cast<float>(ncap) - 1.0f) *
                                    std::numbers::pi_v<f32>;
                    const float ax = detail::cosf(a) * w;
                    const float ay = detail::sinf(a) * w;

                    detail::vset(dst, px - dlx * ax - dx * ay, py - dly * ax - dy * ay, u0, 1);
                    dst++;
                    detail::vset(dst, px, py, 0.5f, 1);
                    dst++;
                }
                detail::vset(dst, px + dlx * w, py + dly * w, u0, 1);
                dst++;
                detail::vset(dst, px - dlx * w, py - dly * w, u1, 1);
                dst++;
                return dst;
            }

            Vertex* round_cap_end(Vertex* dst, const Point* p, const float dx, const float dy,
                                  const float w, const int32_t ncap, float aa, const float u0,
                                  const float u1)
            {
                const float px{ p->x };
                const float py{ p->y };
                const float dlx{ dy };
                const float dly{ -dx };

                detail::vset(dst, px + dlx * w, py + dly * w, u0, 1);
                dst++;
                detail::vset(dst, px - dlx * w, py - dly * w, u1, 1);
                dst++;

                for (int32_t i = 0; i < ncap; i++)
                {
                    const float a = static_cast<float>(i) / static_cast<float>(ncap - 1) *
                                    std::numbers::pi_v<f32>;
                    const float ax = detail::cosf(a) * w;
                    const float ay = detail::sinf(a) * w;

                    detail::vset(dst, px, py, 0.5f, 1);
                    dst++;
                    detail::vset(dst, px - dlx * ax + dx * ay, py - dly * ax + dy * ay, u0, 1);
                    dst++;
                }
                return dst;
            }

            void calculate_joins(const Context* ctx, const float w, const LineCap line_join,
                                 const float miter_limit)
            {
                const PathCache* cache{ ctx->cache };
                float iw = 0.0f;

                if (w > 0.0f)
                    iw = 1.0f / w;

                // Calculate which joins needs extra vertices to append, and gather vertex count.
                for (int32_t i = 0; i < cache->npaths; i++)
                {
                    NVGpath* path = &cache->paths[i];
                    Point* pts = &cache->points[path->first];
                    const Point* p0 = &pts[path->count - 1];
                    Point* p1 = &pts[0];
                    int32_t nleft = 0;

                    path->nbevel = 0;

                    for (int32_t j = 0; j < path->count; j++)
                    {
                        const float dlx0{ p0->dy };
                        const float dly0{ -p0->dx };
                        const float dlx1{ p1->dy };
                        const float dly1{ -p1->dx };

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
                        p1->flags = p1->flags & NvgPtCorner ? NvgPtCorner : 0;

                        // Keep track of left turns.
                        const float cross = p1->dx * p0->dy - p0->dx * p1->dy;
                        if (cross > 0.0f)
                        {
                            nleft++;
                            p1->flags |= NvgPtLeft;
                        }

                        // Calculate if we should use bevel or miter for inner join.
                        const float limit = detail::max(1.01f, detail::min(p0->len, p1->len) * iw);
                        if (dmr2 * limit * limit < 1.0f)
                            p1->flags |= NvgPrInnerbevel;

                        // Check to see if the corner needs to be beveled.
                        if (p1->flags & NvgPtCorner)
                            if (dmr2 * miter_limit * miter_limit < 1.0f ||
                                line_join == LineCap::Bevel || line_join == LineCap::Round)
                                p1->flags |= NvgPtBevel;

                        if ((p1->flags & (NvgPtBevel | NvgPrInnerbevel)) != 0)
                            path->nbevel++;

                        p0 = p1++;
                    }

                    path->convex = nleft == path->count ? 1 : 0;
                }
            }

            int32_t expand_stroke(const Context* ctx, float w, const float fringe,
                                  const LineCap line_cap, const LineCap line_join,
                                  const float miter_limit)
            {
                const PathCache* cache = ctx->cache;
                const float aa = fringe;  // ctx->fringeWidth;

                const int32_t ncap = detail::curve_divs(w, std::numbers::pi_v<f32>,
                                                        ctx->tess_tol);  // Calculate

                // divisions per
                // half circle.
                w += aa * 0.5f;

                float u0 = 0.0f;
                float u1 = 1.0f;
                // Disable the gradient used for antialiasing
                // when antialiasing is not used.
                if (aa == 0.0f)
                {
                    u0 = 0.5f;
                    u1 = 0.5f;
                }

                calculate_joins(ctx, w, line_join, miter_limit);

                // Calculate max vertex usage.
                int32_t cverts = 0;
                for (int32_t i = 0; i < cache->npaths; ++i)
                {
                    const NVGpath* path = &cache->paths[i];
                    const int32_t loop = path->closed == 0 ? 0 : 1;
                    if (line_join == LineCap::Round)
                        cverts += (path->count + path->nbevel * (ncap + 2) + 1) * 2;  // plus one
                                                                                      // for loop
                    else
                        cverts += (path->count + path->nbevel * 5 + 1) * 2;  // plus one for loop
                    if (loop == 0)
                    {
                        // space for caps
                        if (line_cap == LineCap::Round)
                            cverts += (ncap * 2 + 2) * 2;
                        else
                            cverts += (3 + 3) * 2;
                    }
                }

                Vertex* verts = alloc_temp_verts(ctx, cverts);
                if (verts == nullptr)
                    return 0;

                for (int32_t i = 0; i < cache->npaths; ++i)
                {
                    NVGpath* path = &cache->paths[i];
                    Point* pts = &cache->points[path->first];
                    Point* p0;
                    Point* p1;
                    int32_t s, e;
                    float dx, dy;

                    path->fill = nullptr;
                    path->nfill = 0;

                    // Calculate fringe or stroke
                    const int32_t loop = path->closed == 0 ? 0 : 1;
                    Vertex* dst = verts;
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
                        detail::normalize(&dx, &dy);
                        if (line_cap == LineCap::Butt)
                            dst = detail::butt_cap_start(dst, p0, dx, dy, w, -aa * 0.5f, aa, u0, u1);
                        else if (line_cap == LineCap::Butt || line_cap == LineCap::Square)
                            dst = detail::butt_cap_start(dst, p0, dx, dy, w, w - aa, aa, u0, u1);
                        else if (line_cap == LineCap::Round)
                            dst = detail::round_cap_start(dst, p0, dx, dy, w, ncap, aa, u0, u1);
                    }

                    for (int32_t j = s; j < e; ++j)
                    {
                        if ((p1->flags & (NvgPtBevel | NvgPrInnerbevel)) != 0)
                        {
                            if (line_join == LineCap::Round)
                                dst = detail::round_join(dst, p0, p1, w, w, u0, u1, ncap, aa);
                            else
                                dst = detail::bevel_join(dst, p0, p1, w, w, u0, u1, aa);
                        }
                        else
                        {
                            detail::vset(dst, p1->x + p1->dmx * w, p1->y + p1->dmy * w, u0, 1);
                            dst++;
                            detail::vset(dst, p1->x - p1->dmx * w, p1->y - p1->dmy * w, u1, 1);
                            dst++;
                        }
                        p0 = p1++;
                    }

                    if (loop)
                    {
                        // Loop it
                        detail::vset(dst, verts[0].x, verts[0].y, u0, 1);
                        dst++;
                        detail::vset(dst, verts[1].x, verts[1].y, u1, 1);
                        dst++;
                    }
                    else
                    {
                        // Add cap
                        dx = p1->x - p0->x;
                        dy = p1->y - p0->y;
                        detail::normalize(&dx, &dy);
                        if (line_cap == LineCap::Butt)
                            dst = detail::butt_cap_end(dst, p1, dx, dy, w, -aa * 0.5f, aa, u0, u1);
                        else if (line_cap == LineCap::Butt || line_cap == LineCap::Square)
                            dst = detail::butt_cap_end(dst, p1, dx, dy, w, w - aa, aa, u0, u1);
                        else if (line_cap == LineCap::Round)
                            dst = detail::round_cap_end(dst, p1, dx, dy, w, ncap, aa, u0, u1);
                    }

                    path->nstroke = static_cast<int32_t>(dst - verts);

                    verts = dst;
                }

                return 1;
            }

            int32_t expand_fill(const Context* ctx, const float w, const LineCap lineJoin,
                                const float miterLimit)
            {
                const PathCache* cache = ctx->cache;
                int32_t i, j;
                const float aa = ctx->fringe_width;
                const int32_t fringe = w > 0.0f;

                detail::calculate_joins(ctx, w, lineJoin, miterLimit);

                // Calculate max vertex usage.
                int32_t cverts = 0;
                for (i = 0; i < cache->npaths; i++)
                {
                    const NVGpath* path = &cache->paths[i];
                    cverts += path->count + path->nbevel + 1;
                    if (fringe)
                        cverts += (path->count + path->nbevel * 5 + 1) * 2;  // plus one for loop
                }

                Vertex* verts = detail::alloc_temp_verts(ctx, cverts);
                if (verts == nullptr)
                    return 0;

                const auto convex = cache->npaths == 1 && cache->paths[0].convex;

                for (i = 0; i < cache->npaths; i++)
                {
                    NVGpath* path = &cache->paths[i];
                    Point* pts = &cache->points[path->first];
                    Point* p0;
                    Point* p1;

                    // Calculate shape vertices.
                    const float woff = 0.5f * aa;
                    Vertex* dst = verts;
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
                                    detail::vset(dst, lx, ly, 0.5f, 1);
                                    dst++;
                                }
                                else
                                {
                                    const float lx0 = p1->x + dlx0 * woff;
                                    const float ly0 = p1->y + dly0 * woff;
                                    const float lx1 = p1->x + dlx1 * woff;
                                    const float ly1 = p1->y + dly1 * woff;
                                    detail::vset(dst, lx0, ly0, 0.5f, 1);
                                    dst++;
                                    detail::vset(dst, lx1, ly1, 0.5f, 1);
                                    dst++;
                                }
                            }
                            else
                            {
                                detail::vset(dst, p1->x + p1->dmx * woff, p1->y + p1->dmy * woff,
                                             0.5f, 1);
                                dst++;
                            }
                            p0 = p1++;
                        }
                    }
                    else
                        for (j = 0; j < path->count; ++j)
                        {
                            detail::vset(dst, pts[j].x, pts[j].y, 0.5f, 1);
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
                                dst = bevel_join(dst, p0, p1, lw, rw, lu, ru, ctx->fringe_width);
                            else
                            {
                                detail::vset(dst, p1->x + p1->dmx * lw, p1->y + p1->dmy * lw, lu, 1);
                                dst++;
                                detail::vset(dst, p1->x - p1->dmx * rw, p1->y - p1->dmy * rw, ru, 1);
                                dst++;
                            }
                            p0 = p1++;
                        }

                        // Loop it
                        detail::vset(dst, verts[0].x, verts[0].y, lu, 1);
                        dst++;
                        detail::vset(dst, verts[1].x, verts[1].y, ru, 1);
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

            void set_paint_color(PaintStyle* p, const ds::color<f32>& color)
            {
                *p = PaintStyle{};

                transform_identity(p->xform);

                p->radius = 0.0f;
                p->feather = 1.0f;
                p->inner_color = color;
                p->outer_color = color;
            }

            constexpr float hue(float h, const float m1, const float m2)
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

            float quantize(const float a, const float d)
            {
                return (a / d + 0.5f) * d;
            }

            float get_font_scale(const State* state)
            {
                return min(quantize(get_average_scale(state->xform), 0.01f), 4.0f);
            }

            void flush_text_texture(const Context* ctx)
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
                        ctx->params.render_update_texture(ctx->params.user_ptr, font_image, x, y, w,
                                                          h, data);
                    }
                }
            }

            int32_t alloc_text_atlas(Context* ctx)
            {
                flush_text_texture(ctx);
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
                        ctx->params.user_ptr, TextureProperty::Alpha, static_cast<int32_t>(iw),
                        static_cast<int32_t>(ih), ImageFlags::None, nullptr);
                }

                ++ctx->font_image_idx;
                fons_reset_atlas(ctx->fs, static_cast<int32_t>(iw), static_cast<int32_t>(ih));

                return 1;
            }

            void render_text(Context* ctx, const Vertex* vertices, const int32_t vertex_count)
            {
                const State* state = detail::get_state(ctx);
                PaintStyle paint = state->fill;

                // Render triangles.
                paint.image = ctx->font_images[ctx->font_image_idx];

                // Apply global alpha
                paint.inner_color.a *= state->alpha;
                paint.outer_color.a *= state->alpha;

                ctx->params.render_triangles(ctx->params.user_ptr, &paint,
                                             state->composite_operation, &state->scissor, vertices,
                                             vertex_count, ctx->fringe_width);

                ctx->draw_call_count++;
                ctx->text_tri_count += vertex_count / 3;
            }

            int32_t is_transform_flipped(const float* xform)
            {
                const float det = xform[0] * xform[3] - xform[2] * xform[1];
                return det < 0;
            }

            void isect_rects(float* dst, const float ax, const float ay, const float aw,
                             const float ah, const float bx, const float by, const float bw,
                             const float bh)
            {
                const float minx = detail::max(ax, bx);
                const float miny = detail::max(ay, by);
                const float maxx = detail::min(ax + aw, bx + bw);
                const float maxy = detail::min(ay + ah, by + bh);
                dst[0] = minx;
                dst[1] = miny;
                dst[2] = detail::max(0.0f, maxx - minx);
                dst[3] = detail::max(0.0f, maxy - miny);
            }

            float dist_pt_seg(const float x, const float y, const float px, const float py,
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

            void append_commands(Context* ctx, float* vals, const int32_t nvals)
            {
                const State* state = detail::get_state(ctx);
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
                if (val != Commands::Close && val != Commands::Winding)
                {
                    ctx->commandx = vals[nvals - 2];
                    ctx->commandy = vals[nvals - 1];
                }

                // transform commands
                int32_t i = 0;
                while (i < nvals)
                {
                    const auto cmd{ static_cast<Commands>(static_cast<int32_t>(vals[i])) };
                    switch (cmd)
                    {
                        case Commands::MoveTo:
                            [[fallthrough]];
                        case Commands::LineTo:
                            transform_point(&vals[i + 1], &vals[i + 2], state->xform, vals[i + 1],
                                            vals[i + 2]);
                            i += 3;
                            break;

                        case Commands::Bezierto:
                            transform_point(&vals[i + 1], &vals[i + 2], state->xform, vals[i + 1],
                                            vals[i + 2]);
                            transform_point(&vals[i + 3], &vals[i + 4], state->xform, vals[i + 3],
                                            vals[i + 4]);
                            transform_point(&vals[i + 5], &vals[i + 6], state->xform, vals[i + 5],
                                            vals[i + 6]);
                            i += 7;
                            break;

                        case Commands::Close:
                            i++;
                            break;

                        case Commands::Winding:
                            i += 2;
                            break;

                        default:
                            i++;
                    }
                }

                std::memcpy(&ctx->commands[ctx->ncommands], vals, nvals * sizeof(float));
                ctx->ncommands += nvals;
            }
        }
    }

    Context* create_internal(const Params* params)
    {
        FONSparams font_params{};
        const auto ctx{ static_cast<Context*>(std::malloc(sizeof(Context))) };
        if (ctx != nullptr)
        {
            *ctx = Context{ .params = *params };
            for (int32_t& font_image : ctx->font_images)
                font_image = 0;

            ctx->commands = static_cast<float*>(std::malloc(sizeof(float) * NvgInitCommandsSize));
            if (ctx->commands != nullptr)
            {
                ctx->ncommands = 0;
                ctx->ccommands = NvgInitCommandsSize;

                ctx->cache = detail::alloc_path_cache();
                if (ctx->cache != nullptr)
                {
                    save(ctx);
                    reset(ctx);

                    detail::set_device_pixel_ratio(ctx, 1.0f);
                    if (ctx->params.render_create(ctx->params.user_ptr) != 0)
                    {
                        // Init font rendering
                        std::memset(&font_params, 0, sizeof(font_params));
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
                                ctx->params.user_ptr, TextureProperty::Alpha, font_params.width,
                                font_params.height, ImageFlags::None, nullptr);

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

    Params* internal_params(Context* ctx)
    {
        return &ctx->params;
    }

    void delete_internal(Context* ctx)
    {
        if (ctx == nullptr)
            return;
        if (ctx->commands != nullptr)
            std::free(ctx->commands);
        if (ctx->cache != nullptr)
            detail::delete_path_cache(ctx->cache);
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

        std::free(ctx);
    }

    void begin_frame(Context* ctx, const float window_width, const float window_height,
                     const float device_pixel_ratio)
    {
        ctx->nstates = 0;

        save(ctx);
        reset(ctx);

        detail::set_device_pixel_ratio(ctx, device_pixel_ratio);
        ctx->params.render_viewport(ctx->params.user_ptr, window_width, window_height,
                                    device_pixel_ratio);

        ctx->draw_call_count = 0;
        ctx->fill_tri_count = 0;
        ctx->stroke_tri_count = 0;
        ctx->text_tri_count = 0;
    }

    void cancel_frame(const Context* ctx)
    {
        ctx->params.render_cancel(ctx->params.user_ptr);
    }

    void end_frame(Context* ctx)
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

    ds::color<f32> trans_rgba(ds::color<f32> c0, const uint8_t a)
    {
        c0.a = static_cast<f32>(a) / 255.0f;
        return c0;
    }

    ds::color<f32> trans_rgba_f(ds::color<f32> c0, const float a)
    {
        c0.a = a;
        return c0;
    }

    ds::color<f32> lerp_rgba(const ds::color<f32>& c0, const ds::color<f32>& c1, float u)
    {
        ds::color cint{ 0.0f, 0.0f, 0.0f, 0.0f };

        u = std::clamp(u, 0.0f, 1.0f);
        const float oneminu{ 1.0f - u };

        cint.r = c0.r * oneminu + c1.r * u;
        cint.g = c0.g * oneminu + c1.g * u;
        cint.b = c0.b * oneminu + c1.b * u;
        cint.a = c0.a * oneminu + c1.a * u;

        return cint;
    }

    ds::color<f32> hsl(const float h, const float s, const float l)
    {
        return hsla(h, s, l, 255);
    }

    ds::color<f32> hsla(float h, float s, float l, const uint8_t a)
    {
        ds::color<f32> col{ 0, 0, 0, 0 };
        h = detail::modf(h, 1.0f);
        if (h < 0.0f)
            h += 1.0f;

        s = detail::clampf(s, 0.0f, 1.0f);
        l = detail::clampf(l, 0.0f, 1.0f);

        const float m2{ l <= 0.5f ? l * (1 + s) : l + s - l * s };
        const float m1{ 2 * l - m2 };

        col.r = detail::clampf(detail::hue(h + 1.0f / 3.0f, m1, m2), 0.0f, 1.0f);
        col.g = detail::clampf(detail::hue(h, m1, m2), 0.0f, 1.0f);
        col.b = detail::clampf(detail::hue(h - 1.0f / 3.0f, m1, m2), 0.0f, 1.0f);
        col.a = static_cast<f32>(a) / 255.0f;

        return col;
    }

    void transform_identity(float* dst)
    {
        dst[0] = 1.0f;
        dst[1] = 0.0f;
        dst[2] = 0.0f;
        dst[3] = 1.0f;
        dst[4] = 0.0f;
        dst[5] = 0.0f;
    }

    void transform_translate(float* dst, const float tx, const float ty)
    {
        dst[0] = 1.0f;
        dst[1] = 0.0f;
        dst[2] = 0.0f;
        dst[3] = 1.0f;
        dst[4] = tx;
        dst[5] = ty;
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
        const float cs = std::cosf(a);
        const float sn = std::sinf(a);
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
        dst[2] = std::tanf(a);
        dst[3] = 1.0f;
        dst[4] = 0.0f;
        dst[5] = 0.0f;
    }

    void transform_skew_y(float* dst, const float a)
    {
        dst[0] = 1.0f;
        dst[1] = std::tanf(a);
        dst[2] = 0.0f;
        dst[3] = 1.0f;
        dst[4] = 0.0f;
        dst[5] = 0.0f;
    }

    void transform_multiply(float* dst, const float* src)
    {
        const float t0{ dst[0] * src[0] + dst[1] * src[2] };
        const float t2{ dst[2] * src[0] + dst[3] * src[2] };
        const float t4{ dst[4] * src[0] + dst[5] * src[2] + src[4] };

        dst[1] = dst[0] * src[1] + dst[1] * src[3];
        dst[3] = dst[2] * src[1] + dst[3] * src[3];
        dst[5] = dst[4] * src[1] + dst[5] * src[3] + src[5];

        dst[0] = t0;
        dst[2] = t2;
        dst[4] = t4;
    }

    void transform_premultiply(float* dst, const float* src)
    {
        float s2[6] = {};
        std::memcpy(s2, src, sizeof(float) * 6);
        transform_multiply(s2, dst);
        std::memcpy(dst, s2, sizeof(float) * 6);
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

    ds::point<f32> transform_point(Context* ctx, const ds::point<f32>& src_pt)
    {
        float xform[6] = {};
        nvg::current_transform(ctx, xform);
        return ds::point{
            src_pt.x * xform[0] + src_pt.y * xform[2] + xform[4],
            src_pt.x * xform[1] + src_pt.y * xform[3] + xform[5],
        };
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
    void save(Context* ctx)
    {
        if (ctx->nstates >= MaxNVGStates)
            return;

        if (ctx->nstates > 0)
            ctx->states[ctx->nstates] = ctx->states[ctx->nstates - 1];

        ctx->nstates++;
    }

    void restore(Context* ctx)
    {
        if (ctx->nstates <= 1)
            return;
        ctx->nstates--;
    }

    void reset(Context* ctx)
    {
        State* state{ detail::get_state(ctx) };
        *state = {};

        detail::set_paint_color(&state->fill, ds::color<f32>{ 255, 255, 255, 255 });
        detail::set_paint_color(&state->stroke, ds::color<f32>{ 0, 0, 0, 255 });

        state->composite_operation = detail::composite_operation_state(
            CompositeOperation::SourceOver);

        state->shape_anti_alias = true;
        state->stroke_width = 1.0f;
        state->miter_limit = 10.0f;
        state->line_cap = LineCap::Butt;
        state->line_join = LineCap::Miter;
        state->alpha = 1.0f;

        transform_identity(state->xform);

        state->scissor.extent[0] = -1.0f;
        state->scissor.extent[1] = -1.0f;

        state->font_size = 16.0f;
        state->letter_spacing = 0.0f;
        state->line_height = 1.0f;
        state->font_blur = 0.0f;
        state->text_align = Align::HLeft | Align::VBaseline;
        state->font_id = 0;
    }

    // State setting
    void shape_anti_alias(Context* ctx, const bool enabled)
    {
        State* state = detail::get_state(ctx);
        state->shape_anti_alias = enabled;
    }

    void stroke_width(Context* ctx, const float width)
    {
        State* state = detail::get_state(ctx);
        state->stroke_width = width;
    }

    void miter_limit(Context* ctx, const float limit)
    {
        State* state = detail::get_state(ctx);
        state->miter_limit = limit;
    }

    void line_cap(Context* ctx, const LineCap cap)
    {
        State* state = detail::get_state(ctx);
        state->line_cap = cap;
    }

    void line_join(Context* ctx, const LineCap join)
    {
        State* state = detail::get_state(ctx);
        state->line_join = join;
    }

    void global_alpha(Context* ctx, const float alpha)
    {
        State* state = detail::get_state(ctx);
        state->alpha = alpha;
    }

    void transform(Context* ctx, const float a, const float b, const float c, const float d,
                   const float e, const float f)
    {
        State* state = detail::get_state(ctx);
        const float t[6] = { a, b, c, d, e, f };
        transform_premultiply(state->xform, t);
    }

    void reset_transform(Context* ctx)
    {
        State* state = detail::get_state(ctx);
        transform_identity(state->xform);
    }

    void translate(Context* ctx, const float x, const float y)
    {
        State* state = detail::get_state(ctx);
        float t[6];
        transform_translate(t, x, y);
        transform_premultiply(state->xform, t);
    }

    void translate(Context* ctx, const ds::vector2<f32>& local_offset)
    {
        float t[6] = { 0 };
        State* state{ detail::get_state(ctx) };
        transform_translate(t, local_offset.x, local_offset.y);
        transform_premultiply(state->xform, t);
    }

    void rotate(Context* ctx, const float angle)
    {
        State* state = detail::get_state(ctx);
        float t[6];
        transform_rotate(t, angle);
        transform_premultiply(state->xform, t);
    }

    void skew_x(Context* ctx, const float angle)
    {
        State* state = detail::get_state(ctx);
        float t[6];
        transform_skew_x(t, angle);
        transform_premultiply(state->xform, t);
    }

    void skew_y(Context* ctx, const float angle)
    {
        State* state = detail::get_state(ctx);
        float t[6];
        transform_skew_y(t, angle);
        transform_premultiply(state->xform, t);
    }

    void scale(Context* ctx, const float x, const float y)
    {
        State* state = detail::get_state(ctx);
        float t[6];
        transform_scale(t, x, y);
        transform_premultiply(state->xform, t);
    }

    void current_transform(Context* ctx, float* xform)
    {
        const State* state = detail::get_state(ctx);
        if (xform == nullptr)
            return;
        std::memcpy(xform, state->xform, sizeof(float) * 6);
    }

    void stroke_color(Context* ctx, const ds::color<f32>& color)
    {
        State* state = detail::get_state(ctx);
        detail::set_paint_color(&state->stroke, color);
    }

    void stroke_paint(Context* ctx, const PaintStyle& paint)
    {
        State* state = detail::get_state(ctx);
        state->stroke = paint;
        transform_multiply(state->stroke.xform, state->xform);
    }

    void fill_color(Context* ctx, const ds::color<f32>& color)
    {
        State* state = detail::get_state(ctx);
        detail::set_paint_color(&state->fill, color);
    }

    void fill_paint(Context* ctx, PaintStyle&& paint)
    {
        State* state{ detail::get_state(ctx) };
        state->fill = std::move(paint);
        transform_multiply(state->fill.xform, state->xform);
    }

    // TODO: clean up
    // near identical clone of above
    void fill_paint(Context* ctx, const PaintStyle& paint)
    {
        State* state{ detail::get_state(ctx) };
        state->fill = paint;
        transform_multiply(state->fill.xform, state->xform);
    }

#ifndef NO_STB
    int32_t create_image(const Context* ctx, const char* filename, const ImageFlags image_flags)
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

    int32_t create_image_mem(const Context* ctx, const ImageFlags image_flags, const uint8_t* data,
                             const int32_t ndata)
    {
        int32_t w, h, n;
        uint8_t* img = stb::stbi_load_from_memory(data, ndata, &w, &h, &n, 4);
        if (img == nullptr)
            //		printf("Failed to load %s - %s\n", filename, stbi_failure_reason());
            return 0;
        const int32_t image = create_image_rgba(ctx, w, h, image_flags, img);
        stb::stbi_image_free(img);
        return image;
    }
#endif

    int32_t create_image_rgba(const Context* ctx, const int32_t w, const int32_t h,
                              const ImageFlags image_flags, const uint8_t* data)
    {
        return ctx->params.render_create_texture(ctx->params.user_ptr, TextureProperty::RGBA, w, h,
                                                 image_flags, data);
    }

    int32_t create_image_alpha(const Context* ctx, const int32_t w, const int32_t h,
                               const ImageFlags image_flags, const uint8_t* data)
    {
        return ctx->params.render_create_texture(ctx->params.user_ptr, TextureProperty::Alpha, w, h,
                                                 image_flags, data);
    }

    void update_image(const Context* ctx, const int32_t image, const uint8_t* data)
    {
        float w{ 0.0f };
        float h{ 0.0f };
        ctx->params.render_get_texture_size(ctx->params.user_ptr, image, &w, &h);
        ctx->params.render_update_texture(ctx->params.user_ptr, image, 0, 0,
                                          static_cast<int32_t>(w), static_cast<int32_t>(h), data);
    }

    void image_size(const Context* ctx, const int32_t image, float* w, float* h)
    {
        ctx->params.render_get_texture_size(ctx->params.user_ptr, image, w, h);
    }

    ds::dims<f32> image_size(const Context* ctx, const int32_t image)
    {
        ds::dims size{ 0.0f, 0.0f };
        ctx->params.render_get_texture_size(ctx->params.user_ptr, image, &size.width, &size.height);
        return size;
    }

    void delete_image(const Context* ctx, const int32_t image)
    {
        ctx->params.render_delete_texture(ctx->params.user_ptr, image);
    }

    PaintStyle linear_gradient(Context* ctx, const float sx, const float sy, const float ex,
                               const float ey, const ds::color<f32>& inner_color,
                               const ds::color<f32>& outer_color)
    {
        PaintStyle p{};
        constexpr static float large{ 1e5 };

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

        p.feather = detail::max(1.0f, d);

        p.inner_color = inner_color;
        p.outer_color = outer_color;

        return p;
    }

    PaintStyle radial_gradient(Context* ctx, const float cx, const float cy, const float inr,
                               const float outr, const ds::color<f32>& inner_color,
                               const ds::color<f32>& outer_color)
    {
        const float r = (inr + outr) * 0.5f;
        const float f = outr - inr;

        PaintStyle p{};

        transform_identity(p.xform);
        p.xform[4] = cx;
        p.xform[5] = cy;

        p.extent[0] = r;
        p.extent[1] = r;

        p.radius = r;

        p.feather = detail::max(1.0f, f);

        p.inner_color = inner_color;
        p.outer_color = outer_color;

        return p;
    }

    PaintStyle box_gradient(Context* ctx, const float x, const float y, const float w,
                            const float h, const float r, const float f, const ds::color<f32>& icol,
                            const ds::color<f32>& ocol)
    {
        PaintStyle p{};

        transform_identity(p.xform);

        p.xform[4] = x + w * 0.5f;
        p.xform[5] = y + h * 0.5f;
        p.extent[0] = w * 0.5f;
        p.extent[1] = h * 0.5f;

        p.radius = r;
        p.feather = detail::max(1.0f, f);
        p.inner_color = icol;
        p.outer_color = ocol;

        return p;
    }

    PaintStyle box_gradient(Context* ctx, ds::rect<f32>&& rect, const f32 corner_radius,
                            const f32 feather_blur, const ds::color<f32>& inner_color,
                            const ds::color<f32>& outer_gradient_color)
    {
        PaintStyle paint{};
        transform_identity(paint.xform);

        paint.xform[4] = rect.pt.x + rect.size.width * 0.5f;
        paint.xform[5] = rect.pt.y + rect.size.height * 0.5f;
        paint.extent[0] = rect.size.width * 0.5f;
        paint.extent[1] = rect.size.height * 0.5f;

        paint.radius = corner_radius;
        paint.feather = detail::max(1.0f, feather_blur);
        paint.inner_color = inner_color;
        paint.outer_color = outer_gradient_color;

        return paint;
    }

    PaintStyle image_pattern(Context* ctx, const float cx, const float cy, const float w,
                             const float h, const float angle, const int32_t image,
                             const float alpha)
    {
        PaintStyle p{};

        transform_rotate(p.xform, angle);

        p.xform[4] = cx;
        p.xform[5] = cy;

        p.extent[0] = w;
        p.extent[1] = h;

        p.image = image;

        p.inner_color = p.outer_color = ds::color<f32>{ 1.0f, 1.0f, 1.0f, alpha };

        return p;
    }

    // Scissoring
    void scissor(Context* ctx, const float x, const float y, float w, float h)
    {
        State* state = detail::get_state(ctx);

        w = detail::max(0.0f, w);
        h = detail::max(0.0f, h);

        transform_identity(state->scissor.xform);
        state->scissor.xform[4] = x + w * 0.5f;
        state->scissor.xform[5] = y + h * 0.5f;
        transform_multiply(state->scissor.xform, state->xform);

        state->scissor.extent[0] = w * 0.5f;
        state->scissor.extent[1] = h * 0.5f;
    }

    void intersect_scissor(Context* ctx, const float x, const float y, const float w, const float h)
    {
        const State* state = detail::get_state(ctx);
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
        std::memcpy(pxform, state->scissor.xform, sizeof(float) * 6);
        const float ex = state->scissor.extent[0];
        const float ey = state->scissor.extent[1];
        transform_inverse(invxorm, state->xform);
        transform_multiply(pxform, invxorm);
        const float tex = ex * detail::absf(pxform[0]) + ey * detail::absf(pxform[2]);
        const float tey = ex * detail::absf(pxform[1]) + ey * detail::absf(pxform[3]);

        // Intersect rects.
        detail::isect_rects(rect, pxform[4] - tex, pxform[5] - tey, tex * 2, tey * 2, x, y, w, h);

        scissor(ctx, rect[0], rect[1], rect[2], rect[3]);
    }

    void reset_scissor(Context* ctx)
    {
        State* state = detail::get_state(ctx);
        memset(state->scissor.xform, 0, sizeof(state->scissor.xform));
        state->scissor.extent[0] = -1.0f;
        state->scissor.extent[1] = -1.0f;
    }

    // Global composite operation.
    void global_composite_operation(Context* ctx, const CompositeOperation op)
    {
        State* state{ detail::get_state(ctx) };
        state->composite_operation = detail::composite_operation_state(op);
    }

    void global_composite_blend_func(Context* ctx, const BlendFactor sfactor,
                                     const BlendFactor dfactor)
    {
        global_composite_blend_func_separate(ctx, sfactor, dfactor, sfactor, dfactor);
    }

    void global_composite_blend_func_separate(
        Context* ctx, const BlendFactor src_rgb, const BlendFactor dst_rgb,
        const BlendFactor src_alpha, const BlendFactor dst_alpha)
    {
        CompositeOperationState op;
        op.src_rgb = src_rgb;
        op.dst_rgb = dst_rgb;
        op.src_alpha = src_alpha;
        op.dst_alpha = dst_alpha;

        State* state = detail::get_state(ctx);
        state->composite_operation = op;
    }

    // Draw
    void begin_path(Context* ctx)
    {
        ctx->ncommands = 0;
        detail::clear_path_cache(ctx);
    }

    void move_to(Context* ctx, const float x, const float y)
    {
        auto vals = std::array{ static_cast<float>(Commands::MoveTo), x, y };
        detail::append_commands(ctx, vals.data(), static_cast<int32_t>(vals.size()));
    }

    void line_to(Context* ctx, const float x, const float y)
    {
        auto vals = std::array{ static_cast<float>(Commands::LineTo), x, y };
        detail::append_commands(ctx, vals.data(), static_cast<int32_t>(vals.size()));
    }

    void bezier_to(Context* ctx, const float c1_x, const float c1_y, const float c2_x,
                   const float c2_y, const float x, const float y)
    {
        auto vals = std::array{ static_cast<float>(Commands::Bezierto), x, y };
        detail::append_commands(ctx, vals.data(), static_cast<int32_t>(vals.size()));
    }

    void quad_to(Context* ctx, const float cx, const float cy, const float x, const float y)
    {
        const float x0 = ctx->commandx;
        const float y0 = ctx->commandy;
        auto vals = std::array{
            static_cast<float>(Commands::Bezierto),
            x0 + 2.0f / 3.0f * (cx - x0),
            y0 + 2.0f / 3.0f * (cy - y0),
            x + 2.0f / 3.0f * (cx - x),
            y + 2.0f / 3.0f * (cy - y),
            x,
            y,
        };
        detail::append_commands(ctx, vals.data(), static_cast<int32_t>(vals.size()));
    }

    void arc_to(Context* ctx, const float x1, const float y1, const float x2, const float y2,
                const float radius)
    {
        const float x0 = ctx->commandx;
        const float y0 = ctx->commandy;

        if (ctx->ncommands == 0)
            return;

        // Handle degenerate cases.
        if (detail::pt_equals(x0, y0, x1, y1, ctx->dist_tol) ||
            detail::pt_equals(x1, y1, x2, y2, ctx->dist_tol) ||
            detail::dist_pt_seg(x1, y1, x0, y0, x2, y2) < ctx->dist_tol * ctx->dist_tol ||
            radius < ctx->dist_tol)
        {
            line_to(ctx, x1, y1);
            return;
        }

        // Calculate tangential circle to lines (x0,y0)-(x1,y1) and (x1,y1)-(x2,y2).
        float dx0 = x0 - x1;
        float dy0 = y0 - y1;
        float dx1 = x2 - x1;
        float dy1 = y2 - y1;
        detail::normalize(&dx0, &dy0);
        detail::normalize(&dx1, &dy1);
        const float a = detail::acosf(dx0 * dx1 + dy0 * dy1);
        const float d = radius / detail::tanf(a / 2.0f);

        //	printf("a=%f° d=%f\n", a/std::numbers::pi_v<f32>*180.0f, d);

        if (d > 10000.0f)
        {
            line_to(ctx, x1, y1);
            return;
        }

        float cx{ 0.0f };
        float cy{ 0.0f };
        float a0{ 0.0f };
        float a1{ 0.0f };
        ShapeWinding dir{ ShapeWinding::None };
        if (detail::cross(dx0, dy0, dx1, dy1) > 0.0f)
        {
            cx = x1 + dx0 * d + dy0 * radius;
            cy = y1 + dy0 * d + -dx0 * radius;
            a0 = detail::atan2f(dx0, -dy0);
            a1 = detail::atan2f(-dx1, dy1);
            dir = ShapeWinding::Clockwise;
            //		printf("CW c=(%f, %f) a0=%f° a1=%f°\n", cx, cy,
            // a0/std::numbers::pi_v<f32>*180.0f,
            // a1/std::numbers::pi_v<f32>*180.0f);
        }
        else
        {
            cx = x1 + dx0 * d + -dy0 * radius;
            cy = y1 + dy0 * d + dx0 * radius;
            a0 = detail::atan2f(-dx0, dy0);
            a1 = detail::atan2f(dx1, -dy1);
            dir = ShapeWinding::CounterClockwise;
            //		printf("CCW c=(%f, %f) a0=%f° a1=%f°\n", cx, cy,
            // a0/std::numbers::pi_v<f32>*180.0f,
            // a1/std::numbers::pi_v<f32>*180.0f);
        }

        arc(ctx, cx, cy, radius, a0, a1, dir);
    }

    void close_path(Context* ctx)
    {
        float vals[]{ static_cast<float>(Commands::Close) };
        detail::append_commands(ctx, vals, std::size(vals));
    }

    void path_winding(Context* ctx, const Solidity dir)
    {
        float vals[] = { static_cast<float>(Commands::Winding), static_cast<float>(dir) };
        detail::append_commands(ctx, vals, std::size(vals));
    }

    void barc(Context* ctx, const float cx, const float cy, const float r, const float a0,
              const float a1, const ShapeWinding dir, const int32_t join)
    {
        float px = 0.0f;
        float py = 0.0f;
        float ptanx = 0.0f;
        float ptany = 0.0f;
        float vals[3 + 5 * 7 + 100] = { 0.0f };
        const Commands move = (join != 0 && ctx->ncommands > 0) ? Commands::LineTo
                                                                : Commands::MoveTo;

        // Clamp angles
        float da = a1 - a0;
        if (dir == ShapeWinding::Clockwise)
        {
            if (detail::absf(da) >= std::numbers::pi_v<f32> * 2)
                da = std::numbers::pi_v<f32> * 2;
            else
                while (da < 0.0f)
                    da += std::numbers::pi_v<f32> * 2;
        }
        else
        {
            if (detail::absf(da) >= std::numbers::pi_v<f32> * 2)
                da = -std::numbers::pi_v<f32> * 2;
            else
                while (da > 0.0f)
                    da -= std::numbers::pi_v<f32> * 2;
        }

        // Split arc into max 90 degree segments.
        const int32_t ndivs = detail::max(
            1, detail::min(static_cast<int32_t>(
                               std::round(detail::absf(da) / (std::numbers::pi_v<f32> * 0.5f))),
                           5));

        const float hda = da / static_cast<float>(ndivs) / 2.0f;
        float kappa = detail::absf(4.0f / 3.0f * (1.0f - detail::cosf(hda)) / detail::sinf(hda));

        if (dir == ShapeWinding::CounterClockwise)
            kappa = -kappa;

        int32_t nvals = 0;
        for (int32_t i = 0; i <= ndivs; i++)
        {
            const float a = a0 + da * (static_cast<float>(i) / static_cast<float>(ndivs));
            const float dx = detail::cosf(a);
            const float dy = detail::sinf(a);
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
                vals[nvals++] = static_cast<float>(Commands::Bezierto);
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

        detail::append_commands(ctx, vals, nvals);
    }

    void arc(Context* ctx, const float cx, const float cy, const float r, const float a0,
             const float a1, const ShapeWinding dir)
    {
        barc(ctx, cx, cy, r, a0, a1, dir, 1);
    }

    void rect(Context* ctx, const float x, const float y, const float w, const float h)
    {
        auto vals = std::array{
            static_cast<f32>(Commands::MoveTo), x,     y,
            static_cast<f32>(Commands::LineTo), x,     y + h,
            static_cast<f32>(Commands::LineTo), x + w, y + h,
            static_cast<f32>(Commands::LineTo), x + w, y,
            static_cast<f32>(Commands::Close),
        };

        detail::append_commands(ctx, vals.data(), std::size(vals));
    }

    void rounded_rect(Context* ctx, const float x, const float y, const float w, const float h,
                      const float r)
    {
        rounded_rect_varying(ctx, x, y, w, h, r, r, r, r);
    }

    void rounded_rect(Context* ctx, const ds::rect<f32>& rect, const float radius)
    {
        rounded_rect_varying(ctx, rect.pt.x, rect.pt.y, rect.size.width, rect.size.height, radius,
                             radius, radius, radius);
    }

    void rounded_rect_varying(Context* ctx, const float x, const float y, const float w,
                              const float h, const float rad_top_left, const float rad_top_right,
                              const float rad_bottom_right, const float rad_bottom_left)
    {
        if (rad_top_left < 0.1f && rad_top_right < 0.1f && rad_bottom_right < 0.1f &&
            rad_bottom_left < 0.1f)
        {
            rect(ctx, x, y, w, h);
            return;
        }

        const float halfw = detail::absf(w) * 0.5f;
        const float halfh = detail::absf(h) * 0.5f;
        const float rx_bl = detail::min(rad_bottom_left, halfw) * detail::signf(w);
        const float ry_bl = detail::min(rad_bottom_left, halfh) * detail::signf(h);
        const float rx_br = detail::min(rad_bottom_right, halfw) * detail::signf(w);
        const float ry_br = detail::min(rad_bottom_right, halfh) * detail::signf(h);
        const float rx_tr = detail::min(rad_top_right, halfw) * detail::signf(w);
        const float ry_tr = detail::min(rad_top_right, halfh) * detail::signf(h);
        const float rx_tl = detail::min(rad_top_left, halfw) * detail::signf(w);
        const float ry_tl = detail::min(rad_top_left, halfh) * detail::signf(h);

        auto vals = std::array{
            static_cast<float>(Commands::MoveTo),
            x,
            y + ry_tl,
            static_cast<float>(Commands::LineTo),
            x,
            y + h - ry_bl,
            static_cast<float>(Commands::Bezierto),
            x,
            y + h - ry_bl * (1 - NVGKappa90),
            x + rx_bl * (1 - NVGKappa90),
            y + h,
            x + rx_bl,
            y + h,
            static_cast<float>(Commands::LineTo),
            x + w - rx_br,
            y + h,
            static_cast<float>(Commands::Bezierto),
            x + w - rx_br * (1 - NVGKappa90),
            y + h,
            x + w,
            y + h - ry_br * (1 - NVGKappa90),
            x + w,
            y + h - ry_br,
            static_cast<float>(Commands::LineTo),
            x + w,
            y + ry_tr,
            static_cast<float>(Commands::Bezierto),
            x + w,
            y + ry_tr * (1 - NVGKappa90),
            x + w - rx_tr * (1 - NVGKappa90),
            y,
            x + w - rx_tr,
            y,
            static_cast<float>(Commands::LineTo),
            x + rx_tl,
            y,
            static_cast<float>(Commands::Bezierto),
            x + rx_tl * (1 - NVGKappa90),
            y,
            x,
            y + ry_tl * (1 - NVGKappa90),
            x,
            y + ry_tl,
            static_cast<float>(Commands::Close),
        };

        detail::append_commands(ctx, vals.data(), vals.size());
    }

    void ellipse(Context* ctx, const float cx, const float cy, const float rx, const float ry)
    {
        auto vals = std::array{
            static_cast<float>(Commands::MoveTo),
            cx - rx,
            cy,
            static_cast<float>(Commands::Bezierto),
            cx - rx,
            cy + ry * NVGKappa90,
            cx - rx * NVGKappa90,
            cy + ry,
            cx,
            cy + ry,
            static_cast<float>(Commands::Bezierto),
            cx + rx * NVGKappa90,
            cy + ry,
            cx + rx,
            cy + ry * NVGKappa90,
            cx + rx,
            cy,
            static_cast<float>(Commands::Bezierto),
            cx + rx,
            cy - ry * NVGKappa90,
            cx + rx * NVGKappa90,
            cy - ry,
            cx,
            cy - ry,
            static_cast<float>(Commands::Bezierto),
            cx - rx * NVGKappa90,
            cy - ry,
            cx - rx,
            cy - ry * NVGKappa90,
            cx - rx,
            cy,
            static_cast<float>(Commands::Close),
        };

        detail::append_commands(ctx, vals.data(), std::size(vals));
    }

    void circle(Context* ctx, const float cx, const float cy, const float r)
    {
        ellipse(ctx, cx, cy, r, r);
    }

    void debug_dump_path_cache(const Context* ctx)
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

    void fill(Context* ctx)
    {
        const State* state = detail::get_state(ctx);
        PaintStyle fill_paint = state->fill;

        detail::flatten_paths(ctx);
        if (ctx->params.edge_anti_alias && state->shape_anti_alias)
            detail::expand_fill(ctx, ctx->fringe_width, LineCap::Miter, 2.4f);
        else
            detail::expand_fill(ctx, 0.0f, LineCap::Miter, 2.4f);

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

    void stroke(Context* ctx)
    {
        const State* state = detail::get_state(ctx);
        const float scale = detail::get_average_scale(state->xform);
        float stroke_width = detail::clampf(state->stroke_width * scale, 0.0f, 200.0f);
        PaintStyle stroke_paint = state->stroke;

        if (stroke_width < ctx->fringe_width)
        {
            // If the stroke width is less than pixel size, use alpha to emulate coverage.
            // Since coverage is area, scale by alpha*alpha.
            const float alpha = detail::clampf(stroke_width / ctx->fringe_width, 0.0f, 1.0f);
            stroke_paint.inner_color.a *= alpha * alpha;
            stroke_paint.outer_color.a *= alpha * alpha;
            stroke_width = ctx->fringe_width;
        }

        // Apply global alpha
        stroke_paint.inner_color.a *= state->alpha;
        stroke_paint.outer_color.a *= state->alpha;

        detail::flatten_paths(ctx);

        if (ctx->params.edge_anti_alias && state->shape_anti_alias)
            detail::expand_stroke(ctx, stroke_width * 0.5f, ctx->fringe_width, state->line_cap,
                                  state->line_join, state->miter_limit);
        else
            detail::expand_stroke(ctx, stroke_width * 0.5f, 0.0f, state->line_cap, state->line_join,
                                  state->miter_limit);

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
    int32_t create_font(const Context* ctx, const char* name, const char* filename)
    {
        return fons_add_font(ctx->fs, name, filename, 0);
    }

    int32_t create_font_at_index(const Context* ctx, const char* name, const char* filename,
                                 const int32_t font_index)
    {
        return fons_add_font(ctx->fs, name, filename, font_index);
    }

    int32_t create_font_mem(const Context* ctx, const char* name, uint8_t* data,
                            const int32_t ndata, const int32_t free_data)
    {
        return fons_add_font_mem(ctx->fs, name, data, ndata, free_data, 0);
    }

    i32 create_font_mem(const Context* ctx, const std::string_view& name,
                        const std::basic_string_view<u8>& font_data) noexcept
    {
        constexpr static i32 font_index{ 0 };
        constexpr static i32 dealloc_data{ false };

        return fons_add_font_mem(ctx->fs, name.data(), const_cast<uint8_t*>(font_data.data()),
                                 static_cast<i32>(font_data.size()), dealloc_data, font_index);
    }

    int32_t create_font_mem_at_index(const Context* ctx, const char* name, uint8_t* data,
                                     const int32_t ndata, const int32_t free_data,
                                     const int32_t font_index)
    {
        return fons_add_font_mem(ctx->fs, name, data, ndata, free_data, font_index);
    }

    int32_t find_font(const Context* ctx, const char* name)
    {
        if (name == nullptr)
            return -1;
        return fons_get_font_by_name(ctx->fs, name);
    }

    int32_t add_fallback_font_id(const Context* ctx, const int32_t base_font,
                                 const int32_t fallback_font)
    {
        if (base_font == -1 || fallback_font == -1)
            return 0;
        return fons_add_fallback_font(ctx->fs, base_font, fallback_font);
    }

    int32_t add_fallback_font(const Context* ctx, const char* base_font, const char* fallback_font)
    {
        return add_fallback_font_id(ctx, find_font(ctx, base_font), find_font(ctx, fallback_font));
    }

    void reset_fallback_fonts_id(const Context* ctx, const int32_t base_font)
    {
        fons_reset_fallback_font(ctx->fs, base_font);
    }

    void reset_fallback_fonts(const Context* ctx, const char* base_font)
    {
        reset_fallback_fonts_id(ctx, find_font(ctx, base_font));
    }

    // State setting
    void font_size(Context* ctx, const float size)
    {
        State* state = detail::get_state(ctx);
        state->font_size = size;
    }

    void font_blur(Context* ctx, const float blur)
    {
        State* state = detail::get_state(ctx);
        state->font_blur = blur;
    }

    void text_letter_spacing(Context* ctx, const float spacing)
    {
        State* state = detail::get_state(ctx);
        state->letter_spacing = spacing;
    }

    void text_line_height(Context* ctx, const float line_height)
    {
        State* state = detail::get_state(ctx);
        state->line_height = line_height;
    }

    void text_align(Context* ctx, const Align align)
    {
        State* state = detail::get_state(ctx);
        state->text_align = align;
    }

    void font_face_id(Context* ctx, const int32_t font)
    {
        State* state = detail::get_state(ctx);
        state->font_id = font;
    }

    void font_face(Context* ctx, const char* font)
    {
        State* state = detail::get_state(ctx);
        state->font_id = fons_get_font_by_name(ctx->fs, font);
    }

    void font_face(Context* ctx, const std::string_view& font)
    {
        State* state{ detail::get_state(ctx) };
        state->font_id = fons_get_font_by_name(ctx->fs, font.data());
    }

    void font_face(Context* ctx, const std::string& font)
    {
        State* state{ detail::get_state(ctx) };
        state->font_id = fons_get_font_by_name(ctx->fs, font.c_str());
    }

    float text(Context* ctx, float x, float y, const char* string, const char* end /*= nullptr*/)
    {
        State* state = detail::get_state(ctx);
        FONStextIter iter, prev_iter;
        FONSquad q;
        Vertex* verts;
        float scale = detail::get_font_scale(state) * ctx->device_px_ratio;
        float invscale = 1.0f / scale;
        int32_t cverts = 0;
        int32_t nverts = 0;
        int32_t is_flipped = detail::is_transform_flipped(state->xform);

        if (end == nullptr)
            end = string + strlen(string);

        if (state->font_id == FONS_INVALID)
            return x;

        fons_set_size(ctx->fs, state->font_size * scale);
        fons_set_spacing(ctx->fs, state->letter_spacing * scale);
        fons_set_blur(ctx->fs, state->font_blur * scale);
        fons_set_align(ctx->fs, std::to_underlying(state->text_align));
        fons_set_font(ctx->fs, state->font_id);

        cverts = detail::max(2, static_cast<int32_t>(end - string)) * 6;  // conservative
                                                                          // estimate.
        verts = detail::alloc_temp_verts(ctx, cverts);
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
                    detail::render_text(ctx, verts, nverts);
                    nverts = 0;
                }

                if (detail::alloc_text_atlas(ctx) == 0)
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
                detail::vset(&verts[nverts], c[0], c[1], q.s0, q.t0);
                nverts++;
                detail::vset(&verts[nverts], c[4], c[5], q.s1, q.t1);
                nverts++;
                detail::vset(&verts[nverts], c[2], c[3], q.s1, q.t0);
                nverts++;
                detail::vset(&verts[nverts], c[0], c[1], q.s0, q.t0);
                nverts++;
                detail::vset(&verts[nverts], c[6], c[7], q.s0, q.t1);
                nverts++;
                detail::vset(&verts[nverts], c[4], c[5], q.s1, q.t1);
                nverts++;
            }
        }

        // TODO: add back-end bit to do this just once per frame.
        detail::flush_text_texture(ctx);
        detail::render_text(ctx, verts, nverts);

        return iter.nextx / scale;
    }

    void text_box(Context* ctx, const float x, float y, const float break_row_width,
                  const char* string, const char* end /*= nullptr*/)
    {
        State* state = detail::get_state(ctx);
        TextRow rows[2];
        const Align old_align = state->text_align;
        const Align haling = state->text_align & (Align::HLeft | Align::HCenter | Align::HRight);
        const Align valign = state->text_align &
                             (Align::VTop | Align::VMiddle | Align::VBottom | Align::VBaseline);
        float lineh = 0;

        if (state->font_id == FONS_INVALID)
            return;

        text_metrics(ctx, nullptr, nullptr, &lineh);

        state->text_align = Align::HLeft | valign;

        int32_t nrows;
        while ((nrows = text_break_lines(ctx, string, end, break_row_width, rows, 2)))
        {
            for (int32_t i = 0; i < nrows; i++)
            {
                const TextRow* row = &rows[i];
                if ((haling & Align::HLeft) != 0)
                    text(ctx, x, y, row->start, row->end);
                else if ((haling & Align::HCenter) != 0)
                    text(ctx, x + break_row_width * 0.5f - row->width * 0.5f, y, row->start,
                         row->end);
                else if ((haling & Align::HRight) != 0)
                    text(ctx, x + break_row_width - row->width, y, row->start, row->end);
                y += lineh * state->line_height;
            }
            string = rows[nrows - 1].next;
        }

        state->text_align = old_align;
    }

    int32_t text_glyph_positions(Context* ctx, float x, float y, const char* string,
                                 const char* end, GlyphPosition* positions, int32_t max_positions)
    {
        State* state = detail::get_state(ctx);
        float scale = detail::get_font_scale(state) * ctx->device_px_ratio;
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
        fons_set_align(ctx->fs, std::to_underlying(state->text_align));
        fons_set_font(ctx->fs, state->font_id);

        fons_text_iter_init(ctx->fs, &iter, x * scale, y * scale, string, end,
                            FonsGlyphBitmapOptional);
        prev_iter = iter;
        while (fons_text_iter_next(ctx->fs, &iter, &q))
        {
            if (iter.prev_glyph_index < 0 && detail::alloc_text_atlas(ctx))
            {
                // can not retrieve glyph?
                iter = prev_iter;
                fons_text_iter_next(ctx->fs, &iter, &q);  // try again
            }
            prev_iter = iter;
            positions[npos].str = iter.str;
            positions[npos].x = iter.x * invscale;
            positions[npos].min_x = detail::min(iter.x, q.x0) * invscale;
            positions[npos].max_x = detail::max(iter.nextx, q.x1) * invscale;
            npos++;
            if (npos >= max_positions)
                break;
        }

        return npos;
    }

    int32_t text_break_lines(Context* ctx, const char* string, const char* end,
                             float break_row_width, TextRow* rows, int32_t max_rows)
    {
        State* state = detail::get_state(ctx);
        float scale = detail::get_font_scale(state) * ctx->device_px_ratio;
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
        fons_set_align(ctx->fs, std::to_underlying(state->text_align));
        fons_set_font(ctx->fs, state->font_id);

        break_row_width *= scale;

        fons_text_iter_init(ctx->fs, &iter, 0, 0, string, end, FonsGlyphBitmapOptional);
        prev_iter = iter;
        while (fons_text_iter_next(ctx->fs, &iter, &q))
        {
            if (iter.prev_glyph_index < 0 && detail::alloc_text_atlas(ctx))
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
                        type = CJKChar;
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
                rows[nrows].min_x = row_min_x * invscale;
                rows[nrows].max_x = row_max_x * invscale;
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
                if (type == Char || type == CJKChar)
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
                if (type == Char || type == CJKChar)
                {
                    row_end = iter.next;
                    row_width = iter.nextx - row_start_x;
                    row_max_x = q.x1 - row_start_x;
                }
                // track last end of a word
                if (((ptype == Char || ptype == CJKChar) && type == Space) || type == CJKChar)
                {
                    break_end = iter.str;
                    break_width = row_width;
                    break_max_x = row_max_x;
                }
                // track last beginning of a word
                if ((ptype == Space && (type == Char || type == CJKChar)) || type == CJKChar)
                {
                    word_start = iter.str;
                    word_start_x = iter.x;
                    word_min_x = q.x0;
                }

                // Break to new line when a character is beyond break width.
                if ((type == Char || type == CJKChar) && next_width > break_row_width)
                {
                    // The run length is too long, need to break to new line.
                    if (break_end == row_start)
                    {
                        // The current word is longer than the row length, just break it from here.
                        rows[nrows].start = row_start;
                        rows[nrows].end = iter.str;
                        rows[nrows].width = row_width * invscale;
                        rows[nrows].min_x = row_min_x * invscale;
                        rows[nrows].max_x = row_max_x * invscale;
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
                        rows[nrows].min_x = row_min_x * invscale;
                        rows[nrows].max_x = break_max_x * invscale;
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
            rows[nrows].min_x = row_min_x * invscale;
            rows[nrows].max_x = row_max_x * invscale;
            rows[nrows].next = end;
            nrows++;
        }

        return nrows;
    }

    float text_bounds(Context* ctx, const float x, const float y, const char* text,
                      const char* end /*= nullptr*/, float* bounds /*= nullptr*/)
    {
        const State* state = detail::get_state(ctx);
        const float scale = detail::get_font_scale(state) * ctx->device_px_ratio;
        const float invscale = 1.0f / scale;

        if (state->font_id == FONS_INVALID)
            return 0;

        fons_set_size(ctx->fs, state->font_size * scale);
        fons_set_spacing(ctx->fs, state->letter_spacing * scale);
        fons_set_blur(ctx->fs, state->font_blur * scale);
        fons_set_align(ctx->fs, std::to_underlying(state->text_align));
        fons_set_font(ctx->fs, state->font_id);

        const float width = fons_text_bounds(ctx->fs, x * scale, y * scale, text, end, bounds);
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

    float text_bounds(Context* ctx, ds::point<f32>&& pos, std::string&& text)
    {
        return nvg::text_bounds(ctx, pos.x, pos.y, text.data());
    }

    void text_box_bounds(Context* ctx, const float x, float y, const float break_row_width,
                         const char* string, const char* end, float* bounds)
    {
        State* state = detail::get_state(ctx);
        TextRow rows[2];
        const float scale = detail::get_font_scale(state) * ctx->device_px_ratio;
        const float invscale = 1.0f / scale;
        const Align old_align = state->text_align;
        const Align haling = state->text_align & (Align::HLeft | Align::HCenter | Align::HRight);
        const Align valign = state->text_align &
                             (Align::VTop | Align::VMiddle | Align::VBottom | Align::VBaseline);
        float lineh = 0, rminy = 0, rmaxy = 0;
        float maxx, maxy;

        if (state->font_id == FONS_INVALID)
        {
            if (bounds != nullptr)
                bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0f;
            return;
        }

        text_metrics(ctx, nullptr, nullptr, &lineh);

        state->text_align = Align::HLeft | valign;

        float minx = maxx = x;
        float miny = maxy = y;

        fons_set_size(ctx->fs, state->font_size * scale);
        fons_set_spacing(ctx->fs, state->letter_spacing * scale);
        fons_set_blur(ctx->fs, state->font_blur * scale);
        fons_set_align(ctx->fs, std::to_underlying(state->text_align));
        fons_set_font(ctx->fs, state->font_id);
        fons_line_bounds(ctx->fs, 0, &rminy, &rmaxy);
        rminy *= invscale;
        rmaxy *= invscale;

        int32_t nrows;
        while ((nrows = text_break_lines(ctx, string, end, break_row_width, rows, 2)))
        {
            for (int32_t i = 0; i < nrows; i++)
            {
                const TextRow* row = &rows[i];
                float dx = 0;

                // Horizontal bounds
                if ((haling & Align::HLeft) != 0)
                    dx = 0;
                else if ((haling & Align::HCenter) != 0)
                    dx = break_row_width * 0.5f - row->width * 0.5f;
                else if ((haling & Align::HRight) != 0)
                    dx = break_row_width - row->width;

                const float rminx = x + row->min_x + dx;
                const float rmaxx = x + row->max_x + dx;

                minx = detail::min(minx, rminx);
                maxx = detail::max(maxx, rmaxx);

                // Vertical bounds.
                miny = detail::min(miny, y + rminy);
                maxy = detail::max(maxy, y + rmaxy);

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

    void text_metrics(Context* ctx, float* ascender, float* descender, float* lineh)
    {
        const State* state = detail::get_state(ctx);
        const float scale = detail::get_font_scale(state) * ctx->device_px_ratio;
        const float invscale = 1.0f / scale;

        if (state->font_id == FONS_INVALID)
            return;

        fons_set_size(ctx->fs, state->font_size * scale);
        fons_set_spacing(ctx->fs, state->letter_spacing * scale);
        fons_set_blur(ctx->fs, state->font_blur * scale);
        fons_set_align(ctx->fs, std::to_underlying(state->text_align));
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
