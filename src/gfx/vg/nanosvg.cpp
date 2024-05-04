#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <numbers>

#include "gfx/vg/nanosvg.hpp"
#include "utils/conversions.hpp"
#include "utils/numeric.hpp"

#define NSVG_NOTUSED(v)                    \
    do {                                   \
        (void)(1 ? (void)0 : ((void)(v))); \
    }                                      \
    while (0)

#define NSVG_RGB(r, g, b) \
    (((unsigned int)(r)) | ((unsigned int)(g) << 8) | ((unsigned int)(b) << 16))

#ifdef _MSC_VER
  #pragma warning(disable : 4996)  // Switch off security warnings
  #pragma warning(disable : 4100)  // Switch off unreferenced formal parameter warnings
  #pragma warning(disable : 4456)
  #pragma warning(disable : 4244)
// #pragma warning(disable : 2220)
#endif

namespace rl::nvg::svg {
    // Length proportional to radius of a cubic bezier handle for 90deg arcs.
    constexpr float NSVG_KAPPA90 = 0.5522847493f;
    constexpr int NSVG_MAX_DASHES = 8;
    constexpr float NSVG_EPSILON = 1e-6f;  // 1e-12f;

    enum NSVGXml {
        Tag = 1,
        Content = 2,
        MaxAttribs = 256,
        MaxAttr = 128,
    };

    enum NSVGAlign {
        Min = 0,
        Mid = 1,
        Max = 2,
        None = 0,
        Meet = 1,
        Slice = 2,
    };

    enum NSVGgradientUnits {
        NSVGUserSpace = 0,
        NSVGObjectSpace = 1
    };

    enum NSVGunits {
        NSVGUnitsUser,
        NSVGUnitsPx,
        NSVGUnitsPt,
        NSVGUnitsPc,
        NSVGUnitsMm,
        NSVGUnitsCm,
        NSVGUnitsIn,
        NSVGUnitsPercent,
        NSVGUnitsEm,
        NSVGUnitsEx
    };

    namespace {
        int nsvg_isspace(const char c)
        {
            return strchr(" \t\n\v\f\r", c) != nullptr;
        }

        int nsvg_isdigit(const char c)
        {
            return c >= '0' && c <= '9';
        }

        float nsvg_minf(const float a, const float b)
        {
            return a < b ? a : b;
        }

        float nsvg_maxf(const float a, const float b)
        {
            return a > b ? a : b;
        }

        // Simple XML parser

        void nsvg_parse_content(const char* str, void (*content_cb)(void* ud, const char* s),
                                void* ud)
        {
            // Trim start white spaces
            while (*str && nsvg_isspace(*str))
                str++;
            if (!*str)
                return;

            if (content_cb)
                (*content_cb)(ud, str);
        }

        void nsvg_parse_element(char* s,
                                void (*startel_cb)(void* ud, const char* el, const char** attr),
                                void (*endel_cb)(void* ud, const char* el), void* ud)
        {
            const char* attr[MaxAttribs];
            int nattr = 0;
            int start = 0;
            int end = 0;

            // Skip white space after the '<'
            while (*s && nsvg_isspace(*s))
                s++;

            // Check if the tag is end tag
            if (*s == '/') {
                s++;
                end = 1;
            }
            else
                start = 1;

            // Skip comments, data and preprocessor stuff.
            if (!*s || *s == '?' || *s == '!')
                return;

            // Get tag name
            const char* name = s;
            while (*s && !nsvg_isspace(*s))
                s++;
            if (*s)
                *s++ = '\0';

            // Get attribs
            while (!end && *s && nattr < MaxAttribs - 3) {
                const char* name_temp = nullptr;
                const char* value = nullptr;

                // Skip white space before the attrib name
                while (*s && nsvg_isspace(*s))
                    s++;
                if (!*s)
                    break;
                if (*s == '/') {
                    end = 1;
                    break;
                }
                name_temp = s;

                // Find end of the attrib name.
                while (*s && !nsvg_isspace(*s) && *s != '=')
                    s++;

                if (*s)
                    *s++ = '\0';

                // Skip until the beginning of the value.
                while (*s && *s != '\"' && *s != '\'')
                    s++;

                if (!*s)
                    break;

                const char quote = *s;
                s++;

                // Store value and find the end of it.
                value = s;
                while (*s && *s != quote)
                    s++;

                if (*s)
                    *s++ = '\0';

                // Store only well formed attributes
                if (name_temp && value) {
                    attr[nattr++] = name_temp;
                    attr[nattr++] = value;
                }
            }

            // List terminator
            attr[nattr++] = nullptr;
            attr[nattr++] = nullptr;

            // Call callbacks.
            if (start && startel_cb)
                (*startel_cb)(ud, name, attr);
            if (end && endel_cb)
                (*endel_cb)(ud, name);
        }

        int nsvg_parse_xml(char* input,
                           void (*startel_cb)(void* ud, const char* el, const char** attr),
                           void (*endel_cb)(void* ud, const char* el),
                           void (*content_cb)(void* ud, const char* s), void* ud)
        {
            char* s = input;
            char* mark = s;
            int state = Content;
            while (*s)
                if (*s == '<' && state == Content) {
                    // Start of a tag
                    *s++ = '\0';
                    nsvg_parse_content(mark, content_cb, ud);
                    mark = s;
                    state = Tag;
                }
                else if (*s == '>' && state == Tag) {
                    // Start of a content or new tag.
                    *s++ = '\0';
                    nsvg_parse_element(mark, startel_cb, endel_cb, ud);
                    mark = s;
                    state = Content;
                }
                else
                    s++;

            return 1;
        }
    }

    /* Simple SVG parser. */

    struct NSVGcoordinate
    {
        float value;
        int units;
    };

    struct NSVGlinearData
    {
        NSVGcoordinate x1, y1, x2, y2;
    };

    struct NSVGradialData
    {
        NSVGcoordinate cx, cy, r, fx, fy;
    };

    struct NSVGgradientData
    {
        char id[64];
        char ref[64];
        signed char type;

        union
        {
            NSVGlinearData linear;
            NSVGradialData radial;
        };

        char spread;
        char units;
        float xform[6];
        int nstops;
        NSVGgradientStop* stops;
        NSVGgradientData* next;
    };

    struct NSVGattrib
    {
        char id[64];
        float xform[6];
        unsigned int fill_color;
        unsigned int stroke_color;
        float opacity;
        float fill_opacity;
        float stroke_opacity;
        char fill_gradient[64];
        char stroke_gradient[64];
        float stroke_width;
        float stroke_dash_offset;
        float stroke_dash_array[NSVG_MAX_DASHES];
        int stroke_dash_count;
        char stroke_line_join;
        char stroke_line_cap;
        float miter_limit;
        char fill_rule;
        float font_size;
        unsigned int stop_color;
        float stop_opacity;
        float stop_offset;
        char has_fill;
        char has_stroke;
        char visible;
    };

    struct NSVGparser
    {
        NSVGattrib attr[MaxAttr];
        int attr_head;
        float* pts;
        int npts;
        int cpts;
        NSVGpath* plist;
        NSVGimage* image;
        NSVGgradientData* gradients;
        NSVGshape* shapes_tail;
        float view_minx;
        float view_miny;
        float view_width;
        float view_height;
        int align_x;
        int align_y;
        int align_type;
        float dpi;
        char path_flag;
        char defs_flag;
    };

    namespace {
        void nsvg_xform_identity(float* t)
        {
            t[0] = 1.0f;
            t[1] = 0.0f;
            t[2] = 0.0f;
            t[3] = 1.0f;
            t[4] = 0.0f;
            t[5] = 0.0f;
        }

        void nsvg_xform_set_translation(float* t, const float tx, const float ty)
        {
            t[0] = 1.0f;
            t[1] = 0.0f;
            t[2] = 0.0f;
            t[3] = 1.0f;
            t[4] = tx;
            t[5] = ty;
        }

        void nsvg_xform_set_scale(float* t, const float sx, const float sy)
        {
            t[0] = sx;
            t[1] = 0.0f;
            t[2] = 0.0f;
            t[3] = sy;
            t[4] = 0.0f;
            t[5] = 0.0f;
        }

        void nsvg_xform_set_skew_x(float* t, const float a)
        {
            t[0] = 1.0f;
            t[1] = 0.0f;
            t[2] = tanf(a);
            t[3] = 1.0f;
            t[4] = 0.0f;
            t[5] = 0.0f;
        }

        void nsvg_xform_set_skew_y(float* t, const float a)
        {
            t[0] = 1.0f;
            t[1] = tanf(a);
            t[2] = 0.0f;
            t[3] = 1.0f;
            t[4] = 0.0f;
            t[5] = 0.0f;
        }

        void nsvg_xform_set_rotation(float* t, const float a)
        {
            const float cs = cosf(a), sn = sinf(a);
            t[0] = cs;
            t[1] = sn;
            t[2] = -sn;
            t[3] = cs;
            t[4] = 0.0f;
            t[5] = 0.0f;
        }

        void nsvg_xform_multiply(float* t, const float* s)
        {
            const float t0 = t[0] * s[0] + t[1] * s[2];
            const float t2 = t[2] * s[0] + t[3] * s[2];
            const float t4 = t[4] * s[0] + t[5] * s[2] + s[4];
            t[1] = t[0] * s[1] + t[1] * s[3];
            t[3] = t[2] * s[1] + t[3] * s[3];
            t[5] = t[4] * s[1] + t[5] * s[3] + s[5];
            t[0] = t0;
            t[2] = t2;
            t[4] = t4;
        }

        void nsvg_xform_inverse(float* inv, float* t)
        {
            const float det = t[0] * t[3] - t[2] * t[1];
            if (det > -NSVG_EPSILON && det < NSVG_EPSILON) {
                nsvg_xform_identity(t);
                return;
            }
            const float invdet = 1.0f / det;
            inv[0] = t[3] * invdet;
            inv[2] = -t[2] * invdet;
            inv[4] = (t[2] * t[5] - t[3] * t[4]) * invdet;
            inv[1] = -t[1] * invdet;
            inv[3] = t[0] * invdet;
            inv[5] = (t[1] * t[4] - t[0] * t[5]) * invdet;
        }

        void nsvg_xform_premultiply(float* t, const float* s)
        {
            float s2[6];
            std::memcpy(s2, s, sizeof(float) * 6);
            nsvg_xform_multiply(s2, t);
            std::memcpy(t, s2, sizeof(float) * 6);
        }

        void nsvg_xform_point(float* dx, float* dy, const float x, const float y, const float* t)
        {
            *dx = x * t[0] + y * t[2] + t[4];
            *dy = x * t[1] + y * t[3] + t[5];
        }

        void nsvg_xform_vec(float* dx, float* dy, const float x, const float y, const float* t)
        {
            *dx = x * t[0] + y * t[2];
            *dy = x * t[1] + y * t[3];
        }

        int nsvg_pt_in_bounds(const float* pt, const float* bounds)
        {
            return pt[0] >= bounds[0] && pt[0] <= bounds[2] && pt[1] >= bounds[1] &&
                   pt[1] <= bounds[3];
        }

        double nsvg_eval_bezier(const double t, const double p0, const double p1, const double p2,
                                const double p3)
        {
            const double it = 1.0 - t;
            return it * it * it * p0 + 3.0 * it * it * t * p1 + 3.0 * it * t * t * p2 +
                   t * t * t * p3;
        }

        void nsvg_curve_bounds(float* bounds, const float* curve)
        {
            double roots[2] = {};
            double t{};

            const float* v0 = &curve[0];
            const float* v1 = &curve[2];
            const float* v2 = &curve[4];
            const float* v3 = &curve[6];

            // Start the bounding box by end points
            bounds[0] = nsvg_minf(v0[0], v3[0]);
            bounds[1] = nsvg_minf(v0[1], v3[1]);
            bounds[2] = nsvg_maxf(v0[0], v3[0]);
            bounds[3] = nsvg_maxf(v0[1], v3[1]);

            // Bezier curve fits inside the convex hull of it's control points.
            // If control points are inside the bounds, we're done.
            if (nsvg_pt_in_bounds(v1, bounds) && nsvg_pt_in_bounds(v2, bounds))
                return;

            // Add bezier curve inflection points in X and Y.
            for (int i = 0; i < 2; i++) {
                const double a = -3.0 * v0[i] + 9.0 * v1[i] - 9.0 * v2[i] + 3.0 * v3[i];
                const double b = 6.0 * v0[i] - 12.0 * v1[i] + 6.0 * v2[i];
                const double c = 3.0 * v1[i] - 3.0 * v0[i];
                int count = 0;
                if (fabs(a) < NSVG_EPSILON) {
                    if (fabs(b) > NSVG_EPSILON) {
                        t = -c / b;
                        if (t > NSVG_EPSILON && t < 1.0 - NSVG_EPSILON)
                            roots[count++] = t;
                    }
                }
                else {
                    const double b2_ac = b * b - 4.0 * c * a;
                    if (b2_ac > NSVG_EPSILON) {
                        t = (-b + sqrt(b2_ac)) / (2.0 * a);
                        if (t > NSVG_EPSILON && t < 1.0 - NSVG_EPSILON)
                            roots[count++] = t;
                        t = (-b - sqrt(b2_ac)) / (2.0 * a);
                        if (t > NSVG_EPSILON && t < 1.0 - NSVG_EPSILON)
                            roots[count++] = t;
                    }
                }
                for (int j = 0; j < count; j++) {
                    const double v = nsvg_eval_bezier(roots[j], v0[i], v1[i], v2[i], v3[i]);
                    bounds[0 + i] = nsvg_minf(bounds[0 + i], static_cast<float>(v));
                    bounds[2 + i] = nsvg_maxf(bounds[2 + i], static_cast<float>(v));
                }
            }
        }

