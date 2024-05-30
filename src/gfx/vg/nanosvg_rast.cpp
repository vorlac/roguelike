
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <numbers>

#include "gfx/vg/nanosvg.hpp"
#include "gfx/vg/nanosvg_rast.hpp"
#include "utils/numeric.hpp"

namespace rl::nvg::svg {
    constexpr i32 NSVG_SUBSAMPLES{ 5 };
    constexpr i32 NSVG_FIXSHIFT{ 10 };
    constexpr i32 NSVG_FIX{ 1 << NSVG_FIXSHIFT };
    constexpr i32 NSVG_FIXMASK{ NSVG_FIX - 1 };
    constexpr i32 NSVG_MEMPAGE_SIZE{ 1024 };

    struct NSVGedge
    {
        f32 x0{};
        f32 y0{};
        f32 x1{};
        f32 y1{};
        i32 dir{};
        NSVGedge* next{ nullptr };
    };

    struct NSVGpoint
    {
        f32 x{}, y{};
        f32 dx{}, dy{};
        f32 len{};
        f32 dmx{}, dmy{};
        u8 flags{};
    };

    struct NSVGactiveEdge
    {
        i32 x{}, dx{};
        f32 ey{};
        i32 dir{};
        NSVGactiveEdge* next{ nullptr };
    };

    struct NSVGmemPage
    {
        u8 mem[NSVG_MEMPAGE_SIZE]{};
        i32 size{};
        NSVGmemPage* next{ nullptr };
    };

    struct NSVGcachedPaint
    {
        signed char type{};
        char spread{};
        f32 xform[6]{};
        i32 colors[256]{};
    };

    struct NSVGrasterizer
    {
        f32 px{}, py{};

        f32 tess_tol{};
        f32 dist_tol{};

        NSVGedge* edges{ nullptr };
        i32 nedges{};
        i32 cedges{};

        NSVGpoint* points{ nullptr };
        i32 npoints{};
        i32 cpoints{};

        NSVGpoint* points2{ nullptr };
        i32 npoints2{};
        i32 cpoints2{};

        NSVGactiveEdge* freelist{ nullptr };
        NSVGmemPage* pages{ nullptr };
        NSVGmemPage* curpage{ nullptr };

        u8* scanline{ nullptr };
        i32 cscanline{};

        u8* bitmap{ nullptr };
        i32 width{}, height{}, stride{};
    };

    NSVGrasterizer* nsvg_create_rasterizer()
    {
        const auto r = static_cast<NSVGrasterizer*>(malloc(sizeof(NSVGrasterizer)));
        if (r != nullptr) {
            std::memset(r, 0, sizeof(NSVGrasterizer));

            r->tess_tol = 0.25f;
            r->dist_tol = 0.01f;

            return r;
        }

        // error:
        nsvg_delete_rasterizer(r);
        return nullptr;
    }

    void nsvg_delete_rasterizer(NSVGrasterizer* r)
    {
        if (r == nullptr)
            return;

        NSVGmemPage* p = r->pages;
        while (p != nullptr) {
            NSVGmemPage* next = p->next;
            free(p);
            p = next;
        }

        if (r->edges)
            free(r->edges);
        if (r->points)
            free(r->points);
        if (r->points2)
            free(r->points2);
        if (r->scanline)
            free(r->scanline);

        free(r);
    }

    namespace {
        enum NSVGpointFlags {
            NSVGPntCorner = 0x01,
            NSVGPntBevel = 0x02,
            NSVGPntLeft = 0x04
        };

        NSVGmemPage* nsvg_next_page(NSVGrasterizer* r, NSVGmemPage* cur)
        {
            // If using existing chain, return the next page in chain
            if (cur != nullptr && cur->next != nullptr)
                return cur->next;

            // Alloc new page
            const auto newp = static_cast<NSVGmemPage*>(malloc(sizeof(NSVGmemPage)));
            if (newp == nullptr)
                return nullptr;

            std::memset(newp, 0, sizeof(NSVGmemPage));

            // Add to linked list
            if (cur != nullptr)
                cur->next = newp;
            else
                r->pages = newp;

            return newp;
        }

        void nsvg_reset_pool(NSVGrasterizer* r)
        {
            NSVGmemPage* p = r->pages;
            while (p != nullptr) {
                p->size = 0;
                p = p->next;
            }

            r->curpage = r->pages;
        }

        u8* nsvg_alloc(NSVGrasterizer* r, const i32 size)
        {
            if (size > NSVG_MEMPAGE_SIZE)
                return nullptr;

            if (r->curpage == nullptr || r->curpage->size + size > NSVG_MEMPAGE_SIZE)
                r->curpage = nsvg_next_page(r, r->curpage);

            u8* buf = &r->curpage->mem[r->curpage->size];
            r->curpage->size += size;

            return buf;
        }

        i32 nsvg_pt_equals(const f32 x1, const f32 y1, const f32 x2, const f32 y2, const f32 tol)
        {
            const f32 dx = x2 - x1;
            const f32 dy = y2 - y1;
            return dx * dx + dy * dy < tol * tol;
        }

        void nsvg_add_path_point(NSVGrasterizer* r, const f32 x, const f32 y, const i32 flags)
        {
            NSVGpoint* pt;

            if (r->npoints > 0) {
                pt = &r->points[r->npoints - 1];
                if (nsvg_pt_equals(pt->x, pt->y, x, y, r->dist_tol)) {
                    pt->flags = static_cast<u8>(pt->flags | flags);
                    return;
                }
            }

            if (r->npoints + 1 > r->cpoints) {
                r->cpoints = r->cpoints > 0 ? r->cpoints * 2 : 64;
                r->points = static_cast<NSVGpoint*>(
                    std::realloc(r->points, sizeof(NSVGpoint) * r->cpoints));

                if (r->points == nullptr)
                    return;
            }

            pt = &r->points[r->npoints];
            pt->x = x;
            pt->y = y;
            pt->flags = static_cast<u8>(flags);
            r->npoints++;
        }

        void nsvg_append_path_point(NSVGrasterizer* r, const NSVGpoint& pt)
        {
            if (r->npoints + 1 > r->cpoints) {
                r->cpoints = r->cpoints > 0 ? r->cpoints * 2 : 64;
                r->points = static_cast<NSVGpoint*>(
                    std::realloc(r->points, sizeof(NSVGpoint) * r->cpoints));

                if (r->points == nullptr)
                    return;
            }
            r->points[r->npoints] = pt;
            r->npoints++;
        }

        void nsvg_duplicate_points(NSVGrasterizer* r)
        {
            if (r->npoints > r->cpoints2) {
                r->cpoints2 = r->npoints;
                r->points2 = static_cast<NSVGpoint*>(
                    std::realloc(r->points2, sizeof(NSVGpoint) * r->cpoints2));
                if (r->points2 == nullptr)
                    return;
            }

            std::memcpy(r->points2, r->points, sizeof(NSVGpoint) * r->npoints);
            r->npoints2 = r->npoints;
        }