        NSVGparser* nsvg_create_parser()
        {
            NSVGparser* p = static_cast<NSVGparser*>(malloc(sizeof(NSVGparser)));
            if (p != nullptr) {
                std::memset(p, 0, sizeof(NSVGparser));

                p->image = static_cast<NSVGimage*>(malloc(sizeof(NSVGimage)));
                if (p->image != nullptr) {
                    std::memset(p->image, 0, sizeof(NSVGimage));

                    // Init style
                    nsvg_xform_identity(p->attr[0].xform);
                    std::memset(p->attr[0].id, 0, sizeof p->attr[0].id);
                    p->attr[0].fill_color = NSVG_RGB(0, 0, 0);
                    p->attr[0].stroke_color = NSVG_RGB(0, 0, 0);
                    p->attr[0].opacity = 1;
                    p->attr[0].fill_opacity = 1;
                    p->attr[0].stroke_opacity = 1;
                    p->attr[0].stop_opacity = 1;
                    p->attr[0].stroke_width = 1;
                    p->attr[0].stroke_line_join = NSVGJoinMiter;
                    p->attr[0].stroke_line_cap = NSVGCapButt;
                    p->attr[0].miter_limit = 4;
                    p->attr[0].fill_rule = NSVGFillruleNonzero;
                    p->attr[0].has_fill = 1;
                    p->attr[0].visible = 1;

                    return p;
                }
            }

            // error:
            if (p != nullptr) {
                if (p->image)
                    std::free(p->image);
                std::free(p);
            }

            return nullptr;
        }

        void nsvg_delete_paths(NSVGpath* path)
        {
            while (path) {
                NSVGpath* next = path->next;
                if (path->pts != nullptr)
                    std::free(path->pts);
                std::free(path);
                path = next;
            }
        }

        void nsvg_delete_paint(const NSVGpaint* paint)
        {
            if (paint->type == NSVGPaintLinearGradient || paint->type == NSVGPaintRadialGradient)
                std::free(paint->gradient);
        }

        void nsvg_delete_gradient_data(NSVGgradientData* grad)
        {
            while (grad != nullptr) {
                NSVGgradientData* next = grad->next;
                std::free(grad->stops);
                std::free(grad);
                grad = next;
            }
        }

        void nsvg_delete_parser(NSVGparser* p)
        {
            if (p != nullptr) {
                nsvg_delete_paths(p->plist);
                nsvg_delete_gradient_data(p->gradients);
                nsvg_delete(p->image);
                std::free(p->pts);
                std::free(p);
            }
        }

        void nsvg_reset_path(NSVGparser* p)
        {
            p->npts = 0;
        }

        void nsvg_add_point(NSVGparser* p, const float x, const float y)
        {
            if (p->npts + 1 > p->cpts) {
                p->cpts = p->cpts ? p->cpts * 2 : 8;
                p->pts = static_cast<float*>(realloc(p->pts, p->cpts * 2 * sizeof(float)));
                if (!p->pts)
                    return;
            }
            p->pts[p->npts * 2 + 0] = x;
            p->pts[p->npts * 2 + 1] = y;
            p->npts++;
        }

        void nsvg_move_to(NSVGparser* p, const float x, const float y)
        {
            if (p->npts > 0) {
                p->pts[(p->npts - 1) * 2 + 0] = x;
                p->pts[(p->npts - 1) * 2 + 1] = y;
            }
            else
                nsvg_add_point(p, x, y);
        }

        void nsvg_line_to(NSVGparser* p, const float x, const float y)
        {
            if (p->npts > 0) {
                const float px = p->pts[(p->npts - 1) * 2 + 0];
                const float py = p->pts[(p->npts - 1) * 2 + 1];
                const float dx = x - px;
                const float dy = y - py;
                nsvg_add_point(p, px + dx / 3.0f, py + dy / 3.0f);
                nsvg_add_point(p, x - dx / 3.0f, y - dy / 3.0f);
                nsvg_add_point(p, x, y);
            }
        }

        void nsvg_cubic_bez_to(NSVGparser* p, const float cpx1, const float cpy1, const float cpx2,
                               const float cpy2, const float x, const float y)
        {
            if (p->npts > 0) {
                nsvg_add_point(p, cpx1, cpy1);
                nsvg_add_point(p, cpx2, cpy2);
                nsvg_add_point(p, x, y);
            }
        }

        NSVGattrib* nsvg_get_attr(NSVGparser* p)
        {
            return &p->attr[p->attr_head];
        }

        void nsvg_push_attr(NSVGparser* p)
        {
            if (p->attr_head < MaxAttr - 1) {
                p->attr_head++;
                std::memcpy(&p->attr[p->attr_head], &p->attr[p->attr_head - 1], sizeof(NSVGattrib));
            }
        }

        void nsvg_pop_attr(NSVGparser* p)
        {
            if (p->attr_head > 0)
                p->attr_head--;
        }

        float nsvg_actual_orig_x(const NSVGparser* p)
        {
            return p->view_minx;
        }

        float nsvg_actual_orig_y(const NSVGparser* p)
        {
            return p->view_miny;
        }

        float nsvg_actual_width(const NSVGparser* p)
        {
            return p->view_width;
        }

        float nsvg_actual_height(const NSVGparser* p)
        {
            return p->view_height;
        }

        float nsvg_actual_length(const NSVGparser* p)
        {
            const float w = nsvg_actual_width(p);
            const float h = nsvg_actual_height(p);
            return sqrtf(w * w + h * h) / sqrtf(2.0f);
        }

        float nsvg_convert_to_pixels(NSVGparser* p, const NSVGcoordinate c, const float orig,
                                     const float length)
        {
            const NSVGattrib* attr = nsvg_get_attr(p);
            switch (c.units) {
                case NSVGUnitsUser:
                    [[fallthrough]];
                case NSVGUnitsPx:
                    return c.value;
                case NSVGUnitsPt:
                    return c.value / 72.0f * p->dpi;
                case NSVGUnitsPc:
                    return c.value / 6.0f * p->dpi;
                case NSVGUnitsMm:
                    return c.value / 25.4f * p->dpi;
                case NSVGUnitsCm:
                    return c.value / 2.54f * p->dpi;
                case NSVGUnitsIn:
                    return c.value * p->dpi;
                case NSVGUnitsEm:
                    return c.value * attr->font_size;
                case NSVGUnitsEx:
                    return c.value * attr->font_size * 0.52f;  // x-height of Helvetica.
                case NSVGUnitsPercent:
                    return orig + c.value / 100.0f * length;
                default:
                    return c.value;
            }
        }

        NSVGgradientData* nsvg_find_gradient_data(const NSVGparser* p, const char* id)
        {
            NSVGgradientData* grad = p->gradients;
            if (id == nullptr || *id == '\0')
                return nullptr;
            while (grad != nullptr) {
                if (std::strcmp(grad->id, id) == 0)
                    return grad;
                grad = grad->next;
            }
            return nullptr;
        }

        NSVGgradient* nsvg_create_gradient(NSVGparser* p, const char* id, const float* local_bounds,
                                           const float* xform, signed char* paint_type)
        {
            const NSVGgradientData* data = nullptr;
            const NSVGgradientData* ref = nullptr;
            const NSVGgradientStop* stops = nullptr;
            float ox, oy, sw, sh;
            int nstops = 0;

            data = nsvg_find_gradient_data(p, id);
            if (data == nullptr)
                return nullptr;

            // TODO: use ref to fill in all unset values too.
            ref = data;
            int ref_iter = 0;
            while (ref != nullptr) {
                const NSVGgradientData* next_ref = nullptr;
                if (stops == nullptr && ref->stops != nullptr) {
                    stops = ref->stops;
                    nstops = ref->nstops;
                    break;
                }
                next_ref = nsvg_find_gradient_data(p, ref->ref);
                if (next_ref == ref)
                    break;  // prevent infite loops on malformed data
                ref = next_ref;
                ref_iter++;
                if (ref_iter > 32)
                    break;  // prevent infite loops on malformed data
            }
            if (stops == nullptr)
                return nullptr;

            NSVGgradient* grad = static_cast<NSVGgradient*>(
                std::malloc(sizeof(NSVGgradient) + sizeof(NSVGgradientStop) * (nstops - 1)));
            if (grad == nullptr)
                return nullptr;

            // The shape width and height.
            if (data->units == NSVGObjectSpace) {
                ox = local_bounds[0];
                oy = local_bounds[1];
                sw = local_bounds[2] - local_bounds[0];
                sh = local_bounds[3] - local_bounds[1];
            }
            else {
                ox = nsvg_actual_orig_x(p);
                oy = nsvg_actual_orig_y(p);
                sw = nsvg_actual_width(p);
                sh = nsvg_actual_height(p);
            }
            const float sl = sqrtf(sw * sw + sh * sh) / sqrtf(2.0f);

            if (data->type == NSVGPaintLinearGradient) {
                const float x1 = nsvg_convert_to_pixels(p, data->linear.x1, ox, sw);
                const float y1 = nsvg_convert_to_pixels(p, data->linear.y1, oy, sh);
                const float x2 = nsvg_convert_to_pixels(p, data->linear.x2, ox, sw);
                const float y2 = nsvg_convert_to_pixels(p, data->linear.y2, oy, sh);
                // Calculate transform aligned to the line
                const float dx = x2 - x1;
                const float dy = y2 - y1;
                grad->xform[0] = dy;
                grad->xform[1] = -dx;
                grad->xform[2] = dx;
                grad->xform[3] = dy;
                grad->xform[4] = x1;
                grad->xform[5] = y1;
            }
            else {
                const float cx = nsvg_convert_to_pixels(p, data->radial.cx, ox, sw);
                const float cy = nsvg_convert_to_pixels(p, data->radial.cy, oy, sh);
                const float fx = nsvg_convert_to_pixels(p, data->radial.fx, ox, sw);
                const float fy = nsvg_convert_to_pixels(p, data->radial.fy, oy, sh);
                const float r = nsvg_convert_to_pixels(p, data->radial.r, 0, sl);
                // Calculate transform aligned to the circle
                grad->xform[0] = r;
                grad->xform[1] = 0;
                grad->xform[2] = 0;
                grad->xform[3] = r;
                grad->xform[4] = cx;
                grad->xform[5] = cy;
                grad->fx = (fx - cx) / r;
                grad->fy = (fy - cy) / r;
            }

            nsvg_xform_multiply(grad->xform, data->xform);
            nsvg_xform_multiply(grad->xform, xform);

            grad->spread = data->spread;
            std::memcpy(grad->stops, stops, nstops * sizeof(NSVGgradientStop));
            grad->nstops = nstops;

            *paint_type = data->type;

            return grad;
        }

        float nsvg_get_average_scale(const float* t)
        {
            const float sx = sqrtf(t[0] * t[0] + t[2] * t[2]);
            const float sy = sqrtf(t[1] * t[1] + t[3] * t[3]);
            return (sx + sy) * 0.5f;
        }

        void nsvg_get_local_bounds(float* bounds, const NSVGshape* shape, const float* xform)
        {
            float curve[4 * 2], curve_bounds[4];
            int first = 1;
            for (const NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
                nsvg_xform_point(&curve[0], &curve[1], path->pts[0], path->pts[1], xform);
                for (int i = 0; i < path->npts - 1; i += 3) {
                    nsvg_xform_point(&curve[2], &curve[3], path->pts[(i + 1) * 2],
                                     path->pts[(i + 1) * 2 + 1], xform);
                    nsvg_xform_point(&curve[4], &curve[5], path->pts[(i + 2) * 2],
                                     path->pts[(i + 2) * 2 + 1], xform);
                    nsvg_xform_point(&curve[6], &curve[7], path->pts[(i + 3) * 2],
                                     path->pts[(i + 3) * 2 + 1], xform);
                    nsvg_curve_bounds(curve_bounds, curve);
                    if (first) {
                        bounds[0] = curve_bounds[0];
                        bounds[1] = curve_bounds[1];
                        bounds[2] = curve_bounds[2];
                        bounds[3] = curve_bounds[3];
                        first = 0;
                    }
                    else {
                        bounds[0] = nsvg_minf(bounds[0], curve_bounds[0]);
                        bounds[1] = nsvg_minf(bounds[1], curve_bounds[1]);
                        bounds[2] = nsvg_maxf(bounds[2], curve_bounds[2]);
                        bounds[3] = nsvg_maxf(bounds[3], curve_bounds[3]);
                    }
                    curve[0] = curve[6];
                    curve[1] = curve[7];
                }
            }
        }

        void nsvg_add_shape(NSVGparser* p)
        {
            const NSVGattrib* attr = nsvg_get_attr(p);
            float scale = 1.0f;

            if (p->plist == nullptr)
                return;

            NSVGshape* shape = static_cast<NSVGshape*>(malloc(sizeof(NSVGshape)));
            if (shape == nullptr)
                goto error;
            memset(shape, 0, sizeof(NSVGshape));

            std::memcpy(shape->id, attr->id, sizeof shape->id);
            std::memcpy(shape->fill_gradient, attr->fill_gradient, sizeof shape->fill_gradient);
            std::memcpy(shape->stroke_gradient, attr->stroke_gradient, sizeof shape->stroke_gradient);
            std::memcpy(shape->xform, attr->xform, sizeof shape->xform);
            scale = nsvg_get_average_scale(attr->xform);
            shape->stroke_width = attr->stroke_width * scale;
            shape->stroke_dash_offset = attr->stroke_dash_offset * scale;
            shape->stroke_dash_count = static_cast<char>(attr->stroke_dash_count);
            for (int i = 0; i < attr->stroke_dash_count; i++)
                shape->stroke_dash_array[i] = attr->stroke_dash_array[i] * scale;
            shape->stroke_line_join = attr->stroke_line_join;
            shape->stroke_line_cap = attr->stroke_line_cap;
            shape->miter_limit = attr->miter_limit;
            shape->fill_rule = attr->fill_rule;
            shape->opacity = attr->opacity;

            shape->paths = p->plist;
            p->plist = nullptr;

            // Calculate shape bounds
            shape->bounds[0] = shape->paths->bounds[0];
            shape->bounds[1] = shape->paths->bounds[1];
            shape->bounds[2] = shape->paths->bounds[2];
            shape->bounds[3] = shape->paths->bounds[3];
            for (const NSVGpath* path = shape->paths->next; path != nullptr; path = path->next) {
                shape->bounds[0] = nsvg_minf(shape->bounds[0], path->bounds[0]);
                shape->bounds[1] = nsvg_minf(shape->bounds[1], path->bounds[1]);
                shape->bounds[2] = nsvg_maxf(shape->bounds[2], path->bounds[2]);
                shape->bounds[3] = nsvg_maxf(shape->bounds[3], path->bounds[3]);
            }

            // Set fill
            if (attr->has_fill == 0)
                shape->fill.type = NSVGPaintNone;
            else if (attr->has_fill == 1) {
                shape->fill.type = NSVGPaintColor;
                shape->fill.color = attr->fill_color;
                shape->fill.color |= static_cast<unsigned int>(attr->fill_opacity * 255) << 24;
            }
            else if (attr->has_fill == 2)
                shape->fill.type = NSVGPaintUndef;

            // Set stroke
            if (attr->has_stroke == 0)
                shape->stroke.type = NSVGPaintNone;
            else if (attr->has_stroke == 1) {
                shape->stroke.type = NSVGPaintColor;
                shape->stroke.color = attr->stroke_color;
                shape->stroke.color |= static_cast<unsigned int>(attr->stroke_opacity * 255) << 24;
            }
            else if (attr->has_stroke == 2)
                shape->stroke.type = NSVGPaintUndef;

            // Set flags
            shape->flags = attr->visible ? NSVGFlagsVisible : 0x00;

            // Add to tail
            if (p->image->shapes == nullptr)
                p->image->shapes = shape;
            else
                p->shapes_tail->next = shape;
            p->shapes_tail = shape;

            return;

        error:
            if (shape)
                std::free(shape);
        }

        void nsvg_add_path(NSVGparser* p, const char closed)
        {
            const NSVGattrib* attr = nsvg_get_attr(p);
            NSVGpath* path = nullptr;
            float bounds[4];
            int i;

            if (p->npts < 4)
                return;

            if (closed)
                nsvg_line_to(p, p->pts[0], p->pts[1]);

            // Expect 1 + N*3 points (N = number of cubic bezier segments).
            if (p->npts % 3 != 1)
                return;

            path = static_cast<NSVGpath*>(malloc(sizeof(NSVGpath)));
            if (path == nullptr)
                goto error;
            memset(path, 0, sizeof(NSVGpath));

            path->pts = static_cast<float*>(malloc(p->npts * 2 * sizeof(float)));
            if (path->pts == nullptr)
                goto error;
            path->closed = closed;
            path->npts = p->npts;

            // Transform path.
            for (i = 0; i < p->npts; ++i)
                nsvg_xform_point(&path->pts[i * 2], &path->pts[i * 2 + 1], p->pts[i * 2],
                                 p->pts[i * 2 + 1], attr->xform);

            // Find bounds
            for (i = 0; i < path->npts - 1; i += 3) {
                const float* curve = &path->pts[i * 2];
                nsvg_curve_bounds(bounds, curve);
                if (i == 0) {
                    path->bounds[0] = bounds[0];
                    path->bounds[1] = bounds[1];
                    path->bounds[2] = bounds[2];
                    path->bounds[3] = bounds[3];
                }
                else {
                    path->bounds[0] = nsvg_minf(path->bounds[0], bounds[0]);
                    path->bounds[1] = nsvg_minf(path->bounds[1], bounds[1]);
                    path->bounds[2] = nsvg_maxf(path->bounds[2], bounds[2]);
                    path->bounds[3] = nsvg_maxf(path->bounds[3], bounds[3]);
                }
            }

            path->next = p->plist;
            p->plist = path;

            return;

        error:
            if (path != nullptr) {
                if (path->pts != nullptr)
                    std::free(path->pts);
                std::free(path);
            }
        }

        // We roll our own string to float because the std library one uses locale and messes things
        // up.
        double nsvg_atof(const char* s)
        {
            const char* cur = const_cast<char*>(s);
            char* end = nullptr;
            double res = 0.0, sign = 1.0;
            long long int_part = 0;
            long long frac_part = 0;
            char has_int_part = 0, has_frac_part = 0;

            // Parse optional sign
            if (*cur == '+')
                cur++;
            else if (*cur == '-') {
                sign = -1;
                cur++;
            }

            // Parse integer part
            if (nsvg_isdigit(*cur)) {
                // Parse digit sequence
                int_part = strtoll(cur, &end, 10);
                if (cur != end) {
                    res = static_cast<double>(int_part);
                    has_int_part = 1;
                    cur = end;
                }
            }

            // Parse fractional part.
            if (*cur == '.') {
                cur++;  // Skip '.'
                if (nsvg_isdigit(*cur)) {
                    // Parse digit sequence
                    frac_part = strtoll(cur, &end, 10);
                    if (cur != end) {
                        res += static_cast<double>(frac_part) /
                               pow(10.0, static_cast<double>(end - cur));
                        has_frac_part = 1;
                        cur = end;
                    }
                }
            }

            // A valid number should have integer or fractional part.
            if (!has_int_part && !has_frac_part)
                return 0.0;

            // Parse optional exponent
            if (*cur == 'e' || *cur == 'E') {
                long exp_part = 0;
                cur++;                             // skip 'E'
                exp_part = strtol(cur, &end, 10);  // Parse digit sequence with sign
                if (cur != end)
                    res *= pow(10.0, static_cast<double>(exp_part));
            }

            return res * sign;
        }

        const char* nsvg_parse_number(const char* s, char* it, const int size)
        {
            const int last = size - 1;
            int i = 0;

            // sign
            if (*s == '-' || *s == '+') {
                if (i < last)
                    it[i++] = *s;
                s++;
            }
            // integer part
            while (*s && nsvg_isdigit(*s)) {
                if (i < last)
                    it[i++] = *s;
                s++;
            }
            if (*s == '.') {
                // decimal point
                if (i < last)
                    it[i++] = *s;
                s++;
                // fraction part
                while (*s && nsvg_isdigit(*s)) {
                    if (i < last)
                        it[i++] = *s;
                    s++;
                }
            }
            // exponent
            if ((*s == 'e' || *s == 'E') && (s[1] != 'm' && s[1] != 'x')) {
                if (i < last)
                    it[i++] = *s;
                s++;
                if (*s == '-' || *s == '+') {
                    if (i < last)
                        it[i++] = *s;
                    s++;
                }
                while (*s && nsvg_isdigit(*s)) {
                    if (i < last)
                        it[i++] = *s;
                    s++;
                }
            }
            it[i] = '\0';

            return s;
        }

        const char* nsvg_get_next_path_item_when_arc_flag(const char* s, char* it)
        {
            it[0] = '\0';
            // Skip white spaces and commas
            while (*s && (nsvg_isspace(*s) || *s == ','))
                s++;
            if (!*s)
                return s;
            if (*s == '-' || *s == '+' || *s == '.' || nsvg_isdigit(*s))
                s = nsvg_parse_number(s, it, 64);
            else {
                // Parse command
                it[0] = *s++;
                it[1] = '\0';
                return s;
            }

            return s;
        }

        const char* nsvg_get_next_path_item(const char* s, char* it)
        {
            it[0] = '\0';
            // Skip white spaces and commas
            while (*s && (nsvg_isspace(*s) || *s == ','))
                s++;
            if (!*s)
                return s;
            if (*s == '-' || *s == '+' || *s == '.' || nsvg_isdigit(*s))
                s = nsvg_parse_number(s, it, 64);
            else {
                // Parse command
                it[0] = *s++;
                it[1] = '\0';
                return s;
            }

            return s;
        }

        unsigned int nsvg_parse_color_hex(const char* str)
        {
            unsigned int r = 0, g = 0, b = 0;
            if (sscanf(str, "#%2x%2x%2x", &r, &g, &b) == 3)  // 2 digit hex
                return NSVG_RGB(r, g, b);
            if (sscanf(str, "#%1x%1x%1x", &r, &g, &b) == 3)  // 1 digit hex, e.g. #abc -> 0xccbbaa
                return NSVG_RGB(r * 17, g * 17, b * 17);     // same effect as (r<<4|r), (g<<4|g), ..
            return NSVG_RGB(128, 128, 128);
        }

        // Parse rgb color. The pointer 'str' must point at "rgb(" (4+ characters).
        // This function returns gray (rgb(128, 128, 128) == '#808080') on parse errors
        // for backwards compatibility. Note: other image viewers return black instead.

        unsigned int nsvg_parse_color_rgb(const char* str)
        {
            int i;
            unsigned int rgbi[3];
            float rgbf[3];
            // try decimal integers first
            if (sscanf(str, "rgb(%u, %u, %u)", &rgbi[0], &rgbi[1], &rgbi[2]) != 3) {
                // integers failed, try percent values (float, locale independent)
                constexpr char delimiter[3] = { ',', ',', ')' };
                str += 4;  // skip "rgb("
                for (i = 0; i < 3; i++) {
                    while (*str && nsvg_isspace(*str))
                        str++;  // skip leading spaces
                    if (*str == '+')
                        str++;  // skip '+' (don't allow '-')
                    if (!*str)
                        break;
                    rgbf[i] = nsvg_atof(str);

                    // Note 1: it would be great if nsvg__atof() returned how many
                    // bytes it consumed but it doesn't. We need to skip the number,
                    // the '%' character, spaces, and the delimiter ',' or ')'.

                    // Note 2: The following code does not allow values like "33.%",
                    // i.e. a decimal point w/o fractional part, but this is consistent
                    // with other image viewers, e.g. firefox, chrome, eog, gimp.

                    while (*str && nsvg_isdigit(*str))
                        str++;  // skip integer part
                    if (*str == '.') {
                        str++;
                        if (!nsvg_isdigit(*str))
                            break;  // error: no digit after '.'
                        while (*str && nsvg_isdigit(*str))
                            str++;  // skip fractional part
                    }
                    if (*str == '%')
                        str++;
                    else
                        break;
                    while (nsvg_isspace(*str))
                        str++;
                    if (*str == delimiter[i])
                        str++;
                    else
                        break;
                }
                if (i == 3) {
                    rgbi[0] = static_cast<u32>(std::roundf(rgbf[0] * 2.55f));
                    rgbi[1] = static_cast<u32>(std::roundf(rgbf[1] * 2.55f));
                    rgbi[2] = static_cast<u32>(std::roundf(rgbf[2] * 2.55f));
                }
                else
                    rgbi[0] = rgbi[1] = rgbi[2] = 128;
            }
            // clip values as the CSS spec requires
            for (i = 0; i < 3; i++)
                if (rgbi[i] > 255)
                    rgbi[i] = 255;

            return NSVG_RGB(rgbi[0], rgbi[1], rgbi[2]);
        }

        using NSVGNamedColor = struct NSVGNamedColor
        {
            const char* name;
            unsigned int color;
        };