        void nsvg_add_edge(NSVGrasterizer* r, const f32 x0, const f32 y0, const f32 x1, const f32 y1)
        {
            // Skip horizontal edges
            if (std::fabs(y0 - y1) < std::numeric_limits<f32>::epsilon())
                return;

            if (r->nedges + 1 > r->cedges) {
                r->cedges = r->cedges > 0 ? r->cedges * 2 : 64;
                r->edges = static_cast<NSVGedge*>(
                    std::realloc(r->edges, sizeof(NSVGedge) * r->cedges));
                if (r->edges == nullptr)
                    return;
            }

            NSVGedge* e = &r->edges[r->nedges];
            r->nedges++;

            if (y0 < y1) {
                e->x0 = x0;
                e->y0 = y0;
                e->x1 = x1;
                e->y1 = y1;
                e->dir = 1;
            }
            else {
                e->x0 = x1;
                e->y0 = y1;
                e->x1 = x0;
                e->y1 = y0;
                e->dir = -1;
            }
        }

        f32 nsvg_normalize(f32* x, f32* y)
        {
            const f32 d = std::sqrtf((*x) * (*x) + (*y) * (*y));
            if (d > 1e-6f) {
                const f32 id = 1.0f / d;
                *x *= id;
                *y *= id;
            }

            return d;
        }

        f32 nsvg_absf(const f32 x)
        {
            return x < 0 ? -x : x;
        }

        void nsvg_flatten_cubic_bez(NSVGrasterizer* r, const f32 x1, const f32 y1, const f32 x2,
                                    const f32 y2, const f32 x3, const f32 y3, const f32 x4,
                                    const f32 y4, i32 level, const i32 type)
        {
            constexpr i32 max_level = 10;

            if (level > max_level)
                return;

            const f32 x12 = (x1 + x2) * 0.5f;
            const f32 y12 = (y1 + y2) * 0.5f;
            const f32 x23 = (x2 + x3) * 0.5f;
            const f32 y23 = (y2 + y3) * 0.5f;
            const f32 x34 = (x3 + x4) * 0.5f;
            const f32 y34 = (y3 + y4) * 0.5f;
            const f32 x123 = (x12 + x23) * 0.5f;
            const f32 y123 = (y12 + y23) * 0.5f;

            const f32 dx = x4 - x1;
            const f32 dy = y4 - y1;
            const f32 d2 = nsvg_absf((x2 - x4) * dy - (y2 - y4) * dx);
            const f32 d3 = nsvg_absf((x3 - x4) * dy - (y3 - y4) * dx);

            if ((d2 + d3) * (d2 + d3) < r->tess_tol * (dx * dx + dy * dy)) {
                nsvg_add_path_point(r, x4, y4, type);
                return;
            }

            ++level;
            if (level > max_level)
                return;

            const f32 x234 = (x23 + x34) * 0.5f;
            const f32 y234 = (y23 + y34) * 0.5f;
            const f32 x1234 = (x123 + x234) * 0.5f;
            const f32 y1234 = (y123 + y234) * 0.5f;

            nsvg_flatten_cubic_bez(r, x1, y1, x12, y12, x123, y123, x1234, y1234, level, 0);
            nsvg_flatten_cubic_bez(r, x1234, y1234, x234, y234, x34, y34, x4, y4, level, type);
        }

        void nsvg_flatten_shape(NSVGrasterizer* r, const nvg::svg::NSVGshape* shape, const f32 sx,
                                const f32 sy)
        {
            for (const nvg::svg::NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
                r->npoints = 0;

                // Flatten path
                nsvg_add_path_point(r, path->pts[0] * sx, path->pts[1] * sy, 0);
                for (i32 i = 0; i < path->npts - 1; i += 3) {
                    const f32* p = &path->pts[static_cast<ptrdiff_t>(i * 2)];
                    nsvg_flatten_cubic_bez(r, p[0] * sx, p[1] * sy, p[2] * sx, p[3] * sy, p[4] * sx,
                                           p[5] * sy, p[6] * sx, p[7] * sy, 0, 0);
                }

                // Close path
                nsvg_add_path_point(r, path->pts[0] * sx, path->pts[1] * sy, 0);

                // Build edges
                for (i32 i = 0, j = r->npoints - 1; i < r->npoints; j = i++)
                    nsvg_add_edge(r, r->points[j].x, r->points[j].y, r->points[i].x, r->points[i].y);
            }
        }

        void nsvg_init_closed(NSVGpoint* left, NSVGpoint* right, const NSVGpoint* p0,
                              const NSVGpoint* p1, const f32 line_width)
        {
            f32 dx = p1->x - p0->x;
            f32 dy = p1->y - p0->y;
            const f32 len = nsvg_normalize(&dx, &dy);

            const f32 px = p0->x + dx * len * 0.5f;
            const f32 py = p0->y + dy * len * 0.5f;
            const f32 dlx = dy, dly = -dx;

            const f32 w = line_width * 0.5f;
            const f32 lx = px - dlx * w, ly = py - dly * w;
            const f32 rx = px + dlx * w, ry = py + dly * w;

            left->x = lx;
            left->y = ly;

            right->x = rx;
            right->y = ry;
        }

        void nsvg_butt_cap(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right, const NSVGpoint* p,
                           const f32 dx, const f32 dy, const f32 line_width, const i32 connect)
        {
            const f32 w = line_width * 0.5f;
            const f32 px = p->x, py = p->y;
            const f32 dlx = dy, dly = -dx;
            const f32 lx = px - dlx * w, ly = py - dly * w;
            const f32 rx = px + dlx * w, ry = py + dly * w;

            nsvg_add_edge(r, lx, ly, rx, ry);

            if (connect != 0) {
                nsvg_add_edge(r, left->x, left->y, lx, ly);
                nsvg_add_edge(r, rx, ry, right->x, right->y);
            }

            left->x = lx;
            left->y = ly;

            right->x = rx;
            right->y = ry;
        }

        void nsvg_square_cap(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right,
                             const NSVGpoint* p, const f32 dx, const f32 dy, const f32 line_width,
                             const i32 connect)
        {
            const f32 w = line_width * 0.5f;
            const f32 px = p->x - dx * w, py = p->y - dy * w;
            const f32 dlx = dy, dly = -dx;
            const f32 lx = px - dlx * w, ly = py - dly * w;
            const f32 rx = px + dlx * w, ry = py + dly * w;

            nsvg_add_edge(r, lx, ly, rx, ry);

            if (connect) {
                nsvg_add_edge(r, left->x, left->y, lx, ly);
                nsvg_add_edge(r, rx, ry, right->x, right->y);
            }

            left->x = lx;
            left->y = ly;

            right->x = rx;
            right->y = ry;
        }