        NSVGNamedColor nsvg_colors[] = {
            { "red", NSVG_RGB(255, 0, 0) },
            { "green", NSVG_RGB(0, 128, 0) },
            { "blue", NSVG_RGB(0, 0, 255) },
            { "yellow", NSVG_RGB(255, 255, 0) },
            { "cyan", NSVG_RGB(0, 255, 255) },
            { "magenta", NSVG_RGB(255, 0, 255) },
            { "black", NSVG_RGB(0, 0, 0) },
            { "grey", NSVG_RGB(128, 128, 128) },
            { "gray", NSVG_RGB(128, 128, 128) },
            { "white", NSVG_RGB(255, 255, 255) },
            { "aliceblue", NSVG_RGB(240, 248, 255) },
            { "antiquewhite", NSVG_RGB(250, 235, 215) },
            { "aqua", NSVG_RGB(0, 255, 255) },
            { "aquamarine", NSVG_RGB(127, 255, 212) },
            { "azure", NSVG_RGB(240, 255, 255) },
            { "beige", NSVG_RGB(245, 245, 220) },
            { "bisque", NSVG_RGB(255, 228, 196) },
            { "blanchedalmond", NSVG_RGB(255, 235, 205) },
            { "blueviolet", NSVG_RGB(138, 43, 226) },
            { "brown", NSVG_RGB(165, 42, 42) },
            { "burlywood", NSVG_RGB(222, 184, 135) },
            { "cadetblue", NSVG_RGB(95, 158, 160) },
            { "chartreuse", NSVG_RGB(127, 255, 0) },
            { "chocolate", NSVG_RGB(210, 105, 30) },
            { "coral", NSVG_RGB(255, 127, 80) },
            { "cornflowerblue", NSVG_RGB(100, 149, 237) },
            { "cornsilk", NSVG_RGB(255, 248, 220) },
            { "crimson", NSVG_RGB(220, 20, 60) },
            { "darkblue", NSVG_RGB(0, 0, 139) },
            { "darkcyan", NSVG_RGB(0, 139, 139) },
            { "darkgoldenrod", NSVG_RGB(184, 134, 11) },
            { "darkgray", NSVG_RGB(169, 169, 169) },
            { "darkgreen", NSVG_RGB(0, 100, 0) },
            { "darkgrey", NSVG_RGB(169, 169, 169) },
            { "darkkhaki", NSVG_RGB(189, 183, 107) },
            { "darkmagenta", NSVG_RGB(139, 0, 139) },
            { "darkolivegreen", NSVG_RGB(85, 107, 47) },
            { "darkorange", NSVG_RGB(255, 140, 0) },
            { "darkorchid", NSVG_RGB(153, 50, 204) },
            { "darkred", NSVG_RGB(139, 0, 0) },
            { "darksalmon", NSVG_RGB(233, 150, 122) },
            { "darkseagreen", NSVG_RGB(143, 188, 143) },
            { "darkslateblue", NSVG_RGB(72, 61, 139) },
            { "darkslategray", NSVG_RGB(47, 79, 79) },
            { "darkslategrey", NSVG_RGB(47, 79, 79) },
            { "darkturquoise", NSVG_RGB(0, 206, 209) },
            { "darkviolet", NSVG_RGB(148, 0, 211) },
            { "deeppink", NSVG_RGB(255, 20, 147) },
            { "deepskyblue", NSVG_RGB(0, 191, 255) },
            { "dimgray", NSVG_RGB(105, 105, 105) },
            { "dimgrey", NSVG_RGB(105, 105, 105) },
            { "dodgerblue", NSVG_RGB(30, 144, 255) },
            { "firebrick", NSVG_RGB(178, 34, 34) },
            { "floralwhite", NSVG_RGB(255, 250, 240) },
            { "forestgreen", NSVG_RGB(34, 139, 34) },
            { "fuchsia", NSVG_RGB(255, 0, 255) },
            { "gainsboro", NSVG_RGB(220, 220, 220) },
            { "ghostwhite", NSVG_RGB(248, 248, 255) },
            { "gold", NSVG_RGB(255, 215, 0) },
            { "goldenrod", NSVG_RGB(218, 165, 32) },
            { "greenyellow", NSVG_RGB(173, 255, 47) },
            { "honeydew", NSVG_RGB(240, 255, 240) },
            { "hotpink", NSVG_RGB(255, 105, 180) },
            { "indianred", NSVG_RGB(205, 92, 92) },
            { "indigo", NSVG_RGB(75, 0, 130) },
            { "ivory", NSVG_RGB(255, 255, 240) },
            { "khaki", NSVG_RGB(240, 230, 140) },
            { "lavender", NSVG_RGB(230, 230, 250) },
            { "lavenderblush", NSVG_RGB(255, 240, 245) },
            { "lawngreen", NSVG_RGB(124, 252, 0) },
            { "lemonchiffon", NSVG_RGB(255, 250, 205) },
            { "lightblue", NSVG_RGB(173, 216, 230) },
            { "lightcoral", NSVG_RGB(240, 128, 128) },
            { "lightcyan", NSVG_RGB(224, 255, 255) },
            { "lightgoldenrodyellow", NSVG_RGB(250, 250, 210) },
            { "lightgray", NSVG_RGB(211, 211, 211) },
            { "lightgreen", NSVG_RGB(144, 238, 144) },
            { "lightgrey", NSVG_RGB(211, 211, 211) },
            { "lightpink", NSVG_RGB(255, 182, 193) },
            { "lightsalmon", NSVG_RGB(255, 160, 122) },
            { "lightseagreen", NSVG_RGB(32, 178, 170) },
            { "lightskyblue", NSVG_RGB(135, 206, 250) },
            { "lightslategray", NSVG_RGB(119, 136, 153) },
            { "lightslategrey", NSVG_RGB(119, 136, 153) },
            { "lightsteelblue", NSVG_RGB(176, 196, 222) },
            { "lightyellow", NSVG_RGB(255, 255, 224) },
            { "lime", NSVG_RGB(0, 255, 0) },
            { "limegreen", NSVG_RGB(50, 205, 50) },
            { "linen", NSVG_RGB(250, 240, 230) },
            { "maroon", NSVG_RGB(128, 0, 0) },
            { "mediumaquamarine", NSVG_RGB(102, 205, 170) },
            { "mediumblue", NSVG_RGB(0, 0, 205) },
            { "mediumorchid", NSVG_RGB(186, 85, 211) },
            { "mediumpurple", NSVG_RGB(147, 112, 219) },
            { "mediumseagreen", NSVG_RGB(60, 179, 113) },
            { "mediumslateblue", NSVG_RGB(123, 104, 238) },
            { "mediumspringgreen", NSVG_RGB(0, 250, 154) },
            { "mediumturquoise", NSVG_RGB(72, 209, 204) },
            { "mediumvioletred", NSVG_RGB(199, 21, 133) },
            { "midnightblue", NSVG_RGB(25, 25, 112) },
            { "mintcream", NSVG_RGB(245, 255, 250) },
            { "mistyrose", NSVG_RGB(255, 228, 225) },
            { "moccasin", NSVG_RGB(255, 228, 181) },
            { "navajowhite", NSVG_RGB(255, 222, 173) },
            { "navy", NSVG_RGB(0, 0, 128) },
            { "oldlace", NSVG_RGB(253, 245, 230) },
            { "olive", NSVG_RGB(128, 128, 0) },
            { "olivedrab", NSVG_RGB(107, 142, 35) },
            { "orange", NSVG_RGB(255, 165, 0) },
            { "orangered", NSVG_RGB(255, 69, 0) },
            { "orchid", NSVG_RGB(218, 112, 214) },
            { "palegoldenrod", NSVG_RGB(238, 232, 170) },
            { "palegreen", NSVG_RGB(152, 251, 152) },
            { "paleturquoise", NSVG_RGB(175, 238, 238) },
            { "palevioletred", NSVG_RGB(219, 112, 147) },
            { "papayawhip", NSVG_RGB(255, 239, 213) },
            { "peachpuff", NSVG_RGB(255, 218, 185) },
            { "peru", NSVG_RGB(205, 133, 63) },
            { "pink", NSVG_RGB(255, 192, 203) },
            { "plum", NSVG_RGB(221, 160, 221) },
            { "powderblue", NSVG_RGB(176, 224, 230) },
            { "purple", NSVG_RGB(128, 0, 128) },
            { "rosybrown", NSVG_RGB(188, 143, 143) },
            { "royalblue", NSVG_RGB(65, 105, 225) },
            { "saddlebrown", NSVG_RGB(139, 69, 19) },
            { "salmon", NSVG_RGB(250, 128, 114) },
            { "sandybrown", NSVG_RGB(244, 164, 96) },
            { "seagreen", NSVG_RGB(46, 139, 87) },
            { "seashell", NSVG_RGB(255, 245, 238) },
            { "sienna", NSVG_RGB(160, 82, 45) },
            { "silver", NSVG_RGB(192, 192, 192) },
            { "skyblue", NSVG_RGB(135, 206, 235) },
            { "slateblue", NSVG_RGB(106, 90, 205) },
            { "slategray", NSVG_RGB(112, 128, 144) },
            { "slategrey", NSVG_RGB(112, 128, 144) },
            { "snow", NSVG_RGB(255, 250, 250) },
            { "springgreen", NSVG_RGB(0, 255, 127) },
            { "steelblue", NSVG_RGB(70, 130, 180) },
            { "tan", NSVG_RGB(210, 180, 140) },
            { "teal", NSVG_RGB(0, 128, 128) },
            { "thistle", NSVG_RGB(216, 191, 216) },
            { "tomato", NSVG_RGB(255, 99, 71) },
            { "turquoise", NSVG_RGB(64, 224, 208) },
            { "violet", NSVG_RGB(238, 130, 238) },
            { "wheat", NSVG_RGB(245, 222, 179) },
            { "whitesmoke", NSVG_RGB(245, 245, 245) },
            { "yellowgreen", NSVG_RGB(154, 205, 50) },
        };

        unsigned int nsvg_parse_color_name(const char* str)
        {
            constexpr int ncolors = sizeof(nsvg_colors) / sizeof(NSVGNamedColor);

            for (int i = 0; i < ncolors; i++)
                if (std::strcmp(nsvg_colors[i].name, str) == 0)
                    return nsvg_colors[i].color;

            return NSVG_RGB(128, 128, 128);
        }

        unsigned int nsvg_parse_color(const char* str)
        {
            size_t len = 0;
            while (*str == ' ')
                ++str;
            len = strlen(str);
            if (len >= 1 && *str == '#')
                return nsvg_parse_color_hex(str);
            if (len >= 4 && str[0] == 'r' && str[1] == 'g' && str[2] == 'b' && str[3] == '(')
                return nsvg_parse_color_rgb(str);
            return nsvg_parse_color_name(str);
        }

        float nsvg_parse_opacity(const char* str)
        {
            float val = nsvg_atof(str);
            if (val < 0.0f)
                val = 0.0f;
            if (val > 1.0f)
                val = 1.0f;
            return val;
        }

        float nsvg_parse_miter_limit(const char* str)
        {
            float val = nsvg_atof(str);
            if (val < 0.0f)
                val = 0.0f;
            return val;
        }

        int nsvg_parse_units(const char* units)
        {
            if (units[0] == 'p' && units[1] == 'x')
                return NSVGUnitsPx;
            if (units[0] == 'p' && units[1] == 't')
                return NSVGUnitsPt;
            if (units[0] == 'p' && units[1] == 'c')
                return NSVGUnitsPc;
            if (units[0] == 'm' && units[1] == 'm')
                return NSVGUnitsMm;
            if (units[0] == 'c' && units[1] == 'm')
                return NSVGUnitsCm;
            if (units[0] == 'i' && units[1] == 'n')
                return NSVGUnitsIn;
            if (units[0] == '%')
                return NSVGUnitsPercent;
            if (units[0] == 'e' && units[1] == 'm')
                return NSVGUnitsEm;
            if (units[0] == 'e' && units[1] == 'x')
                return NSVGUnitsEx;
            return NSVGUnitsUser;
        }

        int nsvg_is_coordinate(const char* s)
        {
            // optional sign
            if (*s == '-' || *s == '+')
                s++;
            // must have at least one digit, or start by a dot
            return nsvg_isdigit(*s) || *s == '.';
        }

        NSVGcoordinate nsvg_parse_coordinate_raw(const char* str)
        {
            NSVGcoordinate coord = { 0, NSVGUnitsUser };
            char buf[64];
            coord.units = nsvg_parse_units(nsvg_parse_number(str, buf, 64));
            coord.value = static_cast<float>(nsvg_atof(buf));
            return coord;
        }

        NSVGcoordinate nsvg_coord(const float v, const int units)
        {
            const NSVGcoordinate coord = { v, units };
            return coord;
        }

        float nsvg_parse_coordinate(NSVGparser* p, const char* str, const float orig,
                                    const float length)
        {
            const NSVGcoordinate coord = nsvg_parse_coordinate_raw(str);
            return nsvg_convert_to_pixels(p, coord, orig, length);
        }

        int nsvg_parse_transform_args(const char* str, float* args, const int max_na, int* na)
        {
            char it[64];

            *na = 0;
            const char* ptr = str;
            while (*ptr && *ptr != '(')
                ++ptr;
            if (*ptr == 0)
                return 1;
            const char* end = ptr;
            while (*end && *end != ')')
                ++end;
            if (*end == 0)
                return 1;

            while (ptr < end)
                if (*ptr == '-' || *ptr == '+' || *ptr == '.' || nsvg_isdigit(*ptr)) {
                    if (*na >= max_na)
                        return 0;
                    ptr = nsvg_parse_number(ptr, it, 64);
                    args[(*na)++] = static_cast<float>(nsvg_atof(it));
                }
                else
                    ++ptr;
            return static_cast<int>(end - str);
        }

        int nsvg_parse_matrix(float* xform, const char* str)
        {
            float t[6];
            int na = 0;
            const int len = nsvg_parse_transform_args(str, t, 6, &na);
            if (na != 6)
                return len;
            std::memcpy(xform, t, sizeof(float) * 6);
            return len;
        }

        int nsvg_parse_translate(float* xform, const char* str)
        {
            float args[2];
            float t[6];
            int na = 0;
            const int len = nsvg_parse_transform_args(str, args, 2, &na);
            if (na == 1)
                args[1] = 0.0;

            nsvg_xform_set_translation(t, args[0], args[1]);
            std::memcpy(xform, t, sizeof(float) * 6);
            return len;
        }

        int nsvg_parse_scale(float* xform, const char* str)
        {
            float args[2];
            int na = 0;
            float t[6];
            const int len = nsvg_parse_transform_args(str, args, 2, &na);
            if (na == 1)
                args[1] = args[0];
            nsvg_xform_set_scale(t, args[0], args[1]);
            std::memcpy(xform, t, sizeof(float) * 6);
            return len;
        }

        int nsvg_parse_skew_x(float* xform, const char* str)
        {
            float args[1];
            int na = 0;
            float t[6];
            const int len = nsvg_parse_transform_args(str, args, 1, &na);
            nsvg_xform_set_skew_x(t, args[0] / 180.0f * std::numbers::pi_v<float>);
            std::memcpy(xform, t, sizeof(float) * 6);
            return len;
        }

        int nsvg_parse_skew_y(float* xform, const char* str)
        {
            float args[1];
            int na = 0;
            float t[6];
            const int len = nsvg_parse_transform_args(str, args, 1, &na);
            nsvg_xform_set_skew_y(t, args[0] / 180.0f * std::numbers::pi_v<float>);
            std::memcpy(xform, t, sizeof(float) * 6);
            return len;
        }

        int nsvg_parse_rotate(float* xform, const char* str)
        {
            float args[3];
            int na = 0;
            float m[6];
            float t[6];

            const int len = nsvg_parse_transform_args(str, args, 3, &na);
            if (na == 1)
                args[1] = args[2] = 0.0f;

            nsvg_xform_identity(m);

            if (na > 1) {
                nsvg_xform_set_translation(t, -args[1], -args[2]);
                nsvg_xform_multiply(m, t);
            }

            nsvg_xform_set_rotation(t, args[0] / 180.0f * std::numbers::pi_v<float>);
            nsvg_xform_multiply(m, t);

            if (na > 1) {
                nsvg_xform_set_translation(t, args[1], args[2]);
                nsvg_xform_multiply(m, t);
            }

            std::memcpy(xform, m, sizeof(float) * 6);

            return len;
        }

        void nsvg_parse_transform(float* xform, const char* str)
        {
            float t[6];
            int len;
            nsvg_xform_identity(xform);
            while (*str) {
                if (strncmp(str, "matrix", 6) == 0)
                    len = nsvg_parse_matrix(t, str);
                else if (strncmp(str, "translate", 9) == 0)
                    len = nsvg_parse_translate(t, str);
                else if (strncmp(str, "scale", 5) == 0)
                    len = nsvg_parse_scale(t, str);
                else if (strncmp(str, "rotate", 6) == 0)
                    len = nsvg_parse_rotate(t, str);
                else if (strncmp(str, "skewX", 5) == 0)
                    len = nsvg_parse_skew_x(t, str);
                else if (strncmp(str, "skewY", 5) == 0)
                    len = nsvg_parse_skew_y(t, str);
                else {
                    ++str;
                    continue;
                }
                if (len != 0)
                    str += len;
                else {
                    ++str;
                    continue;
                }

                nsvg_xform_premultiply(xform, t);
            }
        }

        void nsvg_parse_url(char* id, const char* str)
        {
            int i = 0;
            str += 4;  // "url(";
            if (*str && *str == '#')
                str++;
            while (i < 63 && *str && *str != ')') {
                id[i] = *str++;
                i++;
            }
            id[i] = '\0';
        }

        char nsvg_parse_line_cap(const char* str)
        {
            if (std::strcmp(str, "butt") == 0)
                return NSVGCapButt;
            if (std::strcmp(str, "round") == 0)
                return NSVGCapRound;
            if (std::strcmp(str, "square") == 0)
                return NSVGCapSquare;
            // TODO: handle inherit.
            return NSVGCapButt;
        }

        char nsvg_parse_line_join(const char* str)
        {
            if (std::strcmp(str, "miter") == 0)
                return NSVGJoinMiter;
            if (std::strcmp(str, "round") == 0)
                return NSVGJoinRound;
            if (std::strcmp(str, "bevel") == 0)
                return NSVGJoinBevel;
            // TODO: handle inherit.
            return NSVGJoinMiter;
        }

        char nsvg_parse_fill_rule(const char* str)
        {
            if (std::strcmp(str, "nonzero") == 0)
                return NSVGFillruleNonzero;
            if (std::strcmp(str, "evenodd") == 0)
                return NSVGFillruleEvenodd;
            // TODO: handle inherit.
            return NSVGFillruleNonzero;
        }

        const char* nsvg_get_next_dash_item(const char* s, char* it)
        {
            int n = 0;
            it[0] = '\0';
            // Skip white spaces and commas
            while (*s && (nsvg_isspace(*s) || *s == ','))
                s++;
            // Advance until whitespace, comma or end.
            while (*s && (!nsvg_isspace(*s) && *s != ',')) {
                if (n < 63)
                    it[n++] = *s;
                s++;
            }
            it[n++] = '\0';
            return s;
        }

        int nsvg_parse_stroke_dash_array(NSVGparser* p, const char* str, float* stroke_dash_array)
        {
            char item[64];
            int count = 0;
            float sum = 0.0f;

            // Handle "none"
            if (str[0] == 'n')
                return 0;

            // Parse dashes
            while (*str) {
                str = nsvg_get_next_dash_item(str, item);
                if (!*item)
                    break;
                if (count < NSVG_MAX_DASHES)
                    stroke_dash_array[count++] = fabsf(
                        nsvg_parse_coordinate(p, item, 0.0f, nsvg_actual_length(p)));
            }

            for (int i = 0; i < count; i++)
                sum += stroke_dash_array[i];
            if (sum <= 1e-6f)
                count = 0;

            return count;
        }

        void nsvg_parse_style(NSVGparser* p, const char* str);

        int nsvg_parse_attr(NSVGparser* p, const char* name, const char* value)
        {
            float xform[6];
            NSVGattrib* attr = nsvg_get_attr(p);
            if (!attr)
                return 0;

            if (std::strcmp(name, "style") == 0)
                nsvg_parse_style(p, value);
            else if (std::strcmp(name, "display") == 0) {
                if (std::strcmp(value, "none") == 0)
                    attr->visible = 0;
                // Don't reset ->visible on display:inline, one display:none hides the whole subtree
            }
            else if (std::strcmp(name, "fill") == 0) {
                if (std::strcmp(value, "none") == 0)
                    attr->has_fill = 0;
                else if (strncmp(value, "url(", 4) == 0) {
                    attr->has_fill = 2;
                    nsvg_parse_url(attr->fill_gradient, value);
                }
                else {
                    attr->has_fill = 1;
                    attr->fill_color = nsvg_parse_color(value);
                }
            }
            else if (std::strcmp(name, "opacity") == 0)
                attr->opacity = nsvg_parse_opacity(value);
            else if (std::strcmp(name, "fill-opacity") == 0)
                attr->fill_opacity = nsvg_parse_opacity(value);
            else if (std::strcmp(name, "stroke") == 0) {
                if (std::strcmp(value, "none") == 0)
                    attr->has_stroke = 0;
                else if (strncmp(value, "url(", 4) == 0) {
                    attr->has_stroke = 2;
                    nsvg_parse_url(attr->stroke_gradient, value);
                }
                else {
                    attr->has_stroke = 1;
                    attr->stroke_color = nsvg_parse_color(value);
                }
            }
            else if (std::strcmp(name, "stroke-width") == 0)
                attr->stroke_width = nsvg_parse_coordinate(p, value, 0.0f, nsvg_actual_length(p));
            else if (std::strcmp(name, "stroke-dasharray") == 0)
                attr->stroke_dash_count = nsvg_parse_stroke_dash_array(p, value,
                                                                       attr->stroke_dash_array);
            else if (std::strcmp(name, "stroke-dashoffset") == 0)
                attr->stroke_dash_offset = nsvg_parse_coordinate(p, value, 0.0f,
                                                                 nsvg_actual_length(p));
            else if (std::strcmp(name, "stroke-opacity") == 0)
                attr->stroke_opacity = nsvg_parse_opacity(value);
            else if (std::strcmp(name, "stroke-linecap") == 0)
                attr->stroke_line_cap = nsvg_parse_line_cap(value);
            else if (std::strcmp(name, "stroke-linejoin") == 0)
                attr->stroke_line_join = nsvg_parse_line_join(value);
            else if (std::strcmp(name, "stroke-miterlimit") == 0)
                attr->miter_limit = nsvg_parse_miter_limit(value);
            else if (std::strcmp(name, "fill-rule") == 0)
                attr->fill_rule = nsvg_parse_fill_rule(value);
            else if (std::strcmp(name, "font-size") == 0)
                attr->font_size = nsvg_parse_coordinate(p, value, 0.0f, nsvg_actual_length(p));
            else if (std::strcmp(name, "transform") == 0) {
                nsvg_parse_transform(xform, value);
                nsvg_xform_premultiply(attr->xform, xform);
            }
            else if (std::strcmp(name, "stop-color") == 0)
                attr->stop_color = nsvg_parse_color(value);
            else if (std::strcmp(name, "stop-opacity") == 0)
                attr->stop_opacity = nsvg_parse_opacity(value);
            else if (std::strcmp(name, "offset") == 0)
                attr->stop_offset = nsvg_parse_coordinate(p, value, 0.0f, 1.0f);
            else if (std::strcmp(name, "id") == 0) {
                strncpy(attr->id, value, 63);
                attr->id[63] = '\0';
            }
            else
                return 0;
            return 1;
        }

        int nsvg_parse_name_value(NSVGparser* p, const char* start, const char* end)
        {
            char name[512];
            char value[512];

            const char* str = start;
            while (str < end && *str != ':')
                ++str;

            const char* val = str;

            // Right Trim
            while (str > start && (*str == ':' || nsvg_isspace(*str)))
                --str;
            ++str;

            int n = static_cast<int>(str - start);
            if (n > 511)
                n = 511;
            if (n)
                std::memcpy(name, start, n);
            name[n] = 0;

            while (val < end && (*val == ':' || nsvg_isspace(*val)))
                ++val;

            n = static_cast<int>(end - val);
            if (n > 511)
                n = 511;
            if (n)
                std::memcpy(value, val, n);
            value[n] = 0;

            return nsvg_parse_attr(p, name, value);
        }

        void nsvg_parse_style(NSVGparser* p, const char* str)
        {
            while (*str) {
                // Left Trim
                while (*str && nsvg_isspace(*str))
                    ++str;
                const char* start = str;
                while (*str && *str != ';')
                    ++str;
                const char* end = str;

                // Right Trim
                while (end > start && (*end == ';' || nsvg_isspace(*end)))
                    --end;
                ++end;

                nsvg_parse_name_value(p, start, end);
                if (*str)
                    ++str;
            }
        }

        void nsvg_parse_attribs(NSVGparser* p, const char** attr)
        {
            for (int i = 0; attr[i]; i += 2)
                if (std::strcmp(attr[i], "style") == 0)
                    nsvg_parse_style(p, attr[i + 1]);
                else
                    nsvg_parse_attr(p, attr[i], attr[i + 1]);
        }

        int nsvg_get_args_per_element(const char cmd)
        {
            switch (cmd) {
                case 'v':
                case 'V':
                case 'h':
                case 'H':
                    return 1;
                case 'm':
                case 'M':
                case 'l':
                case 'L':
                case 't':
                case 'T':
                    return 2;
                case 'q':
                case 'Q':
                case 's':
                case 'S':
                    return 4;
                case 'c':
                case 'C':
                    return 6;
                case 'a':
                case 'A':
                    return 7;
                case 'z':
                case 'Z':
                    return 0;
            }
            return -1;
        }

        void nsvg_path_move_to(NSVGparser* p, float* cpx, float* cpy, const float* args,
                               const int rel)
        {
            if (rel) {
                *cpx += args[0];
                *cpy += args[1];
            }
            else {
                *cpx = args[0];
                *cpy = args[1];
            }
            nsvg_move_to(p, *cpx, *cpy);
        }

        void nsvg_path_line_to(NSVGparser* p, float* cpx, float* cpy, const float* args,
                               const int rel)
        {
            if (rel) {
                *cpx += args[0];
                *cpy += args[1];
            }
            else {
                *cpx = args[0];
                *cpy = args[1];
            }
            nsvg_line_to(p, *cpx, *cpy);
        }

        void nsvg_path_h_line_to(NSVGparser* p, float* cpx, const float* cpy, const float* args,
                                 const int rel)
        {
            if (rel)
                *cpx += args[0];
            else
                *cpx = args[0];
            nsvg_line_to(p, *cpx, *cpy);
        }

        void nsvg_path_v_line_to(NSVGparser* p, const float* cpx, float* cpy, const float* args,
                                 const int rel)
        {
            if (rel)
                *cpy += args[0];
            else
                *cpy = args[0];
            nsvg_line_to(p, *cpx, *cpy);
        }

        void nsvg_path_cubic_bez_to(NSVGparser* p, float* cpx, float* cpy, float* cpx2, float* cpy2,
                                    const float* args, const int rel)
        {
            float x2, y2, cx1, cy1, cx2, cy2;

            if (rel) {
                cx1 = *cpx + args[0];
                cy1 = *cpy + args[1];
                cx2 = *cpx + args[2];
                cy2 = *cpy + args[3];
                x2 = *cpx + args[4];
                y2 = *cpy + args[5];
            }
            else {
                cx1 = args[0];
                cy1 = args[1];
                cx2 = args[2];
                cy2 = args[3];
                x2 = args[4];
                y2 = args[5];
            }

            nsvg_cubic_bez_to(p, cx1, cy1, cx2, cy2, x2, y2);

            *cpx2 = cx2;
            *cpy2 = cy2;
            *cpx = x2;
            *cpy = y2;
        }

        void nsvg_path_cubic_bez_short_to(NSVGparser* p, float* cpx, float* cpy, float* cpx2,
                                          float* cpy2, const float* args, const int rel)
        {
            float x2, y2, cx2, cy2;

            const float x1 = *cpx;
            const float y1 = *cpy;
            if (rel) {
                cx2 = *cpx + args[0];
                cy2 = *cpy + args[1];
                x2 = *cpx + args[2];
                y2 = *cpy + args[3];
            }
            else {
                cx2 = args[0];
                cy2 = args[1];
                x2 = args[2];
                y2 = args[3];
            }

            const float cx1 = 2 * x1 - *cpx2;
            const float cy1 = 2 * y1 - *cpy2;

            nsvg_cubic_bez_to(p, cx1, cy1, cx2, cy2, x2, y2);

            *cpx2 = cx2;
            *cpy2 = cy2;
            *cpx = x2;
            *cpy = y2;
        }

        void nsvg_path_quad_bez_to(NSVGparser* p, float* cpx, float* cpy, float* cpx2, float* cpy2,
                                   const float* args, const int rel)
        {
            float x2, y2, cx, cy;

            const float x1 = *cpx;
            const float y1 = *cpy;
            if (rel) {
                cx = *cpx + args[0];
                cy = *cpy + args[1];
                x2 = *cpx + args[2];
                y2 = *cpy + args[3];
            }
            else {
                cx = args[0];
                cy = args[1];
                x2 = args[2];
                y2 = args[3];
            }

            // Convert to cubic bezier
            const float cx1 = x1 + 2.0f / 3.0f * (cx - x1);
            const float cy1 = y1 + 2.0f / 3.0f * (cy - y1);
            const float cx2 = x2 + 2.0f / 3.0f * (cx - x2);
            const float cy2 = y2 + 2.0f / 3.0f * (cy - y2);

            nsvg_cubic_bez_to(p, cx1, cy1, cx2, cy2, x2, y2);

            *cpx2 = cx;
            *cpy2 = cy;
            *cpx = x2;
            *cpy = y2;
        }

        void nsvg_path_quad_bez_short_to(NSVGparser* p, float* cpx, float* cpy, float* cpx2,
                                         float* cpy2, const float* args, const int rel)
        {
            float x2, y2;
            const float x1 = *cpx;
            const float y1 = *cpy;
            if (rel) {
                x2 = *cpx + args[0];
                y2 = *cpy + args[1];
            }
            else {
                x2 = args[0];
                y2 = args[1];
            }

            const float cx = 2 * x1 - *cpx2;
            const float cy = 2 * y1 - *cpy2;

            // Convert to cubix bezier
            const float cx1 = x1 + 2.0f / 3.0f * (cx - x1);
            const float cy1 = y1 + 2.0f / 3.0f * (cy - y1);
            const float cx2 = x2 + 2.0f / 3.0f * (cx - x2);
            const float cy2 = y2 + 2.0f / 3.0f * (cy - y2);

            nsvg_cubic_bez_to(p, cx1, cy1, cx2, cy2, x2, y2);

            *cpx2 = cx;
            *cpy2 = cy;
            *cpx = x2;
            *cpy = y2;
        }

        float nsvg_sqr(const float x)
        {
            return x * x;
        }

        float nsvg_vmag(const float x, const float y)
        {
            return sqrtf(x * x + y * y);
        }

        float nsvg_vecrat(const float ux, const float uy, const float vx, const float vy)
        {
            return (ux * vx + uy * vy) / (nsvg_vmag(ux, uy) * nsvg_vmag(vx, vy));
        }