        void nsvg_round_cap(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right,
                            const NSVGpoint* p, const f32 dx, const f32 dy, const f32 line_width,
                            const i32 ncap, const i32 connect)
        {
            const f32 w = line_width * 0.5f;
            const f32 px = p->x, py = p->y;
            const f32 dlx = dy, dly = -dx;

            f32 lx = 0;
            f32 ly = 0;
            f32 rx = 0;
            f32 ry = 0;
            f32 prevx = 0;
            f32 prevy = 0;

            for (i32 i = 0; i < ncap; i++) {
                const f32 a = static_cast<f32>(i) / static_cast<f32>(ncap - 1) *
                              std::numbers::pi_v<f32>;

                const f32 ax = cosf(a) * w, ay = sinf(a) * w;
                const f32 x = px - dlx * ax - dx * ay;
                const f32 y = py - dly * ax - dy * ay;

                if (i > 0)
                    nsvg_add_edge(r, prevx, prevy, x, y);

                prevx = x;
                prevy = y;

                if (i == 0) {
                    lx = x;
                    ly = y;
                }
                else if (i == ncap - 1) {
                    rx = x;
                    ry = y;
                }
            }

            if (connect != 0) {
                nsvg_add_edge(r, left->x, left->y, lx, ly);
                nsvg_add_edge(r, rx, ry, right->x, right->y);
            }

            left->x = lx;
            left->y = ly;

            right->x = rx;
            right->y = ry;
        }

        void nsvg_bevel_join(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right,
                             const NSVGpoint* p0, const NSVGpoint* p1, const f32 line_width)
        {
            const f32 w = line_width * 0.5f;
            const f32 dlx0 = p0->dy, dly0 = -p0->dx;
            const f32 dlx1 = p1->dy, dly1 = -p1->dx;
            const f32 lx0 = p1->x - (dlx0 * w), ly0 = p1->y - (dly0 * w);
            const f32 rx0 = p1->x + (dlx0 * w), ry0 = p1->y + (dly0 * w);
            const f32 lx1 = p1->x - (dlx1 * w), ly1 = p1->y - (dly1 * w);
            const f32 rx1 = p1->x + (dlx1 * w), ry1 = p1->y + (dly1 * w);

            nsvg_add_edge(r, lx0, ly0, left->x, left->y);
            nsvg_add_edge(r, lx1, ly1, lx0, ly0);

            nsvg_add_edge(r, right->x, right->y, rx0, ry0);
            nsvg_add_edge(r, rx0, ry0, rx1, ry1);

            left->x = lx1;
            left->y = ly1;
            right->x = rx1;
            right->y = ry1;
        }

        void nsvg_miter_join(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right,
                             const NSVGpoint* p0, const NSVGpoint* p1, const f32 line_width)
        {
            const f32 w = line_width * 0.5f;
            const f32 dlx0 = p0->dy, dly0 = -p0->dx;
            const f32 dlx1 = p1->dy, dly1 = -p1->dx;

            f32 lx1;
            f32 rx1;
            f32 ly1;
            f32 ry1;

            if (p1->flags & NSVGPntLeft) {
                lx1 = p1->x - p1->dmx * w;
                ly1 = p1->y - p1->dmy * w;

                nsvg_add_edge(r, lx1, ly1, left->x, left->y);

                const f32 rx0 = p1->x + (dlx0 * w);
                const f32 ry0 = p1->y + (dly0 * w);

                rx1 = p1->x + (dlx1 * w);
                ry1 = p1->y + (dly1 * w);

                nsvg_add_edge(r, right->x, right->y, rx0, ry0);
                nsvg_add_edge(r, rx0, ry0, rx1, ry1);
            }
            else {
                const f32 lx0 = p1->x - (dlx0 * w);
                const f32 ly0 = p1->y - (dly0 * w);

                lx1 = p1->x - (dlx1 * w);
                ly1 = p1->y - (dly1 * w);

                nsvg_add_edge(r, lx0, ly0, left->x, left->y);
                nsvg_add_edge(r, lx1, ly1, lx0, ly0);

                rx1 = p1->x + p1->dmx * w;
                ry1 = p1->y + p1->dmy * w;

                nsvg_add_edge(r, right->x, right->y, rx1, ry1);
            }

            left->x = lx1;
            left->y = ly1;

            right->x = rx1;
            right->y = ry1;
        }

        void nsvg_round_join(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right,
                             const NSVGpoint* p0, const NSVGpoint* p1, const f32 line_width,
                             const i32 ncap)
        {
            const f32 w = line_width * 0.5f;
            const f32 dlx0 = p0->dy, dly0 = -p0->dx;
            const f32 dlx1 = p1->dy, dly1 = -p1->dx;
            const f32 a0 = std::atan2f(dly0, dlx0);
            const f32 a1 = std::atan2f(dly1, dlx1);

            f32 da = a1 - a0;
            if (da < std::numbers::pi_v<f32>)
                da += std::numbers::pi_v<f32> * 2;
            if (da > std::numbers::pi_v<f32>)
                da -= std::numbers::pi_v<f32> * 2;

            i32 n = static_cast<i32>(
                std::ceilf(nsvg_absf(da) / std::numbers::pi_v<f32> * static_cast<f32>(ncap)));

            if (n < 2)
                n = 2;

            if (n > ncap)
                n = ncap;

            f32 lx = left->x;
            f32 ly = left->y;
            f32 rx = right->x;
            f32 ry = right->y;

            for (i32 i = 0; i < n; i++) {
                const f32 u = static_cast<f32>(i) / static_cast<f32>(n - 1);
                const f32 a = a0 + u * da;
                const f32 ax = cosf(a) * w, ay = sinf(a) * w;
                const f32 lx1 = p1->x - ax, ly1 = p1->y - ay;
                const f32 rx1 = p1->x + ax, ry1 = p1->y + ay;

                nsvg_add_edge(r, lx1, ly1, lx, ly);
                nsvg_add_edge(r, rx, ry, rx1, ry1);

                lx = lx1;
                ly = ly1;
                rx = rx1;
                ry = ry1;
            }

            left->x = lx;
            left->y = ly;

            right->x = rx;
            right->y = ry;
        }

        void nsvg_straight_join(NSVGrasterizer* r, NSVGpoint* left, NSVGpoint* right,
                                const NSVGpoint* p1, const f32 line_width)
        {
            const f32 w = line_width * 0.5f;
            const f32 lx = p1->x - (p1->dmx * w), ly = p1->y - (p1->dmy * w);
            const f32 rx = p1->x + (p1->dmx * w), ry = p1->y + (p1->dmy * w);

            nsvg_add_edge(r, lx, ly, left->x, left->y);
            nsvg_add_edge(r, right->x, right->y, rx, ry);

            left->x = lx;
            left->y = ly;
            right->x = rx;
            right->y = ry;
        }

        i32 nsvg_curve_divs(const f32 r, const f32 arc, const f32 tol)
        {
            const f32 da = std::acosf(r / (r + tol)) * 2.0f;

            i32 divs = static_cast<i32>(ceilf(arc / da));
            if (divs < 2)
                divs = 2;

            return divs;
        }

        void nsvg_expand_stroke(NSVGrasterizer* r, const NSVGpoint* points, const i32 npoints,
                                const i32 closed, const i32 line_join, const i32 line_cap,
                                const f32 line_width)
        {
            const i32 ncap = nsvg_curve_divs(line_width * 0.5f, std::numbers::pi_v<f32>,
                                             r->tess_tol);  // Calculate
            NSVGpoint left = { 0, 0, 0, 0, 0, 0, 0, 0 };
            NSVGpoint right = { 0, 0, 0, 0, 0, 0, 0, 0 };
            NSVGpoint first_left = { 0, 0, 0, 0, 0, 0, 0, 0 };
            NSVGpoint first_right = { 0, 0, 0, 0, 0, 0, 0, 0 };

            const NSVGpoint* p0;
            const NSVGpoint* p1;
            i32 s, e;

            // Build stroke edges
            if (closed) {
                // Looping
                p0 = &points[npoints - 1];
                p1 = &points[0];
                s = 0;
                e = npoints;
            }
            else {
                // Add cap
                p0 = &points[0];
                p1 = &points[1];
                s = 1;
                e = npoints - 1;
            }

            if (closed) {
                nsvg_init_closed(&left, &right, p0, p1, line_width);

                first_left = left;
                first_right = right;
            }
            else {
                // Add cap
                f32 dx = p1->x - p0->x;
                f32 dy = p1->y - p0->y;

                nsvg_normalize(&dx, &dy);

                if (line_cap == nvg::svg::NSVGCapButt)
                    nsvg_butt_cap(r, &left, &right, p0, dx, dy, line_width, 0);
                else if (line_cap == nvg::svg::NSVGCapSquare)
                    nsvg_square_cap(r, &left, &right, p0, dx, dy, line_width, 0);
                else if (line_cap == nvg::svg::NSVGCapRound)
                    nsvg_round_cap(r, &left, &right, p0, dx, dy, line_width, ncap, 0);
            }

            for (i32 j = s; j < e; ++j) {
                if ((p1->flags & NSVGPntCorner) == 0)
                    nsvg_straight_join(r, &left, &right, p1, line_width);
                else if (line_join == nvg::svg::NSVGJoinRound)
                    nsvg_round_join(r, &left, &right, p0, p1, line_width, ncap);
                else if (line_join == nvg::svg::NSVGJoinBevel || (p1->flags & NSVGPntBevel))
                    nsvg_bevel_join(r, &left, &right, p0, p1, line_width);
                else
                    nsvg_miter_join(r, &left, &right, p0, p1, line_width);

                p0 = p1++;
            }

            if (closed) {
                // Loop it
                nsvg_add_edge(r, first_left.x, first_left.y, left.x, left.y);
                nsvg_add_edge(r, right.x, right.y, first_right.x, first_right.y);
            }
            else {
                // Add cap
                f32 dx = p1->x - p0->x;
                f32 dy = p1->y - p0->y;
                nsvg_normalize(&dx, &dy);
                if (line_cap == nvg::svg::NSVGCapButt)
                    nsvg_butt_cap(r, &right, &left, p1, -dx, -dy, line_width, 1);
                else if (line_cap == nvg::svg::NSVGCapSquare)
                    nsvg_square_cap(r, &right, &left, p1, -dx, -dy, line_width, 1);
                else if (line_cap == nvg::svg::NSVGCapRound)
                    nsvg_round_cap(r, &right, &left, p1, -dx, -dy, line_width, ncap, 1);
            }
        }

        void nsvg_prepare_stroke(const NSVGrasterizer* r, const f32 miter_limit,
                                 const char line_join)
        {
            NSVGpoint* p0 = &r->points[r->npoints - 1];
            NSVGpoint* p1 = &r->points[0];
            for (i32 i = 0; i < r->npoints; i++) {
                // Calculate segment direction and length
                p0->dx = p1->x - p0->x;
                p0->dy = p1->y - p0->y;
                p0->len = nsvg_normalize(&p0->dx, &p0->dy);
                // Advance
                p0 = p1++;
            }

            // calculate joins
            p0 = &r->points[r->npoints - 1];
            p1 = &r->points[0];
            for (i32 j = 0; j < r->npoints; j++) {
                const f32 dlx0 = p0->dy;
                const f32 dly0 = -p0->dx;
                const f32 dlx1 = p1->dy;
                const f32 dly1 = -p1->dx;
                // Calculate extrusions
                p1->dmx = (dlx0 + dlx1) * 0.5f;
                p1->dmy = (dly0 + dly1) * 0.5f;
                const f32 dmr2 = p1->dmx * p1->dmx + p1->dmy * p1->dmy;
                if (dmr2 > 0.000001f) {
                    f32 s2 = 1.0f / dmr2;
                    if (s2 > 600.0f)
                        s2 = 600.0f;
                    p1->dmx *= s2;
                    p1->dmy *= s2;
                }

                // Clear flags, but keep the corner.
                p1->flags = (p1->flags & NSVGPntCorner) ? NSVGPntCorner : 0;

                // Keep track of left turns.
                const f32 cross = p1->dx * p0->dy - p0->dx * p1->dy;
                if (cross > 0.0f)
                    p1->flags |= NSVGPntLeft;

                // Check to see if the corner needs to be beveled.
                if (p1->flags & NSVGPntCorner) {
                    if ((dmr2 * miter_limit * miter_limit) < 1.0f ||
                        line_join == nvg::svg::NSVGJoinBevel ||
                        line_join == nvg::svg::NSVGJoinRound) {
                        p1->flags |= NSVGPntBevel;
                    }
                }

                p0 = p1++;
            }
        }