        float nsvg_vecang(const float ux, const float uy, const float vx, const float vy)
        {
            float r = nsvg_vecrat(ux, uy, vx, vy);
            if (r < -1.0f)
                r = -1.0f;
            if (r > 1.0f)
                r = 1.0f;
            return (ux * vy < uy * vx ? -1.0f : 1.0f) * acosf(r);
        }

        void nsvg_path_arc_to(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
        {
            float rx;
            float ry;
            float rotx;
            float x1;
            float y1;
            float x2;
            float y2;
            float cx;
            float cy;
            float dx;
            float dy;
            float d;
            float x1_p;
            float y1_p;
            float cxp;
            float cyp;
            float s;
            float sa;
            float sb;
            float ux;
            float uy;
            float vx;
            float vy;
            float a1;
            float da;
            float x;
            float y;
            float tanx;
            float tany;
            float a;
            float px = 0;
            float py = 0;
            float ptanx = 0;
            float ptany = 0;
            float t[6];
            float sinrx;
            float cosrx;
            int fa;
            int fs;
            int i;
            int ndivs;
            float hda;
            float kappa;

            rx = std::fabsf(args[0]);                             // y radius
            ry = std::fabsf(args[1]);                             // x radius
            rotx = args[2] / 180.0f * std::numbers::pi_v<float>;  // x rotation angle
            fa = std::fabsf(args[3]) > 1e-6 ? 1 : 0;              // Large arc
            fs = std::fabsf(args[4]) > 1e-6 ? 1 : 0;              // Sweep direction
            x1 = *cpx;                                            // start point
            y1 = *cpy;
            if (rel) {  // end point
                x2 = *cpx + args[5];
                y2 = *cpy + args[6];
            }
            else {
                x2 = args[5];
                y2 = args[6];
            }

            dx = x1 - x2;
            dy = y1 - y2;
            d = std::sqrtf(dx * dx + dy * dy);
            if (d < 1e-6f || rx < 1e-6f || ry < 1e-6f) {
                // The arc degenerates to a line
                nsvg_line_to(p, x2, y2);
                *cpx = x2;
                *cpy = y2;
                return;
            }

            sinrx = std::sinf(rotx);
            cosrx = std::cosf(rotx);

            // Convert to center point parameterization.
            // http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
            // 1) Compute x1', y1'
            x1_p = cosrx * dx / 2.0f + sinrx * dy / 2.0f;
            y1_p = -sinrx * dx / 2.0f + cosrx * dy / 2.0f;
            d = nsvg_sqr(x1_p) / nsvg_sqr(rx) + nsvg_sqr(y1_p) / nsvg_sqr(ry);
            if (d > 1) {
                d = sqrtf(d);
                rx *= d;
                ry *= d;
            }
            // 2) Compute cx', cy'
            s = 0.0f;
            sa = nsvg_sqr(rx) * nsvg_sqr(ry) - nsvg_sqr(rx) * nsvg_sqr(y1_p) -
                 nsvg_sqr(ry) * nsvg_sqr(x1_p);
            sb = nsvg_sqr(rx) * nsvg_sqr(y1_p) + nsvg_sqr(ry) * nsvg_sqr(x1_p);
            if (sa < 0.0f)
                sa = 0.0f;
            if (sb > 0.0f)
                s = sqrtf(sa / sb);
            if (fa == fs)
                s = -s;
            cxp = s * rx * y1_p / ry;
            cyp = s * -ry * x1_p / rx;

            // 3) Compute cx,cy from cx',cy'
            cx = (x1 + x2) / 2.0f + cosrx * cxp - sinrx * cyp;
            cy = (y1 + y2) / 2.0f + sinrx * cxp + cosrx * cyp;

            // 4) Calculate theta1, and delta theta.
            ux = (x1_p - cxp) / rx;
            uy = (y1_p - cyp) / ry;
            vx = (-x1_p - cxp) / rx;
            vy = (-y1_p - cyp) / ry;
            a1 = nsvg_vecang(1.0f, 0.0f, ux, uy);  // Initial angle
            da = nsvg_vecang(ux, uy, vx, vy);      // Delta angle

            //  if (vecrat(ux,uy,vx,vy) <= -1.0f) da = std::numbers::pi_v<float>;
            //  if (vecrat(ux,uy,vx,vy) >= 1.0f) da = 0;

            if (fs == 0 && da > 0)
                da -= 2 * std::numbers::pi_v<float>;
            else if (fs == 1 && da < 0)
                da += 2 * std::numbers::pi_v<float>;

            // Approximate the arc using cubic spline segments.
            t[0] = cosrx;
            t[1] = sinrx;
            t[2] = -sinrx;
            t[3] = cosrx;
            t[4] = cx;
            t[5] = cy;

            // Split arc into max 90 degree segments.
            // The loop assumes an iteration per end point (including start and end), this +1.
            ndivs = static_cast<int>(fabsf(da) / (std::numbers::pi_v<float> * 0.5f) + 1.0f);
            hda = da / static_cast<float>(ndivs) / 2.0f;
            // Fix for ticket #179: division by 0: avoid cotangens around 0 (infinite)
            if (hda < 1e-3f && hda > -1e-3f)
                hda *= 0.5f;
            else
                hda = (1.0f - cosf(hda)) / sinf(hda);
            kappa = fabsf(4.0f / 3.0f * hda);
            if (da < 0.0f)
                kappa = -kappa;

            for (i = 0; i <= ndivs; i++) {
                a = a1 + da * (static_cast<float>(i) / static_cast<float>(ndivs));
                dx = cosf(a);
                dy = sinf(a);
                nsvg_xform_point(&x, &y, dx * rx, dy * ry, t);                       // position
                nsvg_xform_vec(&tanx, &tany, -dy * rx * kappa, dx * ry * kappa, t);  // tangent
                if (i > 0)
                    nsvg_cubic_bez_to(p, px + ptanx, py + ptany, x - tanx, y - tany, x, y);
                px = x;
                py = y;
                ptanx = tanx;
                ptany = tany;
            }

            *cpx = x2;
            *cpy = y2;
        }

        void nsvg_parse_path(NSVGparser* p, const char** attr)
        {
            const char* s = nullptr;
            char cmd = '\0';
            float args[10];
            int rargs = 0;
            float cpx, cpy, cpx2, cpy2;
            const char* tmp[4];
            char item[64];

            for (int i = 0; attr[i]; i += 2)
                if (std::strcmp(attr[i], "d") == 0)
                    s = attr[i + 1];
                else {
                    tmp[0] = attr[i];
                    tmp[1] = attr[i + 1];
                    tmp[2] = nullptr;
                    tmp[3] = nullptr;
                    nsvg_parse_attribs(p, tmp);
                }

            if (s != nullptr) {
                nsvg_reset_path(p);
                cpx = 0;
                cpy = 0;
                cpx2 = 0;
                cpy2 = 0;
                char init_point = 0;
                char closed_flag = 0;
                int nargs = 0;

                while (*s) {
                    item[0] = '\0';
                    if ((cmd == 'A' || cmd == 'a') && (nargs == 3 || nargs == 4))
                        s = nsvg_get_next_path_item_when_arc_flag(s, item);
                    if (!*item)
                        s = nsvg_get_next_path_item(s, item);
                    if (!*item)
                        break;
                    if (cmd != '\0' && nsvg_is_coordinate(item)) {
                        if (nargs < 10)
                            args[nargs++] = static_cast<float>(nsvg_atof(item));
                        if (nargs >= rargs) {
                            switch (cmd) {
                                case 'm':
                                case 'M':
                                    nsvg_path_move_to(p, &cpx, &cpy, args, cmd == 'm' ? 1 : 0);
                                    // Moveto can be followed by multiple coordinate pairs,
                                    // which should be treated as linetos.
                                    cmd = cmd == 'm' ? 'l' : 'L';
                                    rargs = nsvg_get_args_per_element(cmd);
                                    cpx2 = cpx;
                                    cpy2 = cpy;
                                    init_point = 1;
                                    break;
                                case 'l':
                                case 'L':
                                    nsvg_path_line_to(p, &cpx, &cpy, args, cmd == 'l' ? 1 : 0);
                                    cpx2 = cpx;
                                    cpy2 = cpy;
                                    break;
                                case 'H':
                                case 'h':
                                    nsvg_path_h_line_to(p, &cpx, &cpy, args, cmd == 'h' ? 1 : 0);
                                    cpx2 = cpx;
                                    cpy2 = cpy;
                                    break;
                                case 'V':
                                case 'v':
                                    nsvg_path_v_line_to(p, &cpx, &cpy, args, cmd == 'v' ? 1 : 0);
                                    cpx2 = cpx;
                                    cpy2 = cpy;
                                    break;
                                case 'C':
                                case 'c':
                                    nsvg_path_cubic_bez_to(p, &cpx, &cpy, &cpx2, &cpy2, args,
                                                           cmd == 'c' ? 1 : 0);
                                    break;
                                case 'S':
                                case 's':
                                    nsvg_path_cubic_bez_short_to(p, &cpx, &cpy, &cpx2, &cpy2, args,
                                                                 cmd == 's' ? 1 : 0);
                                    break;
                                case 'Q':
                                case 'q':
                                    nsvg_path_quad_bez_to(p, &cpx, &cpy, &cpx2, &cpy2, args,
                                                          cmd == 'q' ? 1 : 0);
                                    break;
                                case 'T':
                                case 't':
                                    nsvg_path_quad_bez_short_to(p, &cpx, &cpy, &cpx2, &cpy2, args,
                                                                cmd == 't' ? 1 : 0);
                                    break;
                                case 'A':
                                case 'a':
                                    nsvg_path_arc_to(p, &cpx, &cpy, args, cmd == 'a' ? 1 : 0);
                                    cpx2 = cpx;
                                    cpy2 = cpy;
                                    break;
                                default:
                                    if (nargs >= 2) {
                                        cpx = args[nargs - 2];
                                        cpy = args[nargs - 1];
                                        cpx2 = cpx;
                                        cpy2 = cpy;
                                    }
                                    break;
                            }
                            nargs = 0;
                        }
                    }
                    else {
                        cmd = item[0];
                        if (cmd == 'M' || cmd == 'm') {
                            // Commit path.
                            if (p->npts > 0)
                                nsvg_add_path(p, closed_flag);
                            // Start new subpath.
                            nsvg_reset_path(p);
                            closed_flag = 0;
                            nargs = 0;
                        }
                        else if (init_point == 0)
                            // Do not allow other commands until initial point has been set (moveTo
                            // called once).
                            cmd = '\0';
                        if (cmd == 'Z' || cmd == 'z') {
                            closed_flag = 1;
                            // Commit path.
                            if (p->npts > 0) {
                                // Move current point to first point
                                cpx = p->pts[0];
                                cpy = p->pts[1];
                                cpx2 = cpx;
                                cpy2 = cpy;
                                nsvg_add_path(p, closed_flag);
                            }
                            // Start new subpath.
                            nsvg_reset_path(p);
                            nsvg_move_to(p, cpx, cpy);
                            closed_flag = 0;
                            nargs = 0;
                        }
                        rargs = nsvg_get_args_per_element(cmd);
                        if (rargs == -1) {
                            // Command not recognized
                            cmd = '\0';
                            rargs = 0;
                        }
                    }
                }
                // Commit path.
                if (p->npts)
                    nsvg_add_path(p, closed_flag);
            }

            nsvg_add_shape(p);
        }

        void nsvg_parse_rect(NSVGparser* p, const char** attr)
        {
            float x = 0.0f;
            float y = 0.0f;
            float w = 0.0f;
            float h = 0.0f;
            float rx = -1.0f;  // marks not set
            float ry = -1.0f;

            for (int i = 0; attr[i]; i += 2)
                if (!nsvg_parse_attr(p, attr[i], attr[i + 1])) {
                    if (std::strcmp(attr[i], "x") == 0)
                        x = nsvg_parse_coordinate(p, attr[i + 1], nsvg_actual_orig_x(p),
                                                  nsvg_actual_width(p));
                    if (std::strcmp(attr[i], "y") == 0)
                        y = nsvg_parse_coordinate(p, attr[i + 1], nsvg_actual_orig_y(p),
                                                  nsvg_actual_height(p));
                    if (std::strcmp(attr[i], "width") == 0)
                        w = nsvg_parse_coordinate(p, attr[i + 1], 0.0f, nsvg_actual_width(p));
                    if (std::strcmp(attr[i], "height") == 0)
                        h = nsvg_parse_coordinate(p, attr[i + 1], 0.0f, nsvg_actual_height(p));
                    if (std::strcmp(attr[i], "rx") == 0)
                        rx = std::fabsf(
                            nsvg_parse_coordinate(p, attr[i + 1], 0.0f, nsvg_actual_width(p)));
                    if (std::strcmp(attr[i], "ry") == 0)
                        ry = std::fabsf(
                            nsvg_parse_coordinate(p, attr[i + 1], 0.0f, nsvg_actual_height(p)));
                }

            if (rx < 0.0f && ry > 0.0f)
                rx = ry;
            if (ry < 0.0f && rx > 0.0f)
                ry = rx;
            if (rx < 0.0f)
                rx = 0.0f;
            if (ry < 0.0f)
                ry = 0.0f;
            if (rx > w / 2.0f)
                rx = w / 2.0f;
            if (ry > h / 2.0f)
                ry = h / 2.0f;

            if (w != 0.0f && h != 0.0f) {
                nsvg_reset_path(p);

                if (rx < 0.00001f || ry < 0.0001f) {
                    nsvg_move_to(p, x, y);
                    nsvg_line_to(p, x + w, y);
                    nsvg_line_to(p, x + w, y + h);
                    nsvg_line_to(p, x, y + h);
                }
                else {
                    // Rounded rectangle
                    nsvg_move_to(p, x + rx, y);
                    nsvg_line_to(p, x + w - rx, y);
                    nsvg_cubic_bez_to(p, x + w - rx * (1 - NSVG_KAPPA90), y, x + w,
                                      y + ry * (1 - NSVG_KAPPA90), x + w, y + ry);
                    nsvg_line_to(p, x + w, y + h - ry);
                    nsvg_cubic_bez_to(p, x + w, y + h - ry * (1 - NSVG_KAPPA90),
                                      x + w - rx * (1 - NSVG_KAPPA90), y + h, x + w - rx, y + h);
                    nsvg_line_to(p, x + rx, y + h);
                    nsvg_cubic_bez_to(p, x + rx * (1 - NSVG_KAPPA90), y + h, x,
                                      y + h - ry * (1 - NSVG_KAPPA90), x, y + h - ry);
                    nsvg_line_to(p, x, y + ry);
                    nsvg_cubic_bez_to(p, x, y + ry * (1 - NSVG_KAPPA90),
                                      x + rx * (1 - NSVG_KAPPA90), y, x + rx, y);
                }

                nsvg_add_path(p, 1);

                nsvg_add_shape(p);
            }
        }

        void nsvg_parse_circle(NSVGparser* p, const char** attr)
        {
            float cx = 0.0f;
            float cy = 0.0f;
            float r = 0.0f;

            for (int i = 0; attr[i]; i += 2)
                if (!nsvg_parse_attr(p, attr[i], attr[i + 1])) {
                    if (std::strcmp(attr[i], "cx") == 0)
                        cx = nsvg_parse_coordinate(p, attr[i + 1], nsvg_actual_orig_x(p),
                                                   nsvg_actual_width(p));
                    if (std::strcmp(attr[i], "cy") == 0)
                        cy = nsvg_parse_coordinate(p, attr[i + 1], nsvg_actual_orig_y(p),
                                                   nsvg_actual_height(p));
                    if (std::strcmp(attr[i], "r") == 0)
                        r = std::fabsf(
                            nsvg_parse_coordinate(p, attr[i + 1], 0.0f, nsvg_actual_length(p)));
                }

            if (r > 0.0f) {
                nsvg_reset_path(p);

                nsvg_move_to(p, cx + r, cy);
                nsvg_cubic_bez_to(p, cx + r, cy + r * NSVG_KAPPA90, cx + r * NSVG_KAPPA90, cy + r,
                                  cx, cy + r);
                nsvg_cubic_bez_to(p, cx - r * NSVG_KAPPA90, cy + r, cx - r, cy + r * NSVG_KAPPA90,
                                  cx - r, cy);
                nsvg_cubic_bez_to(p, cx - r, cy - r * NSVG_KAPPA90, cx - r * NSVG_KAPPA90, cy - r,
                                  cx, cy - r);
                nsvg_cubic_bez_to(p, cx + r * NSVG_KAPPA90, cy - r, cx + r, cy - r * NSVG_KAPPA90,
                                  cx + r, cy);

                nsvg_add_path(p, 1);

                nsvg_add_shape(p);
            }
        }

        void nsvg_parse_ellipse(NSVGparser* p, const char** attr)
        {
            float cx = 0.0f;
            float cy = 0.0f;
            float rx = 0.0f;
            float ry = 0.0f;

            for (int i = 0; attr[i]; i += 2)
                if (!nsvg_parse_attr(p, attr[i], attr[i + 1])) {
                    if (std::strcmp(attr[i], "cx") == 0)
                        cx = nsvg_parse_coordinate(p, attr[i + 1], nsvg_actual_orig_x(p),
                                                   nsvg_actual_width(p));
                    if (std::strcmp(attr[i], "cy") == 0)
                        cy = nsvg_parse_coordinate(p, attr[i + 1], nsvg_actual_orig_y(p),
                                                   nsvg_actual_height(p));
                    if (std::strcmp(attr[i], "rx") == 0)
                        rx = fabsf(
                            nsvg_parse_coordinate(p, attr[i + 1], 0.0f, nsvg_actual_width(p)));
                    if (std::strcmp(attr[i], "ry") == 0)
                        ry = fabsf(
                            nsvg_parse_coordinate(p, attr[i + 1], 0.0f, nsvg_actual_height(p)));
                }

            if (rx > 0.0f && ry > 0.0f) {
                nsvg_reset_path(p);

                nsvg_move_to(p, cx + rx, cy);
                nsvg_cubic_bez_to(p, cx + rx, cy + ry * NSVG_KAPPA90, cx + rx * NSVG_KAPPA90,
                                  cy + ry, cx, cy + ry);
                nsvg_cubic_bez_to(p, cx - rx * NSVG_KAPPA90, cy + ry, cx - rx,
                                  cy + ry * NSVG_KAPPA90, cx - rx, cy);
                nsvg_cubic_bez_to(p, cx - rx, cy - ry * NSVG_KAPPA90, cx - rx * NSVG_KAPPA90,
                                  cy - ry, cx, cy - ry);
                nsvg_cubic_bez_to(p, cx + rx * NSVG_KAPPA90, cy - ry, cx + rx,
                                  cy - ry * NSVG_KAPPA90, cx + rx, cy);

                nsvg_add_path(p, 1);

                nsvg_add_shape(p);
            }
        }

        void nsvg_parse_line(NSVGparser* p, const char** attr)
        {
            float x1 = 0.0;
            float y1 = 0.0;
            float x2 = 0.0;
            float y2 = 0.0;

            for (int i = 0; attr[i]; i += 2)
                if (!nsvg_parse_attr(p, attr[i], attr[i + 1])) {
                    if (std::strcmp(attr[i], "x1") == 0)
                        x1 = nsvg_parse_coordinate(p, attr[i + 1], nsvg_actual_orig_x(p),
                                                   nsvg_actual_width(p));
                    if (std::strcmp(attr[i], "y1") == 0)
                        y1 = nsvg_parse_coordinate(p, attr[i + 1], nsvg_actual_orig_y(p),
                                                   nsvg_actual_height(p));
                    if (std::strcmp(attr[i], "x2") == 0)
                        x2 = nsvg_parse_coordinate(p, attr[i + 1], nsvg_actual_orig_x(p),
                                                   nsvg_actual_width(p));
                    if (std::strcmp(attr[i], "y2") == 0)
                        y2 = nsvg_parse_coordinate(p, attr[i + 1], nsvg_actual_orig_y(p),
                                                   nsvg_actual_height(p));
                }

            nsvg_reset_path(p);

            nsvg_move_to(p, x1, y1);
            nsvg_line_to(p, x2, y2);

            nsvg_add_path(p, 0);

            nsvg_add_shape(p);
        }

        void nsvg_parse_poly(NSVGparser* p, const char** attr, const int closeFlag)
        {
            float args[2];
            int npts = 0;
            char item[64];

            nsvg_reset_path(p);

            for (int i = 0; attr[i]; i += 2)
                if (!nsvg_parse_attr(p, attr[i], attr[i + 1]))
                    if (std::strcmp(attr[i], "points") == 0) {
                        const char* s = attr[i + 1];
                        int nargs = 0;
                        while (*s) {
                            s = nsvg_get_next_path_item(s, item);
                            args[nargs++] = static_cast<float>(nsvg_atof(item));
                            if (nargs >= 2) {
                                if (npts == 0)
                                    nsvg_move_to(p, args[0], args[1]);
                                else
                                    nsvg_line_to(p, args[0], args[1]);
                                nargs = 0;
                                npts++;
                            }
                        }
                    }

            nsvg_add_path(p, static_cast<char>(closeFlag));

            nsvg_add_shape(p);
        }

        void nsvg_parse_svg(NSVGparser* p, const char** attr)
        {
            for (int i = 0; attr[i]; i += 2)
                if (!nsvg_parse_attr(p, attr[i], attr[i + 1])) {
                    if (std::strcmp(attr[i], "width") == 0)
                        p->image->width = nsvg_parse_coordinate(p, attr[i + 1], 0.0f, 0.0f);
                    else if (std::strcmp(attr[i], "height") == 0)
                        p->image->height = nsvg_parse_coordinate(p, attr[i + 1], 0.0f, 0.0f);
                    else if (std::strcmp(attr[i], "viewBox") == 0) {
                        const char* s = attr[i + 1];
                        char buf[64];
                        s = nsvg_parse_number(s, buf, 64);
                        p->view_minx = nsvg_atof(buf);
                        while (*s && (nsvg_isspace(*s) || *s == '%' || *s == ','))
                            s++;
                        if (!*s)
                            return;
                        s = nsvg_parse_number(s, buf, 64);
                        p->view_miny = nsvg_atof(buf);
                        while (*s && (nsvg_isspace(*s) || *s == '%' || *s == ','))
                            s++;
                        if (!*s)
                            return;
                        s = nsvg_parse_number(s, buf, 64);
                        p->view_width = nsvg_atof(buf);
                        while (*s && (nsvg_isspace(*s) || *s == '%' || *s == ','))
                            s++;
                        if (!*s)
                            return;
                        s = nsvg_parse_number(s, buf, 64);
                        p->view_height = nsvg_atof(buf);
                    }
                    else if (std::strcmp(attr[i], "preserveAspectRatio") == 0) {
                        if (std::strstr(attr[i + 1], "none") != nullptr)
                            // No uniform scaling
                            p->align_type = None;
                        else {
                            // Parse X align
                            if (std::strstr(attr[i + 1], "xMin") != nullptr)
                                p->align_x = Min;
                            else if (std::strstr(attr[i + 1], "xMid") != nullptr)
                                p->align_x = Mid;
                            else if (std::strstr(attr[i + 1], "xMax") != nullptr)
                                p->align_x = Max;
                            // Parse X align
                            if (std::strstr(attr[i + 1], "yMin") != nullptr)
                                p->align_y = Min;
                            else if (std::strstr(attr[i + 1], "yMid") != nullptr)
                                p->align_y = Mid;
                            else if (std::strstr(attr[i + 1], "yMax") != nullptr)
                                p->align_y = Max;
                            // Parse meet/slice
                            p->align_type = Meet;
                            if (std::strstr(attr[i + 1], "slice") != nullptr)
                                p->align_type = Slice;
                        }
                    }
                }
        }

        void nsvg_parse_gradient(NSVGparser* p, const char** attr, const signed char type)
        {
            NSVGgradientData* grad = static_cast<NSVGgradientData*>(
                std::malloc(sizeof(NSVGgradientData)));
            if (grad == nullptr)
                return;
            memset(grad, 0, sizeof(NSVGgradientData));
            grad->units = NSVGObjectSpace;
            grad->type = type;
            if (grad->type == NSVGPaintLinearGradient) {
                grad->linear.x1 = nsvg_coord(0.0f, NSVGUnitsPercent);
                grad->linear.y1 = nsvg_coord(0.0f, NSVGUnitsPercent);
                grad->linear.x2 = nsvg_coord(100.0f, NSVGUnitsPercent);
                grad->linear.y2 = nsvg_coord(0.0f, NSVGUnitsPercent);
            }
            else if (grad->type == NSVGPaintRadialGradient) {
                grad->radial.cx = nsvg_coord(50.0f, NSVGUnitsPercent);
                grad->radial.cy = nsvg_coord(50.0f, NSVGUnitsPercent);
                grad->radial.r = nsvg_coord(50.0f, NSVGUnitsPercent);
            }

            nsvg_xform_identity(grad->xform);
            int setfx = 0;
            int setfy = 0;

            for (int i = 0; attr[i]; i += 2)
                if (std::strcmp(attr[i], "id") == 0) {
                    strncpy(grad->id, attr[i + 1], 63);
                    grad->id[63] = '\0';
                }
                else if (!nsvg_parse_attr(p, attr[i], attr[i + 1])) {
                    if (std::strcmp(attr[i], "gradientUnits") == 0)
                        if (std::strcmp(attr[i + 1], "objectBoundingBox") == 0)
                            grad->units = NSVGObjectSpace;
                        else
                            grad->units = NSVGUserSpace;
                    else if (std::strcmp(attr[i], "gradientTransform") == 0)
                        nsvg_parse_transform(grad->xform, attr[i + 1]);
                    else if (std::strcmp(attr[i], "cx") == 0)
                        grad->radial.cx = nsvg_parse_coordinate_raw(attr[i + 1]);
                    else if (std::strcmp(attr[i], "cy") == 0)
                        grad->radial.cy = nsvg_parse_coordinate_raw(attr[i + 1]);
                    else if (std::strcmp(attr[i], "r") == 0)
                        grad->radial.r = nsvg_parse_coordinate_raw(attr[i + 1]);
                    else if (std::strcmp(attr[i], "fx") == 0) {
                        grad->radial.fx = nsvg_parse_coordinate_raw(attr[i + 1]);
                        setfx = 1;
                    }
                    else if (std::strcmp(attr[i], "fy") == 0) {
                        grad->radial.fy = nsvg_parse_coordinate_raw(attr[i + 1]);
                        setfy = 1;
                    }
                    else if (std::strcmp(attr[i], "x1") == 0)
                        grad->linear.x1 = nsvg_parse_coordinate_raw(attr[i + 1]);
                    else if (std::strcmp(attr[i], "y1") == 0)
                        grad->linear.y1 = nsvg_parse_coordinate_raw(attr[i + 1]);
                    else if (std::strcmp(attr[i], "x2") == 0)
                        grad->linear.x2 = nsvg_parse_coordinate_raw(attr[i + 1]);
                    else if (std::strcmp(attr[i], "y2") == 0)
                        grad->linear.y2 = nsvg_parse_coordinate_raw(attr[i + 1]);
                    else if (std::strcmp(attr[i], "spreadMethod") == 0) {
                        if (std::strcmp(attr[i + 1], "pad") == 0)
                            grad->spread = NSVGSpreadPad;
                        else if (std::strcmp(attr[i + 1], "reflect") == 0)
                            grad->spread = NSVGSpreadReflect;
                        else if (std::strcmp(attr[i + 1], "repeat") == 0)
                            grad->spread = NSVGSpreadRepeat;
                    }
                    else if (std::strcmp(attr[i], "xlink:href") == 0) {
                        const char* href = attr[i + 1];
                        strncpy(grad->ref, href + 1, 62);
                        grad->ref[62] = '\0';
                    }
                }

            if (grad->type == NSVGPaintRadialGradient && setfx == 0)
                grad->radial.fx = grad->radial.cx;

            if (grad->type == NSVGPaintRadialGradient && setfy == 0)
                grad->radial.fy = grad->radial.cy;

            grad->next = p->gradients;
            p->gradients = grad;
        }

        void nsvg_parse_gradient_stop(NSVGparser* p, const char** attr)
        {
            NSVGattrib* cur_attr = nsvg_get_attr(p);
            int i;

            cur_attr->stop_offset = 0;
            cur_attr->stop_color = 0;
            cur_attr->stop_opacity = 1.0f;

            for (i = 0; attr[i]; i += 2)
                nsvg_parse_attr(p, attr[i], attr[i + 1]);

            // Add stop to the last gradient.
            NSVGgradientData* grad = p->gradients;
            if (grad == nullptr)
                return;

            grad->nstops++;
            grad->stops = static_cast<NSVGgradientStop*>(std::realloc(grad->stops, sizeof(NSVGgradientStop) * grad->nstops));
            if (grad->stops == nullptr)
                return;

            // Insert
            int idx = grad->nstops - 1;
            for (i = 0; i < grad->nstops - 1; i++)
                if (cur_attr->stop_offset < grad->stops[i].offset) {
                    idx = i;
                    break;
                }
            if (idx != grad->nstops - 1)
                for (i = grad->nstops - 1; i > idx; i--)
                    grad->stops[i] = grad->stops[i - 1];

            NSVGgradientStop* stop = &grad->stops[idx];
            stop->color = cur_attr->stop_color;
            stop->color |= static_cast<unsigned int>(cur_attr->stop_opacity * 255) << 24;
            stop->offset = cur_attr->stop_offset;
        }

        void nsvg_start_element(void* ud, const char* el, const char** attr)
        {
            NSVGparser* p = static_cast<NSVGparser*>(ud);

            if (p->defs_flag) {
                // Skip everything but gradients in defs
                if (std::strcmp(el, "linearGradient") == 0)
                    nsvg_parse_gradient(p, attr, NSVGPaintLinearGradient);
                else if (std::strcmp(el, "radialGradient") == 0)
                    nsvg_parse_gradient(p, attr, NSVGPaintRadialGradient);
                else if (std::strcmp(el, "stop") == 0)
                    nsvg_parse_gradient_stop(p, attr);
                return;
            }

            if (std::strcmp(el, "g") == 0) {
                nsvg_push_attr(p);
                nsvg_parse_attribs(p, attr);
            }
            else if (std::strcmp(el, "path") == 0) {
                if (p->path_flag)  // Do not allow nested paths.
                    return;
                nsvg_push_attr(p);
                nsvg_parse_path(p, attr);
                nsvg_pop_attr(p);
            }
            else if (std::strcmp(el, "rect") == 0) {
                nsvg_push_attr(p);
                nsvg_parse_rect(p, attr);
                nsvg_pop_attr(p);
            }
            else if (std::strcmp(el, "circle") == 0) {
                nsvg_push_attr(p);
                nsvg_parse_circle(p, attr);
                nsvg_pop_attr(p);
            }
            else if (std::strcmp(el, "ellipse") == 0) {
                nsvg_push_attr(p);
                nsvg_parse_ellipse(p, attr);
                nsvg_pop_attr(p);
            }
            else if (std::strcmp(el, "line") == 0) {
                nsvg_push_attr(p);
                nsvg_parse_line(p, attr);
                nsvg_pop_attr(p);
            }
            else if (std::strcmp(el, "polyline") == 0) {
                nsvg_push_attr(p);
                nsvg_parse_poly(p, attr, 0);
                nsvg_pop_attr(p);
            }
            else if (std::strcmp(el, "polygon") == 0) {
                nsvg_push_attr(p);
                nsvg_parse_poly(p, attr, 1);
                nsvg_pop_attr(p);
            }
            else if (std::strcmp(el, "linearGradient") == 0)
                nsvg_parse_gradient(p, attr, NSVGPaintLinearGradient);
            else if (std::strcmp(el, "radialGradient") == 0)
                nsvg_parse_gradient(p, attr, NSVGPaintRadialGradient);
            else if (std::strcmp(el, "stop") == 0)
                nsvg_parse_gradient_stop(p, attr);
            else if (std::strcmp(el, "defs") == 0)
                p->defs_flag = 1;
            else if (std::strcmp(el, "svg") == 0)
                nsvg_parse_svg(p, attr);
        }

        void nsvg_end_element(void* ud, const char* el)
        {
            NSVGparser* p = static_cast<NSVGparser*>(ud);

            if (std::strcmp(el, "g") == 0)
                nsvg_pop_attr(p);
            else if (std::strcmp(el, "path") == 0)
                p->path_flag = 0;
            else if (std::strcmp(el, "defs") == 0)
                p->defs_flag = 0;
        }

        void nsvg_content(void* ud, const char* s)
        {
            NSVG_NOTUSED(ud);
            NSVG_NOTUSED(s);
            // empty
        }

        void nsvg_image_bounds(const NSVGparser* p, float* bounds)
        {
            const NSVGshape* shape = p->image->shapes;
            if (shape == nullptr) {
                bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0;
                return;
            }
            bounds[0] = shape->bounds[0];
            bounds[1] = shape->bounds[1];
            bounds[2] = shape->bounds[2];
            bounds[3] = shape->bounds[3];
            for (shape = shape->next; shape != nullptr; shape = shape->next) {
                bounds[0] = nsvg_minf(bounds[0], shape->bounds[0]);
                bounds[1] = nsvg_minf(bounds[1], shape->bounds[1]);
                bounds[2] = nsvg_maxf(bounds[2], shape->bounds[2]);
                bounds[3] = nsvg_maxf(bounds[3], shape->bounds[3]);
            }
        }

        float nsvg_view_align(const float content, const float container, const int type)
        {
            if (type == Min)
                return 0;
            if (type == Max)
                return container - content;
            // mid
            return (container - content) * 0.5f;
        }

        void nsvg_scale_gradient(NSVGgradient* grad, const float tx, const float ty, const float sx,
                                 const float sy)
        {
            float t[6];
            nsvg_xform_set_translation(t, tx, ty);
            nsvg_xform_multiply(grad->xform, t);

            nsvg_xform_set_scale(t, sx, sy);
            nsvg_xform_multiply(grad->xform, t);
        }

        void nsvg_scale_to_viewbox(NSVGparser* p, const char* units)
        {
            float bounds[4], t[6];
            int i;

            // Guess image size if not set completely.
            nsvg_image_bounds(p, bounds);

            if (math::equal(p->view_width, 0.0f)) {
                if (p->image->width > 0)
                    p->view_width = p->image->width;
                else {
                    p->view_minx = bounds[0];
                    p->view_width = bounds[2] - bounds[0];
                }
            }
            if (math::equal(p->view_height, 0.0f)) {
                if (p->image->height > 0)
                    p->view_height = p->image->height;
                else {
                    p->view_miny = bounds[1];
                    p->view_height = bounds[3] - bounds[1];
                }
            }
            if (math::equal(p->image->width, 0.0f))
                p->image->width = p->view_width;
            if (math::equal(p->image->height, 0.0f))
                p->image->height = p->view_height;

            float tx = -p->view_minx;
            float ty = -p->view_miny;
            float sx = p->view_width > 0 ? p->image->width / p->view_width : 0;
            float sy = p->view_height > 0 ? p->image->height / p->view_height : 0;
            // Unit scaling
            const float us = 1.0f / nsvg_convert_to_pixels(
                                        p, nsvg_coord(1.0f, nsvg_parse_units(units)), 0.0f, 1.0f);

            // Fix aspect ratio
            if (p->align_type == Meet) {
                // fit whole image into viewbox
                sx = sy = nsvg_minf(sx, sy);
                tx += nsvg_view_align(p->view_width * sx, p->image->width, p->align_x) / sx;
                ty += nsvg_view_align(p->view_height * sy, p->image->height, p->align_y) / sy;
            }
            else if (p->align_type == Slice) {
                // fill whole viewbox with image
                sx = sy = nsvg_maxf(sx, sy);
                tx += nsvg_view_align(p->view_width * sx, p->image->width, p->align_x) / sx;
                ty += nsvg_view_align(p->view_height * sy, p->image->height, p->align_y) / sy;
            }

            // Transform
            sx *= us;
            sy *= us;
            const float avgs = (sx + sy) / 2.0f;
            for (NSVGshape* shape = p->image->shapes; shape != nullptr; shape = shape->next) {
                shape->bounds[0] = (shape->bounds[0] + tx) * sx;
                shape->bounds[1] = (shape->bounds[1] + ty) * sy;
                shape->bounds[2] = (shape->bounds[2] + tx) * sx;
                shape->bounds[3] = (shape->bounds[3] + ty) * sy;
                for (NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
                    path->bounds[0] = (path->bounds[0] + tx) * sx;
                    path->bounds[1] = (path->bounds[1] + ty) * sy;
                    path->bounds[2] = (path->bounds[2] + tx) * sx;
                    path->bounds[3] = (path->bounds[3] + ty) * sy;
                    for (i = 0; i < path->npts; i++) {
                        float* pt = &path->pts[i * 2];
                        pt[0] = (pt[0] + tx) * sx;
                        pt[1] = (pt[1] + ty) * sy;
                    }
                }

                if (shape->fill.type == NSVGPaintLinearGradient ||
                    shape->fill.type == NSVGPaintRadialGradient) {
                    nsvg_scale_gradient(shape->fill.gradient, tx, ty, sx, sy);
                    std::memcpy(t, shape->fill.gradient->xform, sizeof(float) * 6);
                    nsvg_xform_inverse(shape->fill.gradient->xform, t);
                }
                if (shape->stroke.type == NSVGPaintLinearGradient ||
                    shape->stroke.type == NSVGPaintRadialGradient) {
                    nsvg_scale_gradient(shape->stroke.gradient, tx, ty, sx, sy);
                    std::memcpy(t, shape->stroke.gradient->xform, sizeof(float) * 6);
                    nsvg_xform_inverse(shape->stroke.gradient->xform, t);
                }

                shape->stroke_width *= avgs;
                shape->stroke_dash_offset *= avgs;
                for (i = 0; i < shape->stroke_dash_count; i++)
                    shape->stroke_dash_array[i] *= avgs;
            }
        }

        void nsvg_create_gradients(NSVGparser* p)
        {
            for (NSVGshape* shape = p->image->shapes; shape != nullptr; shape = shape->next) {
                if (shape->fill.type == NSVGPaintUndef) {
                    if (shape->fill_gradient[0] != '\0') {
                        float inv[6], localBounds[4];
                        nsvg_xform_inverse(inv, shape->xform);
                        nsvg_get_local_bounds(localBounds, shape, inv);
                        shape->fill.gradient = nsvg_create_gradient(
                            p, shape->fill_gradient, localBounds, shape->xform, &shape->fill.type);
                    }
                    if (shape->fill.type == NSVGPaintUndef)
                        shape->fill.type = NSVGPaintNone;
                }
                if (shape->stroke.type == NSVGPaintUndef) {
                    if (shape->stroke_gradient[0] != '\0') {
                        float inv[6], localBounds[4];
                        nsvg_xform_inverse(inv, shape->xform);
                        nsvg_get_local_bounds(localBounds, shape, inv);
                        shape->stroke.gradient = nsvg_create_gradient(p, shape->stroke_gradient,
                                                                      localBounds, shape->xform,
                                                                      &shape->stroke.type);
                    }
                    if (shape->stroke.type == NSVGPaintUndef)
                        shape->stroke.type = NSVGPaintNone;
                }
            }
        }
    }