        void nsvg_flatten_shape_stroke(NSVGrasterizer* r, const nvg::svg::NSVGshape* shape,
                                       const f32 sx, const f32 sy)
        {
            i32 j;
            const f32 miter_limit = shape->miter_limit;
            const char line_join = shape->stroke_line_join;
            const char line_cap = shape->stroke_line_cap;
            const f32 sw = (sx + sy) / 2;                     // average scaling factor
            const f32 line_width = shape->stroke_width * sw;  // FIXME (?)

            for (const nvg::svg::NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
                // Flatten path
                r->npoints = 0;
                nsvg_add_path_point(r, path->pts[0] * sx, path->pts[1] * sy, NSVGPntCorner);
                for (i32 i = 0; i < path->npts - 1; i += 3) {
                    const f32* p = &path->pts[static_cast<ptrdiff_t>(i * 2)];
                    nsvg_flatten_cubic_bez(r, p[0] * sx, p[1] * sy, p[2] * sx, p[3] * sy, p[4] * sx,
                                           p[5] * sy, p[6] * sx, p[7] * sy, 0, NSVGPntCorner);
                }
                if (r->npoints < 2)
                    continue;

                char closed = path->closed;

                // If the first and last points are the same, remove the last, mark as closed path.
                const NSVGpoint* p0 = &r->points[r->npoints - 1];
                const NSVGpoint* p1 = &r->points[0];
                if (nsvg_pt_equals(p0->x, p0->y, p1->x, p1->y, r->dist_tol)) {
                    r->npoints--;
                    p0 = &r->points[r->npoints - 1];
                    closed = 1;
                }

                if (shape->stroke_dash_count > 0) {
                    i32 idash = 0;
                    i32 dash_state = 1;
                    f32 total_dist = 0;

                    if (closed)
                        nsvg_append_path_point(r, r->points[0]);

                    // Duplicate points -> points2.
                    nsvg_duplicate_points(r);

                    r->npoints = 0;
                    NSVGpoint cur = r->points2[0];
                    nsvg_append_path_point(r, cur);

                    // Figure out dash offset.
                    f32 all_dash_len = 0;
                    for (j = 0; j < shape->stroke_dash_count; j++)
                        all_dash_len += shape->stroke_dash_array[j];

                    if (shape->stroke_dash_count & 1)
                        all_dash_len *= 2.0f;

                    // Find location inside pattern
                    f32 dash_offset = fmodf(shape->stroke_dash_offset, all_dash_len);
                    if (dash_offset < 0.0f)
                        dash_offset += all_dash_len;

                    while (dash_offset > shape->stroke_dash_array[idash]) {
                        dash_offset -= shape->stroke_dash_array[idash];
                        idash = (idash + 1) % shape->stroke_dash_count;
                    }

                    f32 dash_len = (shape->stroke_dash_array[idash] - dash_offset) * sw;
                    for (j = 1; j < r->npoints2;) {
                        const f32 dx = r->points2[j].x - cur.x;
                        const f32 dy = r->points2[j].y - cur.y;
                        const f32 dist = sqrtf(dx * dx + dy * dy);

                        if ((total_dist + dist) > dash_len) {
                            // Calculate intermediate point
                            const f32 d = (dash_len - total_dist) / dist;
                            const f32 x = cur.x + dx * d;
                            const f32 y = cur.y + dy * d;
                            nsvg_add_path_point(r, x, y, NSVGPntCorner);

                            // Stroke
                            if (r->npoints > 1 && dash_state) {
                                nsvg_prepare_stroke(r, miter_limit, line_join);
                                nsvg_expand_stroke(r, r->points, r->npoints, 0, line_join, line_cap,
                                                   line_width);
                            }

                            // Advance dash pattern
                            dash_state = !dash_state;
                            idash = (idash + 1) % shape->stroke_dash_count;
                            dash_len = shape->stroke_dash_array[idash] * sw;

                            // Restart
                            cur.x = x;
                            cur.y = y;
                            cur.flags = NSVGPntCorner;
                            total_dist = 0.0f;
                            r->npoints = 0;
                            nsvg_append_path_point(r, cur);
                        }
                        else {
                            total_dist += dist;
                            cur = r->points2[j];
                            nsvg_append_path_point(r, cur);
                            j++;
                        }
                    }

                    // Stroke any leftover path
                    if (r->npoints > 1 && dash_state)
                        nsvg_expand_stroke(r, r->points, r->npoints, 0, line_join, line_cap,
                                           line_width);
                }
                else {
                    nsvg_prepare_stroke(r, miter_limit, line_join);
                    nsvg_expand_stroke(r, r->points, r->npoints, closed, line_join, line_cap,
                                       line_width);
                }
            }
        }

        i32 nsvg_cmp_edge(const void* p, const void* q)
        {
            const auto a = static_cast<const NSVGedge*>(p);
            const auto b = static_cast<const NSVGedge*>(q);

            if (a->y0 < b->y0)
                return -1;
            if (a->y0 > b->y0)
                return 1;

            return 0;
        }

        NSVGactiveEdge* nsvg_add_active(NSVGrasterizer* r, const NSVGedge* e, const f32 startPoint)
        {
            NSVGactiveEdge* z;

            if (r->freelist != nullptr) {
                // Restore from freelist.
                z = r->freelist;
                r->freelist = z->next;
            }
            else {
                // Alloc new edge.
                z = reinterpret_cast<NSVGactiveEdge*>(nsvg_alloc(r, sizeof(NSVGactiveEdge)));
                if (z == nullptr)
                    return nullptr;
            }

            const f32 dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
            // round dx down to avoid going too far
            if (dxdy < 0)
                z->dx = static_cast<i32>(-floorf(NSVG_FIX * -dxdy));
            else
                z->dx = static_cast<i32>(floorf(NSVG_FIX * dxdy));

            z->x = static_cast<i32>(floorf(NSVG_FIX * (e->x0 + dxdy * (startPoint - e->y0))));

            //  z->x -= off_x * FIX;
            z->ey = e->y1;
            z->next = nullptr;
            z->dir = e->dir;

            return z;
        }

        void nsvg_free_active(NSVGrasterizer* r, NSVGactiveEdge* z)
        {
            z->next = r->freelist;
            r->freelist = z;
        }

        void nsvg_fill_scanline(u8* scanline, const i32 len, const i32 x0, const i32 x1,
                                const i32 max_weight, i32* xmin, i32* xmax)
        {
            i32 i = x0 >> NSVG_FIXSHIFT;
            i32 j = x1 >> NSVG_FIXSHIFT;

            if (i < *xmin)
                *xmin = i;
            if (j > *xmax)
                *xmax = j;
            if (i < len && j >= 0) {
                if (i == j) {
                    // x0,x1 are the same pixel, so compute combined coverage
                    scanline[i] = static_cast<u8>(
                        scanline[i] + ((x1 - x0) * max_weight >> NSVG_FIXSHIFT));
                }
                else {
                    if (i >= 0)  // add antialiasing for x0
                        scanline[i] = static_cast<u8>(
                            scanline[i] +
                            (((NSVG_FIX - (x0 & NSVG_FIXMASK)) * max_weight) >> NSVG_FIXSHIFT));
                    else
                        i = -1;  // clip

                    if (j < len)  // add antialiasing for x1
                        scanline[j] = static_cast<u8>(
                            scanline[j] + (((x1 & NSVG_FIXMASK) * max_weight) >> NSVG_FIXSHIFT));
                    else
                        j = len;  // clip

                    for (++i; i < j; ++i)  // fill pixels between x0 and x1
                        scanline[i] = static_cast<u8>(scanline[i] + max_weight);
                }
            }
        }