    NSVGimage* nsvg_parse(char* input, const char* units, const float dpi)
    {
        NSVGimage* ret = nullptr;

        NSVGparser* p = nsvg_create_parser();
        if (p == nullptr)
            return nullptr;
        p->dpi = dpi;

        nsvg_parse_xml(input, nsvg_start_element, nsvg_end_element, nsvg_content, p);

        // Create gradients after all definitions have been parsed
        nsvg_create_gradients(p);

        // Scale to viewBox
        nsvg_scale_to_viewbox(p, units);

        ret = p->image;
        p->image = nullptr;

        nsvg_delete_parser(p);

        return ret;
    }

    NSVGimage* nsvg_parse_from_file(const char* filename, const char* units, const float dpi)
    {
        FILE* fp = nullptr;
        char* data = nullptr;
        NSVGimage* image = nullptr;

        fp = std::fopen(filename, "rb");
        if (fp != nullptr) {
            std::fseek(fp, 0, SEEK_END);
            const size_t size = ftell(fp);
            std::fseek(fp, 0, SEEK_SET);
            data = static_cast<char*>(malloc(size + 1));
            if (data != nullptr)
                if (fread(data, 1, size, fp) == size) {
                    data[size] = '\0';  // Must be null terminated.
                    std::fclose(fp);
                    image = nsvg_parse(data, units, dpi);
                    std::free(data);

                    return image;
                }
        }

        // error:
        if (fp != nullptr)
            std::fclose(fp);
        if (data != nullptr)
            std::free(data);
        if (image != nullptr)
            nsvg_delete(image);

        return nullptr;
    }