        // note: this routine clips fills that extend off the edges... ideally this
        // wouldn't happen, but it could happen if the truetype glyph bounding boxes
        // are wrong, or if the user supplies a too-small bitmap
        void nsvg_fill_active_edges(u8* scanline, const i32 len, const NSVGactiveEdge* e,
                                    const i32 maxWeight, i32* xmin, i32* xmax, const char fillRule)
        {
            // non-zero winding fill
            i32 x0 = 0, w = 0;

            if (fillRule == nvg::svg::NSVGFillruleNonzero) {
                // Non-zero
                while (e != nullptr) {
                    if (w == 0) {
                        // if we're currently at zero, we need to record the edge start point
                        x0 = e->x;
                        w += e->dir;
                    }
                    else {
                        const i32 x1 = e->x;
                        w += e->dir;
                        // if we went to zero, we need to draw
                        if (w == 0)
                            nsvg_fill_scanline(scanline, len, x0, x1, maxWeight, xmin, xmax);
                    }
                    e = e->next;
                }
            }
            else if (fillRule == nvg::svg::NSVGFillruleEvenodd) {
                // Even-odd
                while (e != nullptr) {
                    if (w == 0) {
                        // if we're currently at zero, we need to record the edge start point
                        x0 = e->x;
                        w = 1;
                    }
                    else {
                        const i32 x1 = e->x;
                        w = 0;
                        nsvg_fill_scanline(scanline, len, x0, x1, maxWeight, xmin, xmax);
                    }
                    e = e->next;
                }
            }
        }

        f32 nsvg_clampf(const f32 a, const f32 mn, const f32 mx)
        {
            return a < mn ? mn : (a > mx ? mx : a);
        }

        i32 nsvg_rgba(const u8 r, const u8 g, const u8 b, const u8 a)
        {
            return static_cast<i32>(r) | (static_cast<i32>(g) << 8) | (static_cast<i32>(b) << 16) |
                   (static_cast<i32>(a) << 24);
        }

        i32 nsvg_lerp_rgba(const i32 c0, const i32 c1, const f32 u)
        {
            const i32 iu = static_cast<i32>(nsvg_clampf(u, 0.0f, 1.0f) * 256.0f);
            const i32 r = (((c0) & 0xff) * (256 - iu) + (((c1) & 0xff) * iu)) >> 8;
            const i32 g = (((c0 >> 8) & 0xff) * (256 - iu) + (((c1 >> 8) & 0xff) * iu)) >> 8;
            const i32 b = (((c0 >> 16) & 0xff) * (256 - iu) + (((c1 >> 16) & 0xff) * iu)) >> 8;
            const i32 a = (((c0 >> 24) & 0xff) * (256 - iu) + (((c1 >> 24) & 0xff) * iu)) >> 8;
            return nsvg_rgba(static_cast<u8>(r), static_cast<u8>(g), static_cast<u8>(b),
                             static_cast<u8>(a));
        }

        i32 nsvg_apply_opacity(const i32 c, const f32 u)
        {
            const i32 iu = static_cast<i32>(nsvg_clampf(u, 0.0f, 1.0f) * 256.0f);
            const i32 r = (c) & 0xff;
            const i32 g = (c >> 8) & 0xff;
            const i32 b = (c >> 16) & 0xff;
            const i32 a = (((c >> 24) & 0xff) * iu) >> 8;
            return nsvg_rgba(static_cast<u8>(r), static_cast<u8>(g), static_cast<u8>(b),
                             static_cast<u8>(a));
        }

        i32 nsvg_div255(const i32 x)
        {
            return ((x + 1) * 257) >> 16;
        }

        void nsvg_scanline_solid(u8* dst, const i32 count, u8* cover, const i32 x, const i32 y,
                                 const f32 tx, const f32 ty, const f32 sx, const f32 sy,
                                 NSVGcachedPaint* cache)
        {
            if (cache->type == nvg::svg::NSVGPaintColor) {
                const i32 cr = (cache->colors[0] & 0xff);
                const i32 cg = (cache->colors[0] >> 8) & 0xff;
                const i32 cb = (cache->colors[0] >> 16) & 0xff;
                const i32 ca = (cache->colors[0] >> 24) & 0xff;

                for (i32 i = 0; i < count; i++) {
                    i32 a = nsvg_div255(static_cast<i32>(cover[0]) * ca);
                    const i32 ia = 255 - a;

                    // Premultiply
                    i32 r = nsvg_div255(cr * a);
                    i32 g = nsvg_div255(cg * a);
                    i32 b = nsvg_div255(cb * a);

                    // Blend over
                    r += nsvg_div255(ia * static_cast<i32>(dst[0]));
                    g += nsvg_div255(ia * static_cast<i32>(dst[1]));
                    b += nsvg_div255(ia * static_cast<i32>(dst[2]));
                    a += nsvg_div255(ia * static_cast<i32>(dst[3]));

                    dst[0] = static_cast<u8>(r);
                    dst[1] = static_cast<u8>(g);
                    dst[2] = static_cast<u8>(b);
                    dst[3] = static_cast<u8>(a);

                    cover++;
                    dst += 4;
                }
            }
            else if (cache->type == nvg::svg::NSVGPaintLinearGradient) {
                /*f32 fx;
                f32 fy;
                f32 dx;*/
                // f32 gy;

                // i32 cr;
                // i32 cg;
                // i32 cb;
                // i32 ca;
                // i32 c;

                const f32* t = cache->xform;
                f32 fx = (static_cast<f32>(x) - tx) / sx;
                const f32 fy = (static_cast<f32>(y) - ty) / sy;
                const f32 dx = 1.0f / sx;

                for (i32 i = 0; i < count; i++) {
                    // i32 r;
                    // i32 g;
                    // i32 b;
                    // i32 a;
                    // i32 ia;

                    const f32 gy = fx * t[1] + fy * t[3] + t[5];
                    const i32
                        c = cache->colors[static_cast<i32>(nsvg_clampf(gy * 255.0f, 0, 255.0f))];

                    const i32 cr = (c) & 0xff;
                    const i32 cg = (c >> 8) & 0xff;
                    const i32 cb = (c >> 16) & 0xff;
                    const i32 ca = (c >> 24) & 0xff;

                    i32 a = nsvg_div255(static_cast<i32>(cover[0]) * ca);
                    const i32 ia = 255 - a;

                    // Premultiply
                    i32 r = nsvg_div255(cr * a);
                    i32 g = nsvg_div255(cg * a);
                    i32 b = nsvg_div255(cb * a);

                    // Blend over
                    r += nsvg_div255(ia * static_cast<i32>(dst[0]));
                    g += nsvg_div255(ia * static_cast<i32>(dst[1]));
                    b += nsvg_div255(ia * static_cast<i32>(dst[2]));
                    a += nsvg_div255(ia * static_cast<i32>(dst[3]));

                    dst[0] = static_cast<u8>(r);
                    dst[1] = static_cast<u8>(g);
                    dst[2] = static_cast<u8>(b);
                    dst[3] = static_cast<u8>(a);

                    cover++;
                    dst += 4;
                    fx += dx;
                }
            }
            else if (cache->type == nvg::svg::NSVGPaintRadialGradient) {
                const f32* t = cache->xform;
                f32 fx = (static_cast<f32>(x) - tx) / sx;
                const f32 fy = (static_cast<f32>(y) - ty) / sy;
                const f32 dx = 1.0f / sx;

                for (i32 i = 0; i < count; i++) {
                    const f32 gx = fx * t[0] + fy * t[2] + t[4];
                    const f32 gy = fx * t[1] + fy * t[3] + t[5];
                    const f32 gd = sqrtf(gx * gx + gy * gy);
                    const i32
                        c = cache->colors[static_cast<i32>(nsvg_clampf(gd * 255.0f, 0, 255.0f))];
                    const i32 cr = (c) & 0xff;
                    const i32 cg = (c >> 8) & 0xff;
                    const i32 cb = (c >> 16) & 0xff;
                    const i32 ca = (c >> 24) & 0xff;

                    i32 a = nsvg_div255(static_cast<i32>(cover[0]) * ca);
                    const i32 ia = 255 - a;

                    // Premultiply
                    i32 r = nsvg_div255(cr * a);
                    i32 g = nsvg_div255(cg * a);
                    i32 b = nsvg_div255(cb * a);

                    // Blend over
                    r += nsvg_div255(ia * static_cast<i32>(dst[0]));
                    g += nsvg_div255(ia * static_cast<i32>(dst[1]));
                    b += nsvg_div255(ia * static_cast<i32>(dst[2]));
                    a += nsvg_div255(ia * static_cast<i32>(dst[3]));

                    dst[0] = static_cast<u8>(r);
                    dst[1] = static_cast<u8>(g);
                    dst[2] = static_cast<u8>(b);
                    dst[3] = static_cast<u8>(a);

                    cover++;
                    dst += 4;
                    fx += dx;
                }
            }
        }

        void nsvg_rasterize_sorted_edges(NSVGrasterizer* r, const f32 tx, const f32 ty,
                                         const f32 sx, const f32 sy, NSVGcachedPaint* cache,
                                         const char fill_rule)
        {
            // weight per vertical scanline
            constexpr static i32 max_weight = (255 / NSVG_SUBSAMPLES);
            NSVGactiveEdge* active = nullptr;

            i32 e = 0;
            i32 xmin;
            i32 xmax;

            for (i32 y = 0; y < r->height; y++) {
                std::memset(r->scanline, 0, r->width);
                xmin = r->width;
                xmax = 0;

                for (i32 s = 0; s < NSVG_SUBSAMPLES; ++s) {
                    // find center of pixel for this scanline
                    const f32 scany = static_cast<f32>(y * NSVG_SUBSAMPLES + s) + 0.5f;
                    NSVGactiveEdge** step = &active;

                    // update all active edges;
                    // remove all active edges that terminate before the center of this scanline
                    while (*step) {
                        NSVGactiveEdge* z = *step;
                        if (z->ey <= scany) {
                            // delete from list
                            *step = z->next;
                            nsvg_free_active(r, z);
                        }
                        else {
                            // advance to position for current scanline
                            z->x += z->dx;
                            // advance through list
                            step = &((*step)->next);
                        }
                    }

                    // resort the list if needed
                    for (;;) {
                        i32 changed = 0;

                        step = &active;
                        while (*step && (*step)->next) {
                            if ((*step)->x > (*step)->next->x) {
                                NSVGactiveEdge* t = *step;
                                NSVGactiveEdge* q = t->next;
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

                    // insert all edges that start before the center of this scanline -- omit ones
                    // that also end on this scanline
                    while (e < r->nedges && r->edges[e].y0 <= scany) {
                        if (r->edges[e].y1 > scany) {
                            NSVGactiveEdge* z = nsvg_add_active(r, &r->edges[e], scany);
                            if (z == nullptr)
                                break;

                            // find insertion point
                            if (active == nullptr)
                                active = z;
                            else if (z->x < active->x) {
                                // insert at front
                                z->next = active;
                                active = z;
                            }
                            else {
                                // find thing to insert AFTER
                                NSVGactiveEdge* p = active;
                                while (p->next && p->next->x < z->x)
                                    p = p->next;

                                // at this point, p->next->x is NOT < z->x
                                z->next = p->next;
                                p->next = z;
                            }
                        }

                        e++;
                    }

                    // now process all active edges in non-zero fashion
                    if (active != nullptr)
                        nsvg_fill_active_edges(r->scanline, r->width, active, max_weight, &xmin,
                                               &xmax, fill_rule);
                }

                // Blit
                if (xmin < 0)
                    xmin = 0;
                if (xmax > r->width - 1)
                    xmax = r->width - 1;

                if (xmin <= xmax) {
                    nsvg_scanline_solid(&r->bitmap[y * r->stride] + xmin * 4, xmax - xmin + 1,
                                        &r->scanline[xmin], xmin, y, tx, ty, sx, sy, cache);
                }
            }
        }

        void nsvg_unpremultiply_alpha(u8* image, const i32 w, const i32 h, const i32 stride)
        {
            i32 x;
            i32 y;

            // Unpremultiply
            for (y = 0; y < h; y++) {
                u8* row = &image[y * stride];
                for (x = 0; x < w; x++) {
                    const i32 r = row[0];
                    const i32 g = row[1];
                    const i32 b = row[2];
                    const i32 a = row[3];

                    if (a != 0) {
                        row[0] = static_cast<u8>(r * 255 / a);
                        row[1] = static_cast<u8>(g * 255 / a);
                        row[2] = static_cast<u8>(b * 255 / a);
                    }
                    row += 4;
                }
            }

            // Defringe
            for (y = 0; y < h; y++) {
                u8* row = &image[y * stride];
                for (x = 0; x < w; x++) {
                    i32 r = 0, g = 0, b = 0, a = row[3], n = 0;
                    if (a == 0) {
                        if (x - 1 > 0 && row[-1] != 0) {
                            r += row[-4];
                            g += row[-3];
                            b += row[-2];
                            n++;
                        }
                        if (x + 1 < w && row[7] != 0) {
                            r += row[4];
                            g += row[5];
                            b += row[6];
                            n++;
                        }
                        if (y - 1 > 0 && row[-stride + 3] != 0) {
                            r += row[-stride];
                            g += row[-stride + 1];
                            b += row[-stride + 2];
                            n++;
                        }
                        if (y + 1 < h && row[stride + 3] != 0) {
                            r += row[stride];
                            g += row[stride + 1];
                            b += row[stride + 2];
                            n++;
                        }
                        if (n > 0) {
                            row[0] = static_cast<u8>(r / n);
                            row[1] = static_cast<u8>(g / n);
                            row[2] = static_cast<u8>(b / n);
                        }
                    }
                    row += 4;
                }
            }
        }

        void nsvg_init_paint(NSVGcachedPaint* cache, const nvg::svg::NSVGpaint* paint,
                             const f32 opacity)
        {
            i32 i;

            cache->type = paint->type;

            if (paint->type == nvg::svg::NSVGPaintColor) {
                cache->colors[0] = nsvg_apply_opacity(paint->color, opacity);
                return;
            }

            const nvg::svg::NSVGgradient* grad = paint->gradient;

            cache->spread = grad->spread;
            std::memcpy(cache->xform, grad->xform, sizeof(f32) * 6);

            if (grad->nstops == 0)
                for (i = 0; i < 256; i++)
                    cache->colors[i] = 0;
            if (grad->nstops == 1) {
                for (i = 0; i < 256; i++)
                    cache->colors[i] = nsvg_apply_opacity(grad->stops[i].color, opacity);
            }
            else {
                i32 cb = 0;

                i32 ca = nsvg_apply_opacity(grad->stops[0].color, opacity);
                f32 ua = nsvg_clampf(grad->stops[0].offset, 0, 1);
                f32 ub = nsvg_clampf(grad->stops[grad->nstops - 1].offset, ua, 1);
                i32 ia = static_cast<i32>(ua * 255.0f);
                i32 ib = static_cast<i32>(ub * 255.0f);

                for (i = 0; i < ia; i++)
                    cache->colors[i] = ca;

                for (i = 0; i < grad->nstops - 1; i++) {
                    ca = nsvg_apply_opacity(grad->stops[i].color, opacity);
                    cb = nsvg_apply_opacity(grad->stops[i + 1].color, opacity);
                    ua = nsvg_clampf(grad->stops[i].offset, 0, 1);
                    ub = nsvg_clampf(grad->stops[i + 1].offset, 0, 1);

                    ia = static_cast<i32>(ua * 255.0f);
                    ib = static_cast<i32>(ub * 255.0f);

                    const i32 count = ib - ia;
                    if (count <= 0)
                        continue;

                    f32 u = 0;
                    const f32 du = 1.0f / static_cast<f32>(count);
                    for (i32 j = 0; j < count; j++) {
                        cache->colors[ia + j] = nsvg_lerp_rgba(ca, cb, u);
                        u += du;
                    }
                }

                for (i = ib; i < 256; i++)
                    cache->colors[i] = cb;
            }
        }
    }

    void nsvg_rasterize_xy(NSVGrasterizer* r, const nvg::svg::NSVGimage* image, const f32 tx,
                           const f32 ty, const f32 sx, const f32 sy, u8* dst, const i32 w,
                           const i32 h, const i32 stride)
    {
        NSVGedge* e = nullptr;
        NSVGcachedPaint cache;
        i32 i;

        r->bitmap = dst;
        r->width = w;
        r->height = h;
        r->stride = stride;

        if (w > r->cscanline) {
            r->cscanline = w;
            r->scanline = static_cast<u8*>(std::realloc(r->scanline, w));
            if (r->scanline == nullptr)
                return;
        }

        for (i = 0; i < h; i++)
            std::memset(&dst[static_cast<ptrdiff_t>(i * stride)], 0, w * 4);

        for (const nvg::svg::NSVGshape* shape = image->shapes; shape != nullptr;
             shape = shape->next) {
            if ((shape->flags & nvg::svg::NSVGFlagsVisible) == 0)
                continue;

            if (shape->fill.type != nvg::svg::NSVGPaintNone) {
                nsvg_reset_pool(r);
                r->freelist = nullptr;
                r->nedges = 0;

                nsvg_flatten_shape(r, shape, sx, sy);

                // Scale and translate edges
                for (i = 0; i < r->nedges; i++) {
                    e = &r->edges[i];
                    e->x0 = tx + e->x0;
                    e->y0 = (ty + e->y0) * NSVG_SUBSAMPLES;
                    e->x1 = tx + e->x1;
                    e->y1 = (ty + e->y1) * NSVG_SUBSAMPLES;
                }

                // Rasterize edges
                if (r->nedges != 0)
                    std::qsort(r->edges, r->nedges, sizeof(NSVGedge), nsvg_cmp_edge);

                // now, traverse the scanlines and find the intersections on each scanline, use
                // non-zero rule
                nsvg_init_paint(&cache, &shape->fill, shape->opacity);
                nsvg_rasterize_sorted_edges(r, tx, ty, sx, sy, &cache, shape->fill_rule);
            }
            if (shape->stroke.type != nvg::svg::NSVGPaintNone &&
                (shape->stroke_width * sx) > 0.01f) {
                nsvg_reset_pool(r);
                r->freelist = nullptr;
                r->nedges = 0;

                nsvg_flatten_shape_stroke(r, shape, sx, sy);

                //          dumpEdges(r, "edge.svg");

                // Scale and translate edges
                for (i = 0; i < r->nedges; i++) {
                    e = &r->edges[i];
                    e->x0 = tx + e->x0;
                    e->y0 = (ty + e->y0) * NSVG_SUBSAMPLES;
                    e->x1 = tx + e->x1;
                    e->y1 = (ty + e->y1) * NSVG_SUBSAMPLES;
                }

                // Rasterize edges
                if (r->nedges != 0)
                    qsort(r->edges, r->nedges, sizeof(NSVGedge), nsvg_cmp_edge);

                // now, traverse the scanlines and find the intersections on each scanline, use
                // non-zero rule
                nsvg_init_paint(&cache, &shape->stroke, shape->opacity);

                nsvg_rasterize_sorted_edges(r, tx, ty, sx, sy, &cache,
                                            nvg::svg::NSVGFillruleNonzero);
            }
        }

        nsvg_unpremultiply_alpha(dst, w, h, stride);

        r->bitmap = nullptr;
        r->width = 0;
        r->height = 0;
        r->stride = 0;
    }

    void nsvg_rasterize(NSVGrasterizer* r, const nvg::svg::NSVGimage* image, const f32 tx,
                        const f32 ty, const f32 scale, u8* dst, const i32 w, const i32 h,
                        const i32 stride)
    {
        nsvg_rasterize_xy(r, image, tx, ty, scale, scale, dst, w, h, stride);
    }
}