    NSVGpath* nsvg_duplicate_path(const NSVGpath* p)
    {
        NSVGpath* res = nullptr;

        if (p == nullptr)
            return nullptr;

        res = static_cast<NSVGpath*>(malloc(sizeof(NSVGpath)));
        if (res != nullptr) {
            std::memset(res, 0, sizeof(NSVGpath));
            res->pts = static_cast<float*>(
                std::malloc(static_cast<std::size_t>(p->npts) * 2 * sizeof(float)));
            if (res->pts != nullptr) {
                std::memcpy(res->pts, p->pts, static_cast<std::size_t>(p->npts) * sizeof(float) * 2);
                res->npts = p->npts;

                std::memcpy(res->bounds, p->bounds, sizeof(p->bounds));

                res->closed = p->closed;

                return res;
            }
        }

        // error:
        if (res != nullptr) {
            std::free(res->pts);
            std::free(res);
        }
        return nullptr;
    }

    void nsvg_delete(NSVGimage* image)
    {
        if (image == nullptr)
            return;

        NSVGshape* shape = image->shapes;
        while (shape != nullptr) {
            NSVGshape* snext = shape->next;
            nsvg_delete_paths(shape->paths);
            nsvg_delete_paint(&shape->fill);
            nsvg_delete_paint(&shape->stroke);
            std::free(shape);
            shape = snext;
        }
        std::free(image);
    }
}