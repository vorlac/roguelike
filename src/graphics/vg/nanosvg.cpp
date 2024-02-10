#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graphics/vg/nanosvg.hpp"

#define NSVG_PI (3.14159265358979323846264338327f)
#define NSVG_KAPPA90 \
    (0.5522847493f)  // Length proportional to radius of a cubic bezier handle for 90deg arcs.

#define NSVG_ALIGN_MIN   0
#define NSVG_ALIGN_MID   1
#define NSVG_ALIGN_MAX   2
#define NSVG_ALIGN_NONE  0
#define NSVG_ALIGN_MEET  1
#define NSVG_ALIGN_SLICE 2

#define NSVG_NOTUSED(v)                    \
    do                                     \
    {                                      \
        (void)(1 ? (void)0 : ((void)(v))); \
    }                                      \
    while (0)

#define NSVG_RGB(r, g, b) (((unsigned int)r) | ((unsigned int)g << 8) | ((unsigned int)b << 16))

#ifdef _MSC_VER
  #pragma warning(disable : 4996)  // Switch off security warnings
  #pragma warning(disable : 4100)  // Switch off unreferenced formal parameter warnings
  #pragma warning(disable : 4456)
  #pragma warning(disable : 4244)
// #pragma warning(disable : 2220)
#endif

namespace rl::nsvg {
#define NSVG_XML_TAG         1
#define NSVG_XML_CONTENT     2
#define NSVG_XML_MAX_ATTRIBS 256
#define NSVG_MAX_ATTR        128

    enum NSVGgradientUnits {
        NSVG_USER_SPACE = 0,
        NSVG_OBJECT_SPACE = 1
    };

#define NSVG_MAX_DASHES 8

    enum NSVGunits {
        NSVG_UNITS_USER,
        NSVG_UNITS_PX,
        NSVG_UNITS_PT,
        NSVG_UNITS_PC,
        NSVG_UNITS_MM,
        NSVG_UNITS_CM,
        NSVG_UNITS_IN,
        NSVG_UNITS_PERCENT,
        NSVG_UNITS_EM,
        NSVG_UNITS_EX
    };

    namespace {
        int nsvg_isspace(char c)
        {
            return strchr(" \t\n\v\f\r", c) != nullptr;
        }

        int nsvg_isdigit(char c)
        {
            return c >= '0' && c <= '9';
        }

        float nsvg_minf(float a, float b)
        {
            return a < b ? a : b;
        }

        float nsvg_maxf(float a, float b)
        {
            return a > b ? a : b;
        }

        // Simple XML parser

        void nsvg_parseContent(char* s, void (*contentCb)(void* ud, const char* s), void* ud)
        {
            // Trim start white spaces
            while (*s && nsvg_isspace(*s))
                s++;
            if (!*s)
                return;

            if (contentCb)
                (*contentCb)(ud, s);
        }

        void nsvg_parseElement(char* s,
                               void (*startelCb)(void* ud, const char* el, const char** attr),
                               void (*endelCb)(void* ud, const char* el), void* ud)
        {
            const char* attr[NSVG_XML_MAX_ATTRIBS];
            int nattr = 0;
            char* name;
            int start = 0;
            int end = 0;
            char quote;

            // Skip white space after the '<'
            while (*s && nsvg_isspace(*s))
                s++;

            // Check if the tag is end tag
            if (*s == '/')
            {
                s++;
                end = 1;
            }
            else
            {
                start = 1;
            }

            // Skip comments, data and preprocessor stuff.
            if (!*s || *s == '?' || *s == '!')
                return;

            // Get tag name
            name = s;
            while (*s && !nsvg_isspace(*s))
                s++;
            if (*s)
                *s++ = '\0';

            // Get attribs
            while (!end && *s && nattr < NSVG_XML_MAX_ATTRIBS - 3)
            {
                const char* name = nullptr;
                const char* value = nullptr;

                // Skip white space before the attrib name
                while (*s && nsvg_isspace(*s))
                    s++;
                if (!*s)
                    break;
                if (*s == '/')
                {
                    end = 1;
                    break;
                }
                name = s;
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
                quote = *s;
                s++;
                // Store value and find the end of it.
                value = s;
                while (*s && *s != quote)
                    s++;
                if (*s)
                    *s++ = '\0';

                // Store only well formed attributes
                if (name && value)
                {
                    attr[nattr++] = name;
                    attr[nattr++] = value;
                }
            }

            // List terminator
            attr[nattr++] = nullptr;
            attr[nattr++] = nullptr;

            // Call callbacks.
            if (start && startelCb)
                (*startelCb)(ud, name, attr);
            if (end && endelCb)
                (*endelCb)(ud, name);
        }
    }

    int nsvg_parse_xml(char* input, void (*startelCb)(void* ud, const char* el, const char** attr),
                       void (*endelCb)(void* ud, const char* el),
                       void (*contentCb)(void* ud, const char* s), void* ud)
    {
        char* s = input;
        char* mark = s;
        int state = NSVG_XML_CONTENT;
        while (*s)
        {
            if (*s == '<' && state == NSVG_XML_CONTENT)
            {
                // Start of a tag
                *s++ = '\0';
                nsvg_parseContent(mark, contentCb, ud);
                mark = s;
                state = NSVG_XML_TAG;
            }
            else if (*s == '>' && state == NSVG_XML_TAG)
            {
                // Start of a content or new tag.
                *s++ = '\0';
                nsvg_parseElement(mark, startelCb, endelCb, ud);
                mark = s;
                state = NSVG_XML_CONTENT;
            }
            else
            {
                s++;
            }
        }

        return 1;
    }

    /* Simple SVG parser. */

    typedef struct NSVGcoordinate
    {
        float value;
        int units;
    } NSVGcoordinate;

    typedef struct NSVGlinearData
    {
        NSVGcoordinate x1, y1, x2, y2;
    } NSVGlinearData;

    typedef struct NSVGradialData
    {
        NSVGcoordinate cx, cy, r, fx, fy;
    } NSVGradialData;

    typedef struct NSVGgradientData
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
        struct NSVGgradientData* next;
    } NSVGgradientData;

    typedef struct NSVGattrib
    {
        char id[64];
        float xform[6];
        unsigned int fillColor;
        unsigned int strokeColor;
        float opacity;
        float fillOpacity;
        float strokeOpacity;
        char fillGradient[64];
        char strokeGradient[64];
        float strokeWidth;
        float strokeDashOffset;
        float strokeDashArray[NSVG_MAX_DASHES];
        int strokeDashCount;
        char strokeLineJoin;
        char strokeLineCap;
        float miterLimit;
        char fillRule;
        float fontSize;
        unsigned int stopColor;
        float stopOpacity;
        float stopOffset;
        char hasFill;
        char hasStroke;
        char visible;
    } NSVGattrib;

    typedef struct NSVGparser
    {
        NSVGattrib attr[NSVG_MAX_ATTR];
        int attrHead;
        float* pts;
        int npts;
        int cpts;
        NSVGpath* plist;
        NSVGimage* image;
        NSVGgradientData* gradients;
        NSVGshape* shapesTail;
        float viewMinx, viewMiny, viewWidth, viewHeight;
        int alignX, alignY, alignType;
        float dpi;
        char pathFlag;
        char defsFlag;
    } NSVGparser;

    static void nsvg_xformIdentity(float* t)
    {
        t[0] = 1.0f;
        t[1] = 0.0f;
        t[2] = 0.0f;
        t[3] = 1.0f;
        t[4] = 0.0f;
        t[5] = 0.0f;
    }

    static void nsvg_xformSetTranslation(float* t, float tx, float ty)
    {
        t[0] = 1.0f;
        t[1] = 0.0f;
        t[2] = 0.0f;
        t[3] = 1.0f;
        t[4] = tx;
        t[5] = ty;
    }

    static void nsvg_xformSetScale(float* t, float sx, float sy)
    {
        t[0] = sx;
        t[1] = 0.0f;
        t[2] = 0.0f;
        t[3] = sy;
        t[4] = 0.0f;
        t[5] = 0.0f;
    }

    static void nsvg_xformSetSkewX(float* t, float a)
    {
        t[0] = 1.0f;
        t[1] = 0.0f;
        t[2] = tanf(a);
        t[3] = 1.0f;
        t[4] = 0.0f;
        t[5] = 0.0f;
    }

    static void nsvg_xformSetSkewY(float* t, float a)
    {
        t[0] = 1.0f;
        t[1] = tanf(a);
        t[2] = 0.0f;
        t[3] = 1.0f;
        t[4] = 0.0f;
        t[5] = 0.0f;
    }

    static void nsvg_xformSetRotation(float* t, float a)
    {
        float cs = cosf(a), sn = sinf(a);
        t[0] = cs;
        t[1] = sn;
        t[2] = -sn;
        t[3] = cs;
        t[4] = 0.0f;
        t[5] = 0.0f;
    }

    static void nsvg_xformMultiply(float* t, float* s)
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

    static void nsvg_xformInverse(float* inv, float* t)
    {
        double invdet, det = (double)t[0] * t[3] - (double)t[2] * t[1];
        if (det > -1e-6 && det < 1e-6)
        {
            nsvg_xformIdentity(t);
            return;
        }
        invdet = 1.0 / det;
        inv[0] = (float)(t[3] * invdet);
        inv[2] = (float)(-t[2] * invdet);
        inv[4] = (float)(((double)t[2] * t[5] - (double)t[3] * t[4]) * invdet);
        inv[1] = (float)(-t[1] * invdet);
        inv[3] = (float)(t[0] * invdet);
        inv[5] = (float)(((double)t[1] * t[4] - (double)t[0] * t[5]) * invdet);
    }

    static void nsvg_xformPremultiply(float* t, float* s)
    {
        float s2[6];
        memcpy(s2, s, sizeof(float) * 6);
        nsvg_xformMultiply(s2, t);
        memcpy(t, s2, sizeof(float) * 6);
    }

    static void nsvg_xformPoint(float* dx, float* dy, float x, float y, float* t)
    {
        *dx = x * t[0] + y * t[2] + t[4];
        *dy = x * t[1] + y * t[3] + t[5];
    }

    static void nsvg_xformVec(float* dx, float* dy, float x, float y, float* t)
    {
        *dx = x * t[0] + y * t[2];
        *dy = x * t[1] + y * t[3];
    }

#define NSVG_EPSILON (1e-12)

    static int nsvg_ptInBounds(float* pt, float* bounds)
    {
        return pt[0] >= bounds[0] && pt[0] <= bounds[2] && pt[1] >= bounds[1] && pt[1] <= bounds[3];
    }

    static double nsvg_evalBezier(double t, double p0, double p1, double p2, double p3)
    {
        const double it = 1.0 - t;
        return it * it * it * p0 + 3.0 * it * it * t * p1 + 3.0 * it * t * t * p2 + t * t * t * p3;
    }

    static void nsvg_curveBounds(float* bounds, float* curve)
    {
        int i, j, count;
        double roots[2], a, b, c, b2ac, t, v;
        const float* v0 = &curve[0];
        float* v1 = &curve[2];
        float* v2 = &curve[4];
        const float* v3 = &curve[6];

        // Start the bounding box by end points
        bounds[0] = nsvg_minf(v0[0], v3[0]);
        bounds[1] = nsvg_minf(v0[1], v3[1]);
        bounds[2] = nsvg_maxf(v0[0], v3[0]);
        bounds[3] = nsvg_maxf(v0[1], v3[1]);

        // Bezier curve fits inside the convex hull of it's control points.
        // If control points are inside the bounds, we're done.
        if (nsvg_ptInBounds(v1, bounds) && nsvg_ptInBounds(v2, bounds))
            return;

        // Add bezier curve inflection points in X and Y.
        for (i = 0; i < 2; i++)
        {
            a = -3.0 * v0[i] + 9.0 * v1[i] - 9.0 * v2[i] + 3.0 * v3[i];
            b = 6.0 * v0[i] - 12.0 * v1[i] + 6.0 * v2[i];
            c = 3.0 * v1[i] - 3.0 * v0[i];
            count = 0;
            if (fabs(a) < NSVG_EPSILON)
            {
                if (fabs(b) > NSVG_EPSILON)
                {
                    t = -c / b;
                    if (t > NSVG_EPSILON && t < 1.0 - NSVG_EPSILON)
                        roots[count++] = t;
                }
            }
            else
            {
                b2ac = b * b - 4.0 * c * a;
                if (b2ac > NSVG_EPSILON)
                {
                    t = (-b + sqrt(b2ac)) / (2.0 * a);
                    if (t > NSVG_EPSILON && t < 1.0 - NSVG_EPSILON)
                        roots[count++] = t;
                    t = (-b - sqrt(b2ac)) / (2.0 * a);
                    if (t > NSVG_EPSILON && t < 1.0 - NSVG_EPSILON)
                        roots[count++] = t;
                }
            }
            for (j = 0; j < count; j++)
            {
                v = nsvg_evalBezier(roots[j], v0[i], v1[i], v2[i], v3[i]);
                bounds[0 + i] = nsvg_minf(bounds[0 + i], (float)v);
                bounds[2 + i] = nsvg_maxf(bounds[2 + i], (float)v);
            }
        }
    }

    static NSVGparser* nsvg_createParser(void)
    {
        NSVGparser* p;
        p = (NSVGparser*)malloc(sizeof(NSVGparser));
        if (p == nullptr)
            goto error;
        memset(p, 0, sizeof(NSVGparser));

        p->image = (NSVGimage*)malloc(sizeof(NSVGimage));
        if (p->image == nullptr)
            goto error;
        memset(p->image, 0, sizeof(NSVGimage));

        // Init style
        nsvg_xformIdentity(p->attr[0].xform);
        memset(p->attr[0].id, 0, sizeof p->attr[0].id);
        p->attr[0].fillColor = NSVG_RGB(0, 0, 0);
        p->attr[0].strokeColor = NSVG_RGB(0, 0, 0);
        p->attr[0].opacity = 1;
        p->attr[0].fillOpacity = 1;
        p->attr[0].strokeOpacity = 1;
        p->attr[0].stopOpacity = 1;
        p->attr[0].strokeWidth = 1;
        p->attr[0].strokeLineJoin = NSVGJoinMiter;
        p->attr[0].strokeLineCap = NSVGCapButt;
        p->attr[0].miterLimit = 4;
        p->attr[0].fillRule = NSVGFillruleNonzero;
        p->attr[0].hasFill = 1;
        p->attr[0].visible = 1;

        return p;

    error:
        if (p)
        {
            if (p->image)
                free(p->image);
            free(p);
        }
        return nullptr;
    }

    static void nsvg_deletePaths(NSVGpath* path)
    {
        while (path)
        {
            NSVGpath* next = path->next;
            if (path->pts != nullptr)
                free(path->pts);
            free(path);
            path = next;
        }
    }

    static void nsvg_deletePaint(NSVGpaint* paint)
    {
        if (paint->type == NSVGPaintLinearGradient || paint->type == NSVGPaintRadialGradient)
            free(paint->gradient);
    }

    static void nsvg_deleteGradientData(NSVGgradientData* grad)
    {
        NSVGgradientData* next;
        while (grad != nullptr)
        {
            next = grad->next;
            free(grad->stops);
            free(grad);
            grad = next;
        }
    }

    static void nsvg_deleteParser(NSVGparser* p)
    {
        if (p != nullptr)
        {
            nsvg_deletePaths(p->plist);
            nsvg_deleteGradientData(p->gradients);
            nsvg_delete(p->image);
            free(p->pts);
            free(p);
        }
    }

    static void nsvg_resetPath(NSVGparser* p)
    {
        p->npts = 0;
    }

    static void nsvg_addPoint(NSVGparser* p, float x, float y)
    {
        if (p->npts + 1 > p->cpts)
        {
            p->cpts = p->cpts ? p->cpts * 2 : 8;
            p->pts = (float*)realloc(p->pts, p->cpts * 2 * sizeof(float));
            if (!p->pts)
                return;
        }
        p->pts[p->npts * 2 + 0] = x;
        p->pts[p->npts * 2 + 1] = y;
        p->npts++;
    }

    static void nsvg_moveTo(NSVGparser* p, float x, float y)
    {
        if (p->npts > 0)
        {
            p->pts[(p->npts - 1) * 2 + 0] = x;
            p->pts[(p->npts - 1) * 2 + 1] = y;
        }
        else
        {
            nsvg_addPoint(p, x, y);
        }
    }

    static void nsvg_lineTo(NSVGparser* p, float x, float y)
    {
        float px, py, dx, dy;
        if (p->npts > 0)
        {
            px = p->pts[(p->npts - 1) * 2 + 0];
            py = p->pts[(p->npts - 1) * 2 + 1];
            dx = x - px;
            dy = y - py;
            nsvg_addPoint(p, px + dx / 3.0f, py + dy / 3.0f);
            nsvg_addPoint(p, x - dx / 3.0f, y - dy / 3.0f);
            nsvg_addPoint(p, x, y);
        }
    }

    static void nsvg_cubicBezTo(NSVGparser* p, float cpx1, float cpy1, float cpx2, float cpy2,
                                float x, float y)
    {
        if (p->npts > 0)
        {
            nsvg_addPoint(p, cpx1, cpy1);
            nsvg_addPoint(p, cpx2, cpy2);
            nsvg_addPoint(p, x, y);
        }
    }

    static NSVGattrib* nsvg_getAttr(NSVGparser* p)
    {
        return &p->attr[p->attrHead];
    }

    static void nsvg_pushAttr(NSVGparser* p)
    {
        if (p->attrHead < NSVG_MAX_ATTR - 1)
        {
            p->attrHead++;
            memcpy(&p->attr[p->attrHead], &p->attr[p->attrHead - 1], sizeof(NSVGattrib));
        }
    }

    static void nsvg_popAttr(NSVGparser* p)
    {
        if (p->attrHead > 0)
            p->attrHead--;
    }

    static float nsvg_actualOrigX(NSVGparser* p)
    {
        return p->viewMinx;
    }

    static float nsvg_actualOrigY(NSVGparser* p)
    {
        return p->viewMiny;
    }

    static float nsvg_actualWidth(NSVGparser* p)
    {
        return p->viewWidth;
    }

    static float nsvg_actualHeight(NSVGparser* p)
    {
        return p->viewHeight;
    }

    static float nsvg_actualLength(NSVGparser* p)
    {
        float w = nsvg_actualWidth(p), h = nsvg_actualHeight(p);
        return sqrtf(w * w + h * h) / sqrtf(2.0f);
    }

    static float nsvg_convertToPixels(NSVGparser* p, NSVGcoordinate c, float orig, float length)
    {
        const NSVGattrib* attr = nsvg_getAttr(p);
        switch (c.units)
        {
            case NSVG_UNITS_USER:
                return c.value;
            case NSVG_UNITS_PX:
                return c.value;
            case NSVG_UNITS_PT:
                return c.value / 72.0f * p->dpi;
            case NSVG_UNITS_PC:
                return c.value / 6.0f * p->dpi;
            case NSVG_UNITS_MM:
                return c.value / 25.4f * p->dpi;
            case NSVG_UNITS_CM:
                return c.value / 2.54f * p->dpi;
            case NSVG_UNITS_IN:
                return c.value * p->dpi;
            case NSVG_UNITS_EM:
                return c.value * attr->fontSize;
            case NSVG_UNITS_EX:
                return c.value * attr->fontSize * 0.52f;  // x-height of Helvetica.
            case NSVG_UNITS_PERCENT:
                return orig + c.value / 100.0f * length;
            default:
                return c.value;
        }
    }

    static NSVGgradientData* nsvg_findGradientData(NSVGparser* p, const char* id)
    {
        NSVGgradientData* grad = p->gradients;
        if (id == nullptr || *id == '\0')
            return nullptr;
        while (grad != nullptr)
        {
            if (strcmp(grad->id, id) == 0)
                return grad;
            grad = grad->next;
        }
        return nullptr;
    }

    static NSVGgradient* nsvg_createGradient(NSVGparser* p, const char* id,
                                             const float* localBounds, float* xform,
                                             signed char* paintType)
    {
        NSVGgradientData* data = nullptr;
        const NSVGgradientData* ref = nullptr;
        const NSVGgradientStop* stops = nullptr;
        NSVGgradient* grad;
        float ox, oy, sw, sh, sl;
        int nstops = 0;
        int refIter;

        data = nsvg_findGradientData(p, id);
        if (data == nullptr)
            return nullptr;

        // TODO: use ref to fill in all unset values too.
        ref = data;
        refIter = 0;
        while (ref != nullptr)
        {
            NSVGgradientData* nextRef = nullptr;
            if (stops == nullptr && ref->stops != nullptr)
            {
                stops = ref->stops;
                nstops = ref->nstops;
                break;
            }
            nextRef = nsvg_findGradientData(p, ref->ref);
            if (nextRef == ref)
                break;  // prevent infite loops on malformed data
            ref = nextRef;
            refIter++;
            if (refIter > 32)
                break;  // prevent infite loops on malformed data
        }
        if (stops == nullptr)
            return nullptr;

        grad = (NSVGgradient*)malloc(sizeof(NSVGgradient) + sizeof(NSVGgradientStop) * (nstops - 1));
        if (grad == nullptr)
            return nullptr;

        // The shape width and height.
        if (data->units == NSVG_OBJECT_SPACE)
        {
            ox = localBounds[0];
            oy = localBounds[1];
            sw = localBounds[2] - localBounds[0];
            sh = localBounds[3] - localBounds[1];
        }
        else
        {
            ox = nsvg_actualOrigX(p);
            oy = nsvg_actualOrigY(p);
            sw = nsvg_actualWidth(p);
            sh = nsvg_actualHeight(p);
        }
        sl = sqrtf(sw * sw + sh * sh) / sqrtf(2.0f);

        if (data->type == NSVGPaintLinearGradient)
        {
            float x1, y1, x2, y2, dx, dy;
            x1 = nsvg_convertToPixels(p, data->linear.x1, ox, sw);
            y1 = nsvg_convertToPixels(p, data->linear.y1, oy, sh);
            x2 = nsvg_convertToPixels(p, data->linear.x2, ox, sw);
            y2 = nsvg_convertToPixels(p, data->linear.y2, oy, sh);
            // Calculate transform aligned to the line
            dx = x2 - x1;
            dy = y2 - y1;
            grad->xform[0] = dy;
            grad->xform[1] = -dx;
            grad->xform[2] = dx;
            grad->xform[3] = dy;
            grad->xform[4] = x1;
            grad->xform[5] = y1;
        }
        else
        {
            float cx, cy, fx, fy, r;
            cx = nsvg_convertToPixels(p, data->radial.cx, ox, sw);
            cy = nsvg_convertToPixels(p, data->radial.cy, oy, sh);
            fx = nsvg_convertToPixels(p, data->radial.fx, ox, sw);
            fy = nsvg_convertToPixels(p, data->radial.fy, oy, sh);
            r = nsvg_convertToPixels(p, data->radial.r, 0, sl);
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

        nsvg_xformMultiply(grad->xform, data->xform);
        nsvg_xformMultiply(grad->xform, xform);

        grad->spread = data->spread;
        memcpy(grad->stops, stops, nstops * sizeof(NSVGgradientStop));
        grad->nstops = nstops;

        *paintType = data->type;

        return grad;
    }

    static float nsvg_getAverageScale(float* t)
    {
        const float sx = sqrtf(t[0] * t[0] + t[2] * t[2]);
        const float sy = sqrtf(t[1] * t[1] + t[3] * t[3]);
        return (sx + sy) * 0.5f;
    }

    static void nsvg_getLocalBounds(float* bounds, NSVGshape* shape, float* xform)
    {
        NSVGpath* path;
        float curve[4 * 2], curveBounds[4];
        int i, first = 1;
        for (path = shape->paths; path != nullptr; path = path->next)
        {
            nsvg_xformPoint(&curve[0], &curve[1], path->pts[0], path->pts[1], xform);
            for (i = 0; i < path->npts - 1; i += 3)
            {
                nsvg_xformPoint(&curve[2], &curve[3], path->pts[(i + 1) * 2],
                                path->pts[(i + 1) * 2 + 1], xform);
                nsvg_xformPoint(&curve[4], &curve[5], path->pts[(i + 2) * 2],
                                path->pts[(i + 2) * 2 + 1], xform);
                nsvg_xformPoint(&curve[6], &curve[7], path->pts[(i + 3) * 2],
                                path->pts[(i + 3) * 2 + 1], xform);
                nsvg_curveBounds(curveBounds, curve);
                if (first)
                {
                    bounds[0] = curveBounds[0];
                    bounds[1] = curveBounds[1];
                    bounds[2] = curveBounds[2];
                    bounds[3] = curveBounds[3];
                    first = 0;
                }
                else
                {
                    bounds[0] = nsvg_minf(bounds[0], curveBounds[0]);
                    bounds[1] = nsvg_minf(bounds[1], curveBounds[1]);
                    bounds[2] = nsvg_maxf(bounds[2], curveBounds[2]);
                    bounds[3] = nsvg_maxf(bounds[3], curveBounds[3]);
                }
                curve[0] = curve[6];
                curve[1] = curve[7];
            }
        }
    }

    static void nsvg_addShape(NSVGparser* p)
    {
        NSVGattrib* attr = nsvg_getAttr(p);
        float scale = 1.0f;
        NSVGshape* shape;
        NSVGpath* path;
        int i;

        if (p->plist == nullptr)
            return;

        shape = (NSVGshape*)malloc(sizeof(NSVGshape));
        if (shape == nullptr)
            goto error;
        memset(shape, 0, sizeof(NSVGshape));

        memcpy(shape->id, attr->id, sizeof shape->id);
        memcpy(shape->fill_gradient, attr->fillGradient, sizeof shape->fill_gradient);
        memcpy(shape->stroke_gradient, attr->strokeGradient, sizeof shape->stroke_gradient);
        memcpy(shape->xform, attr->xform, sizeof shape->xform);
        scale = nsvg_getAverageScale(attr->xform);
        shape->stroke_width = attr->strokeWidth * scale;
        shape->stroke_dash_offset = attr->strokeDashOffset * scale;
        shape->stroke_dash_count = (char)attr->strokeDashCount;
        for (i = 0; i < attr->strokeDashCount; i++)
            shape->stroke_dash_array[i] = attr->strokeDashArray[i] * scale;
        shape->stroke_line_join = attr->strokeLineJoin;
        shape->stroke_line_cap = attr->strokeLineCap;
        shape->miter_limit = attr->miterLimit;
        shape->fill_rule = attr->fillRule;
        shape->opacity = attr->opacity;

        shape->paths = p->plist;
        p->plist = nullptr;

        // Calculate shape bounds
        shape->bounds[0] = shape->paths->bounds[0];
        shape->bounds[1] = shape->paths->bounds[1];
        shape->bounds[2] = shape->paths->bounds[2];
        shape->bounds[3] = shape->paths->bounds[3];
        for (path = shape->paths->next; path != nullptr; path = path->next)
        {
            shape->bounds[0] = nsvg_minf(shape->bounds[0], path->bounds[0]);
            shape->bounds[1] = nsvg_minf(shape->bounds[1], path->bounds[1]);
            shape->bounds[2] = nsvg_maxf(shape->bounds[2], path->bounds[2]);
            shape->bounds[3] = nsvg_maxf(shape->bounds[3], path->bounds[3]);
        }

        // Set fill
        if (attr->hasFill == 0)
        {
            shape->fill.type = NSVGPaintNone;
        }
        else if (attr->hasFill == 1)
        {
            shape->fill.type = NSVGPaintColor;
            shape->fill.color = attr->fillColor;
            shape->fill.color |= (unsigned int)(attr->fillOpacity * 255) << 24;
        }
        else if (attr->hasFill == 2)
        {
            shape->fill.type = NSVGPaintUndef;
        }

        // Set stroke
        if (attr->hasStroke == 0)
        {
            shape->stroke.type = NSVGPaintNone;
        }
        else if (attr->hasStroke == 1)
        {
            shape->stroke.type = NSVGPaintColor;
            shape->stroke.color = attr->strokeColor;
            shape->stroke.color |= (unsigned int)(attr->strokeOpacity * 255) << 24;
        }
        else if (attr->hasStroke == 2)
        {
            shape->stroke.type = NSVGPaintUndef;
        }

        // Set flags
        shape->flags = (attr->visible ? NSVGFlagsVisible : 0x00);

        // Add to tail
        if (p->image->shapes == nullptr)
            p->image->shapes = shape;
        else
            p->shapesTail->next = shape;
        p->shapesTail = shape;

        return;

    error:
        if (shape)
            free(shape);
    }

    static void nsvg_addPath(NSVGparser* p, char closed)
    {
        NSVGattrib* attr = nsvg_getAttr(p);
        NSVGpath* path = nullptr;
        float bounds[4];
        float* curve;
        int i;

        if (p->npts < 4)
            return;

        if (closed)
            nsvg_lineTo(p, p->pts[0], p->pts[1]);

        // Expect 1 + N*3 points (N = number of cubic bezier segments).
        if ((p->npts % 3) != 1)
            return;

        path = (NSVGpath*)malloc(sizeof(NSVGpath));
        if (path == nullptr)
            goto error;
        memset(path, 0, sizeof(NSVGpath));

        path->pts = (float*)malloc(p->npts * 2 * sizeof(float));
        if (path->pts == nullptr)
            goto error;
        path->closed = closed;
        path->npts = p->npts;

        // Transform path.
        for (i = 0; i < p->npts; ++i)
            nsvg_xformPoint(&path->pts[i * 2], &path->pts[i * 2 + 1], p->pts[i * 2],
                            p->pts[i * 2 + 1], attr->xform);

        // Find bounds
        for (i = 0; i < path->npts - 1; i += 3)
        {
            curve = &path->pts[i * 2];
            nsvg_curveBounds(bounds, curve);
            if (i == 0)
            {
                path->bounds[0] = bounds[0];
                path->bounds[1] = bounds[1];
                path->bounds[2] = bounds[2];
                path->bounds[3] = bounds[3];
            }
            else
            {
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
        if (path != nullptr)
        {
            if (path->pts != nullptr)
                free(path->pts);
            free(path);
        }
    }

    // We roll our own string to float because the std library one uses locale and messes things up.
    static double nsvg_atof(const char* s)
    {
        const char* cur = (char*)s;
        char* end = nullptr;
        double res = 0.0, sign = 1.0;
        long long intPart = 0, fracPart = 0;
        char hasIntPart = 0, hasFracPart = 0;

        // Parse optional sign
        if (*cur == '+')
        {
            cur++;
        }
        else if (*cur == '-')
        {
            sign = -1;
            cur++;
        }

        // Parse integer part
        if (nsvg_isdigit(*cur))
        {
            // Parse digit sequence
            intPart = strtoll(cur, &end, 10);
            if (cur != end)
            {
                res = (double)intPart;
                hasIntPart = 1;
                cur = end;
            }
        }

        // Parse fractional part.
        if (*cur == '.')
        {
            cur++;  // Skip '.'
            if (nsvg_isdigit(*cur))
            {
                // Parse digit sequence
                fracPart = strtoll(cur, &end, 10);
                if (cur != end)
                {
                    res += (double)fracPart / pow(10.0, (double)(end - cur));
                    hasFracPart = 1;
                    cur = end;
                }
            }
        }

        // A valid number should have integer or fractional part.
        if (!hasIntPart && !hasFracPart)
            return 0.0;

        // Parse optional exponent
        if (*cur == 'e' || *cur == 'E')
        {
            long expPart = 0;
            cur++;                            // skip 'E'
            expPart = strtol(cur, &end, 10);  // Parse digit sequence with sign
            if (cur != end)
                res *= pow(10.0, (double)expPart);
        }

        return res * sign;
    }

    static const char* nsvg_parseNumber(const char* s, char* it, const int size)
    {
        const int last = size - 1;
        int i = 0;

        // sign
        if (*s == '-' || *s == '+')
        {
            if (i < last)
                it[i++] = *s;
            s++;
        }
        // integer part
        while (*s && nsvg_isdigit(*s))
        {
            if (i < last)
                it[i++] = *s;
            s++;
        }
        if (*s == '.')
        {
            // decimal point
            if (i < last)
                it[i++] = *s;
            s++;
            // fraction part
            while (*s && nsvg_isdigit(*s))
            {
                if (i < last)
                    it[i++] = *s;
                s++;
            }
        }
        // exponent
        if ((*s == 'e' || *s == 'E') && (s[1] != 'm' && s[1] != 'x'))
        {
            if (i < last)
                it[i++] = *s;
            s++;
            if (*s == '-' || *s == '+')
            {
                if (i < last)
                    it[i++] = *s;
                s++;
            }
            while (*s && nsvg_isdigit(*s))
            {
                if (i < last)
                    it[i++] = *s;
                s++;
            }
        }
        it[i] = '\0';

        return s;
    }

    static const char* nsvg_getNextPathItemWhenArcFlag(const char* s, char* it)
    {
        it[0] = '\0';
        // Skip white spaces and commas
        while (*s && (nsvg_isspace(*s) || *s == ','))
            s++;
        if (!*s)
            return s;
        if (*s == '-' || *s == '+' || *s == '.' || nsvg_isdigit(*s))
        {
            s = nsvg_parseNumber(s, it, 64);
        }
        else
        {
            // Parse command
            it[0] = *s++;
            it[1] = '\0';
            return s;
        }

        return s;
    }

    static const char* nsvg_getNextPathItem(const char* s, char* it)
    {
        it[0] = '\0';
        // Skip white spaces and commas
        while (*s && (nsvg_isspace(*s) || *s == ','))
            s++;
        if (!*s)
            return s;
        if (*s == '-' || *s == '+' || *s == '.' || nsvg_isdigit(*s))
        {
            s = nsvg_parseNumber(s, it, 64);
        }
        else
        {
            // Parse command
            it[0] = *s++;
            it[1] = '\0';
            return s;
        }

        return s;
    }

    static unsigned int nsvg_parseColorHex(const char* str)
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

    static unsigned int nsvg_parseColorRGB(const char* str)
    {
        int i;
        unsigned int rgbi[3];
        float rgbf[3];
        // try decimal integers first
        if (sscanf(str, "rgb(%u, %u, %u)", &rgbi[0], &rgbi[1], &rgbi[2]) != 3)
        {
            // integers failed, try percent values (float, locale independent)
            const char delimiter[3] = { ',', ',', ')' };
            str += 4;  // skip "rgb("
            for (i = 0; i < 3; i++)
            {
                while (*str && (nsvg_isspace(*str)))
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
                if (*str == '.')
                {
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
            if (i == 3)
            {
                rgbi[0] = roundf(rgbf[0] * 2.55f);
                rgbi[1] = roundf(rgbf[1] * 2.55f);
                rgbi[2] = roundf(rgbf[2] * 2.55f);
            }
            else
            {
                rgbi[0] = rgbi[1] = rgbi[2] = 128;
            }
        }
        // clip values as the CSS spec requires
        for (i = 0; i < 3; i++)
            if (rgbi[i] > 255)
                rgbi[i] = 255;
        return NSVG_RGB(rgbi[0], rgbi[1], rgbi[2]);
    }

    typedef struct NSVGNamedColor
    {
        const char* name;
        unsigned int color;
    } NSVGNamedColor;

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

#ifdef NANOSVG_ALL_COLOR_KEYWORDS
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
#endif
    };

    static unsigned int nsvg_parseColorName(const char* str)
    {
        int i, ncolors = sizeof(nsvg_colors) / sizeof(NSVGNamedColor);

        for (i = 0; i < ncolors; i++)
            if (strcmp(nsvg_colors[i].name, str) == 0)
                return nsvg_colors[i].color;

        return NSVG_RGB(128, 128, 128);
    }

    static unsigned int nsvg_parseColor(const char* str)
    {
        size_t len = 0;
        while (*str == ' ')
            ++str;
        len = strlen(str);
        if (len >= 1 && *str == '#')
            return nsvg_parseColorHex(str);
        else if (len >= 4 && str[0] == 'r' && str[1] == 'g' && str[2] == 'b' && str[3] == '(')
            return nsvg_parseColorRGB(str);
        return nsvg_parseColorName(str);
    }

    static float nsvg_parseOpacity(const char* str)
    {
        float val = nsvg_atof(str);
        if (val < 0.0f)
            val = 0.0f;
        if (val > 1.0f)
            val = 1.0f;
        return val;
    }

    static float nsvg_parseMiterLimit(const char* str)
    {
        float val = nsvg_atof(str);
        if (val < 0.0f)
            val = 0.0f;
        return val;
    }

    static int nsvg_parseUnits(const char* units)
    {
        if (units[0] == 'p' && units[1] == 'x')
            return NSVG_UNITS_PX;
        else if (units[0] == 'p' && units[1] == 't')
            return NSVG_UNITS_PT;
        else if (units[0] == 'p' && units[1] == 'c')
            return NSVG_UNITS_PC;
        else if (units[0] == 'm' && units[1] == 'm')
            return NSVG_UNITS_MM;
        else if (units[0] == 'c' && units[1] == 'm')
            return NSVG_UNITS_CM;
        else if (units[0] == 'i' && units[1] == 'n')
            return NSVG_UNITS_IN;
        else if (units[0] == '%')
            return NSVG_UNITS_PERCENT;
        else if (units[0] == 'e' && units[1] == 'm')
            return NSVG_UNITS_EM;
        else if (units[0] == 'e' && units[1] == 'x')
            return NSVG_UNITS_EX;
        return NSVG_UNITS_USER;
    }

    static int nsvg_isCoordinate(const char* s)
    {
        // optional sign
        if (*s == '-' || *s == '+')
            s++;
        // must have at least one digit, or start by a dot
        return nsvg_isdigit(*s) || *s == '.';
    }

    static NSVGcoordinate nsvg_parseCoordinateRaw(const char* str)
    {
        NSVGcoordinate coord = { 0, NSVG_UNITS_USER };
        char buf[64];
        coord.units = nsvg_parseUnits(nsvg_parseNumber(str, buf, 64));
        coord.value = nsvg_atof(buf);
        return coord;
    }

    static NSVGcoordinate nsvg_coord(float v, int units)
    {
        const NSVGcoordinate coord = { v, units };
        return coord;
    }

    static float nsvg_parseCoordinate(NSVGparser* p, const char* str, float orig, float length)
    {
        const NSVGcoordinate coord = nsvg_parseCoordinateRaw(str);
        return nsvg_convertToPixels(p, coord, orig, length);
    }

    static int nsvg_parseTransformArgs(const char* str, float* args, int maxNa, int* na)
    {
        const char* end;
        const char* ptr;
        char it[64];

        *na = 0;
        ptr = str;
        while (*ptr && *ptr != '(')
            ++ptr;
        if (*ptr == 0)
            return 1;
        end = ptr;
        while (*end && *end != ')')
            ++end;
        if (*end == 0)
            return 1;

        while (ptr < end)
        {
            if (*ptr == '-' || *ptr == '+' || *ptr == '.' || nsvg_isdigit(*ptr))
            {
                if (*na >= maxNa)
                    return 0;
                ptr = nsvg_parseNumber(ptr, it, 64);
                args[(*na)++] = (float)nsvg_atof(it);
            }
            else
            {
                ++ptr;
            }
        }
        return (int)(end - str);
    }

    static int nsvg_parseMatrix(float* xform, const char* str)
    {
        float t[6];
        int na = 0;
        const int len = nsvg_parseTransformArgs(str, t, 6, &na);
        if (na != 6)
            return len;
        memcpy(xform, t, sizeof(float) * 6);
        return len;
    }

    static int nsvg_parseTranslate(float* xform, const char* str)
    {
        float args[2];
        float t[6];
        int na = 0;
        const int len = nsvg_parseTransformArgs(str, args, 2, &na);
        if (na == 1)
            args[1] = 0.0;

        nsvg_xformSetTranslation(t, args[0], args[1]);
        memcpy(xform, t, sizeof(float) * 6);
        return len;
    }

    static int nsvg_parseScale(float* xform, const char* str)
    {
        float args[2];
        int na = 0;
        float t[6];
        const int len = nsvg_parseTransformArgs(str, args, 2, &na);
        if (na == 1)
            args[1] = args[0];
        nsvg_xformSetScale(t, args[0], args[1]);
        memcpy(xform, t, sizeof(float) * 6);
        return len;
    }

    static int nsvg_parseSkewX(float* xform, const char* str)
    {
        float args[1];
        int na = 0;
        float t[6];
        const int len = nsvg_parseTransformArgs(str, args, 1, &na);
        nsvg_xformSetSkewX(t, args[0] / 180.0f * NSVG_PI);
        memcpy(xform, t, sizeof(float) * 6);
        return len;
    }

    static int nsvg_parseSkewY(float* xform, const char* str)
    {
        float args[1];
        int na = 0;
        float t[6];
        const int len = nsvg_parseTransformArgs(str, args, 1, &na);
        nsvg_xformSetSkewY(t, args[0] / 180.0f * NSVG_PI);
        memcpy(xform, t, sizeof(float) * 6);
        return len;
    }

    static int nsvg_parseRotate(float* xform, const char* str)
    {
        float args[3];
        int na = 0;
        float m[6];
        float t[6];
        const int len = nsvg_parseTransformArgs(str, args, 3, &na);
        if (na == 1)
            args[1] = args[2] = 0.0f;
        nsvg_xformIdentity(m);

        if (na > 1)
        {
            nsvg_xformSetTranslation(t, -args[1], -args[2]);
            nsvg_xformMultiply(m, t);
        }

        nsvg_xformSetRotation(t, args[0] / 180.0f * NSVG_PI);
        nsvg_xformMultiply(m, t);

        if (na > 1)
        {
            nsvg_xformSetTranslation(t, args[1], args[2]);
            nsvg_xformMultiply(m, t);
        }

        memcpy(xform, m, sizeof(float) * 6);

        return len;
    }

    static void nsvg_parseTransform(float* xform, const char* str)
    {
        float t[6];
        int len;
        nsvg_xformIdentity(xform);
        while (*str)
        {
            if (strncmp(str, "matrix", 6) == 0)
                len = nsvg_parseMatrix(t, str);
            else if (strncmp(str, "translate", 9) == 0)
                len = nsvg_parseTranslate(t, str);
            else if (strncmp(str, "scale", 5) == 0)
                len = nsvg_parseScale(t, str);
            else if (strncmp(str, "rotate", 6) == 0)
                len = nsvg_parseRotate(t, str);
            else if (strncmp(str, "skewX", 5) == 0)
                len = nsvg_parseSkewX(t, str);
            else if (strncmp(str, "skewY", 5) == 0)
                len = nsvg_parseSkewY(t, str);
            else
            {
                ++str;
                continue;
            }
            if (len != 0)
            {
                str += len;
            }
            else
            {
                ++str;
                continue;
            }

            nsvg_xformPremultiply(xform, t);
        }
    }

    static void nsvg_parseUrl(char* id, const char* str)
    {
        int i = 0;
        str += 4;  // "url(";
        if (*str && *str == '#')
            str++;
        while (i < 63 && *str && *str != ')')
        {
            id[i] = *str++;
            i++;
        }
        id[i] = '\0';
    }

    static char nsvg_parseLineCap(const char* str)
    {
        if (strcmp(str, "butt") == 0)
            return NSVGCapButt;
        else if (strcmp(str, "round") == 0)
            return NSVGCapRound;
        else if (strcmp(str, "square") == 0)
            return NSVGCapSquare;
        // TODO: handle inherit.
        return NSVGCapButt;
    }

    static char nsvg_parseLineJoin(const char* str)
    {
        if (strcmp(str, "miter") == 0)
            return NSVGJoinMiter;
        else if (strcmp(str, "round") == 0)
            return NSVGJoinRound;
        else if (strcmp(str, "bevel") == 0)
            return NSVGJoinBevel;
        // TODO: handle inherit.
        return NSVGJoinMiter;
    }

    static char nsvg_parseFillRule(const char* str)
    {
        if (strcmp(str, "nonzero") == 0)
            return NSVGFillruleNonzero;
        else if (strcmp(str, "evenodd") == 0)
            return NSVGFillruleEvenodd;
        // TODO: handle inherit.
        return NSVGFillruleNonzero;
    }

    static const char* nsvg_getNextDashItem(const char* s, char* it)
    {
        int n = 0;
        it[0] = '\0';
        // Skip white spaces and commas
        while (*s && (nsvg_isspace(*s) || *s == ','))
            s++;
        // Advance until whitespace, comma or end.
        while (*s && (!nsvg_isspace(*s) && *s != ','))
        {
            if (n < 63)
                it[n++] = *s;
            s++;
        }
        it[n++] = '\0';
        return s;
    }

    static int nsvg_parseStrokeDashArray(NSVGparser* p, const char* str, float* strokeDashArray)
    {
        char item[64];
        int count = 0, i;
        float sum = 0.0f;

        // Handle "none"
        if (str[0] == 'n')
            return 0;

        // Parse dashes
        while (*str)
        {
            str = nsvg_getNextDashItem(str, item);
            if (!*item)
                break;
            if (count < NSVG_MAX_DASHES)
                strokeDashArray[count++] = fabsf(
                    nsvg_parseCoordinate(p, item, 0.0f, nsvg_actualLength(p)));
        }

        for (i = 0; i < count; i++)
            sum += strokeDashArray[i];
        if (sum <= 1e-6f)
            count = 0;

        return count;
    }

    static void nsvg_parseStyle(NSVGparser* p, const char* str);

    static int nsvg_parseAttr(NSVGparser* p, const char* name, const char* value)
    {
        float xform[6];
        NSVGattrib* attr = nsvg_getAttr(p);
        if (!attr)
            return 0;

        if (strcmp(name, "style") == 0)
        {
            nsvg_parseStyle(p, value);
        }
        else if (strcmp(name, "display") == 0)
        {
            if (strcmp(value, "none") == 0)
                attr->visible = 0;
            // Don't reset ->visible on display:inline, one display:none hides the whole subtree
        }
        else if (strcmp(name, "fill") == 0)
        {
            if (strcmp(value, "none") == 0)
            {
                attr->hasFill = 0;
            }
            else if (strncmp(value, "url(", 4) == 0)
            {
                attr->hasFill = 2;
                nsvg_parseUrl(attr->fillGradient, value);
            }
            else
            {
                attr->hasFill = 1;
                attr->fillColor = nsvg_parseColor(value);
            }
        }
        else if (strcmp(name, "opacity") == 0)
        {
            attr->opacity = nsvg_parseOpacity(value);
        }
        else if (strcmp(name, "fill-opacity") == 0)
        {
            attr->fillOpacity = nsvg_parseOpacity(value);
        }
        else if (strcmp(name, "stroke") == 0)
        {
            if (strcmp(value, "none") == 0)
            {
                attr->hasStroke = 0;
            }
            else if (strncmp(value, "url(", 4) == 0)
            {
                attr->hasStroke = 2;
                nsvg_parseUrl(attr->strokeGradient, value);
            }
            else
            {
                attr->hasStroke = 1;
                attr->strokeColor = nsvg_parseColor(value);
            }
        }
        else if (strcmp(name, "stroke-width") == 0)
        {
            attr->strokeWidth = nsvg_parseCoordinate(p, value, 0.0f, nsvg_actualLength(p));
        }
        else if (strcmp(name, "stroke-dasharray") == 0)
        {
            attr->strokeDashCount = nsvg_parseStrokeDashArray(p, value, attr->strokeDashArray);
        }
        else if (strcmp(name, "stroke-dashoffset") == 0)
        {
            attr->strokeDashOffset = nsvg_parseCoordinate(p, value, 0.0f, nsvg_actualLength(p));
        }
        else if (strcmp(name, "stroke-opacity") == 0)
        {
            attr->strokeOpacity = nsvg_parseOpacity(value);
        }
        else if (strcmp(name, "stroke-linecap") == 0)
        {
            attr->strokeLineCap = nsvg_parseLineCap(value);
        }
        else if (strcmp(name, "stroke-linejoin") == 0)
        {
            attr->strokeLineJoin = nsvg_parseLineJoin(value);
        }
        else if (strcmp(name, "stroke-miterlimit") == 0)
        {
            attr->miterLimit = nsvg_parseMiterLimit(value);
        }
        else if (strcmp(name, "fill-rule") == 0)
        {
            attr->fillRule = nsvg_parseFillRule(value);
        }
        else if (strcmp(name, "font-size") == 0)
        {
            attr->fontSize = nsvg_parseCoordinate(p, value, 0.0f, nsvg_actualLength(p));
        }
        else if (strcmp(name, "transform") == 0)
        {
            nsvg_parseTransform(xform, value);
            nsvg_xformPremultiply(attr->xform, xform);
        }
        else if (strcmp(name, "stop-color") == 0)
        {
            attr->stopColor = nsvg_parseColor(value);
        }
        else if (strcmp(name, "stop-opacity") == 0)
        {
            attr->stopOpacity = nsvg_parseOpacity(value);
        }
        else if (strcmp(name, "offset") == 0)
        {
            attr->stopOffset = nsvg_parseCoordinate(p, value, 0.0f, 1.0f);
        }
        else if (strcmp(name, "id") == 0)
        {
            strncpy(attr->id, value, 63);
            attr->id[63] = '\0';
        }
        else
        {
            return 0;
        }
        return 1;
    }

    static int nsvg_parseNameValue(NSVGparser* p, const char* start, const char* end)
    {
        const char* str;
        const char* val;
        char name[512];
        char value[512];
        int n;

        str = start;
        while (str < end && *str != ':')
            ++str;

        val = str;

        // Right Trim
        while (str > start && (*str == ':' || nsvg_isspace(*str)))
            --str;
        ++str;

        n = (int)(str - start);
        if (n > 511)
            n = 511;
        if (n)
            memcpy(name, start, n);
        name[n] = 0;

        while (val < end && (*val == ':' || nsvg_isspace(*val)))
            ++val;

        n = (int)(end - val);
        if (n > 511)
            n = 511;
        if (n)
            memcpy(value, val, n);
        value[n] = 0;

        return nsvg_parseAttr(p, name, value);
    }

    static void nsvg_parseStyle(NSVGparser* p, const char* str)
    {
        const char* start;
        const char* end;

        while (*str)
        {
            // Left Trim
            while (*str && nsvg_isspace(*str))
                ++str;
            start = str;
            while (*str && *str != ';')
                ++str;
            end = str;

            // Right Trim
            while (end > start && (*end == ';' || nsvg_isspace(*end)))
                --end;
            ++end;

            nsvg_parseNameValue(p, start, end);
            if (*str)
                ++str;
        }
    }

    static void nsvg_parseAttribs(NSVGparser* p, const char** attr)
    {
        int i;
        for (i = 0; attr[i]; i += 2)
            if (strcmp(attr[i], "style") == 0)
                nsvg_parseStyle(p, attr[i + 1]);
            else
                nsvg_parseAttr(p, attr[i], attr[i + 1]);
    }

    static int nsvg_getArgsPerElement(char cmd)
    {
        switch (cmd)
        {
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

    static void nsvg_pathMoveTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
    {
        if (rel)
        {
            *cpx += args[0];
            *cpy += args[1];
        }
        else
        {
            *cpx = args[0];
            *cpy = args[1];
        }
        nsvg_moveTo(p, *cpx, *cpy);
    }

    static void nsvg_pathLineTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
    {
        if (rel)
        {
            *cpx += args[0];
            *cpy += args[1];
        }
        else
        {
            *cpx = args[0];
            *cpy = args[1];
        }
        nsvg_lineTo(p, *cpx, *cpy);
    }

    static void nsvg_pathHLineTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
    {
        if (rel)
            *cpx += args[0];
        else
            *cpx = args[0];
        nsvg_lineTo(p, *cpx, *cpy);
    }

    static void nsvg_pathVLineTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
    {
        if (rel)
            *cpy += args[0];
        else
            *cpy = args[0];
        nsvg_lineTo(p, *cpx, *cpy);
    }

    static void nsvg_pathCubicBezTo(NSVGparser* p, float* cpx, float* cpy, float* cpx2, float* cpy2,
                                    float* args, int rel)
    {
        float x2, y2, cx1, cy1, cx2, cy2;

        if (rel)
        {
            cx1 = *cpx + args[0];
            cy1 = *cpy + args[1];
            cx2 = *cpx + args[2];
            cy2 = *cpy + args[3];
            x2 = *cpx + args[4];
            y2 = *cpy + args[5];
        }
        else
        {
            cx1 = args[0];
            cy1 = args[1];
            cx2 = args[2];
            cy2 = args[3];
            x2 = args[4];
            y2 = args[5];
        }

        nsvg_cubicBezTo(p, cx1, cy1, cx2, cy2, x2, y2);

        *cpx2 = cx2;
        *cpy2 = cy2;
        *cpx = x2;
        *cpy = y2;
    }

    static void nsvg_pathCubicBezShortTo(NSVGparser* p, float* cpx, float* cpy, float* cpx2,
                                         float* cpy2, float* args, int rel)
    {
        float x1, y1, x2, y2, cx1, cy1, cx2, cy2;

        x1 = *cpx;
        y1 = *cpy;
        if (rel)
        {
            cx2 = *cpx + args[0];
            cy2 = *cpy + args[1];
            x2 = *cpx + args[2];
            y2 = *cpy + args[3];
        }
        else
        {
            cx2 = args[0];
            cy2 = args[1];
            x2 = args[2];
            y2 = args[3];
        }

        cx1 = 2 * x1 - *cpx2;
        cy1 = 2 * y1 - *cpy2;

        nsvg_cubicBezTo(p, cx1, cy1, cx2, cy2, x2, y2);

        *cpx2 = cx2;
        *cpy2 = cy2;
        *cpx = x2;
        *cpy = y2;
    }

    static void nsvg_pathQuadBezTo(NSVGparser* p, float* cpx, float* cpy, float* cpx2, float* cpy2,
                                   float* args, int rel)
    {
        float x1, y1, x2, y2, cx, cy;
        float cx1, cy1, cx2, cy2;

        x1 = *cpx;
        y1 = *cpy;
        if (rel)
        {
            cx = *cpx + args[0];
            cy = *cpy + args[1];
            x2 = *cpx + args[2];
            y2 = *cpy + args[3];
        }
        else
        {
            cx = args[0];
            cy = args[1];
            x2 = args[2];
            y2 = args[3];
        }

        // Convert to cubic bezier
        cx1 = x1 + 2.0f / 3.0f * (cx - x1);
        cy1 = y1 + 2.0f / 3.0f * (cy - y1);
        cx2 = x2 + 2.0f / 3.0f * (cx - x2);
        cy2 = y2 + 2.0f / 3.0f * (cy - y2);

        nsvg_cubicBezTo(p, cx1, cy1, cx2, cy2, x2, y2);

        *cpx2 = cx;
        *cpy2 = cy;
        *cpx = x2;
        *cpy = y2;
    }

    static void nsvg_pathQuadBezShortTo(NSVGparser* p, float* cpx, float* cpy, float* cpx2,
                                        float* cpy2, float* args, int rel)
    {
        float x1, y1, x2, y2, cx, cy;
        float cx1, cy1, cx2, cy2;

        x1 = *cpx;
        y1 = *cpy;
        if (rel)
        {
            x2 = *cpx + args[0];
            y2 = *cpy + args[1];
        }
        else
        {
            x2 = args[0];
            y2 = args[1];
        }

        cx = 2 * x1 - *cpx2;
        cy = 2 * y1 - *cpy2;

        // Convert to cubix bezier
        cx1 = x1 + 2.0f / 3.0f * (cx - x1);
        cy1 = y1 + 2.0f / 3.0f * (cy - y1);
        cx2 = x2 + 2.0f / 3.0f * (cx - x2);
        cy2 = y2 + 2.0f / 3.0f * (cy - y2);

        nsvg_cubicBezTo(p, cx1, cy1, cx2, cy2, x2, y2);

        *cpx2 = cx;
        *cpy2 = cy;
        *cpx = x2;
        *cpy = y2;
    }

    static float nsvg_sqr(float x)
    {
        return x * x;
    }

    static float nsvg_vmag(float x, float y)
    {
        return sqrtf(x * x + y * y);
    }

    static float nsvg_vecrat(float ux, float uy, float vx, float vy)
    {
        return (ux * vx + uy * vy) / (nsvg_vmag(ux, uy) * nsvg_vmag(vx, vy));
    }

    static float nsvg_vecang(float ux, float uy, float vx, float vy)
    {
        float r = nsvg_vecrat(ux, uy, vx, vy);
        if (r < -1.0f)
            r = -1.0f;
        if (r > 1.0f)
            r = 1.0f;
        return ((ux * vy < uy * vx) ? -1.0f : 1.0f) * acosf(r);
    }

    static void nsvg_pathArcTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
    {
        // Ported from canvg (https://code.google.com/p/canvg/)
        float rx, ry, rotx;
        float x1, y1, x2, y2, cx, cy, dx, dy, d;
        float x1p, y1p, cxp, cyp, s, sa, sb;
        float ux, uy, vx, vy, a1, da;
        float x, y, tanx, tany, a, px = 0, py = 0, ptanx = 0, ptany = 0, t[6];
        float sinrx, cosrx;
        int fa, fs;
        int i, ndivs;
        float hda, kappa;

        rx = fabsf(args[0]);                 // y radius
        ry = fabsf(args[1]);                 // x radius
        rotx = args[2] / 180.0f * NSVG_PI;   // x rotation angle
        fa = fabsf(args[3]) > 1e-6 ? 1 : 0;  // Large arc
        fs = fabsf(args[4]) > 1e-6 ? 1 : 0;  // Sweep direction
        x1 = *cpx;                           // start point
        y1 = *cpy;
        if (rel)
        {  // end point
            x2 = *cpx + args[5];
            y2 = *cpy + args[6];
        }
        else
        {
            x2 = args[5];
            y2 = args[6];
        }

        dx = x1 - x2;
        dy = y1 - y2;
        d = sqrtf(dx * dx + dy * dy);
        if (d < 1e-6f || rx < 1e-6f || ry < 1e-6f)
        {
            // The arc degenerates to a line
            nsvg_lineTo(p, x2, y2);
            *cpx = x2;
            *cpy = y2;
            return;
        }

        sinrx = sinf(rotx);
        cosrx = cosf(rotx);

        // Convert to center point parameterization.
        // http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
        // 1) Compute x1', y1'
        x1p = cosrx * dx / 2.0f + sinrx * dy / 2.0f;
        y1p = -sinrx * dx / 2.0f + cosrx * dy / 2.0f;
        d = nsvg_sqr(x1p) / nsvg_sqr(rx) + nsvg_sqr(y1p) / nsvg_sqr(ry);
        if (d > 1)
        {
            d = sqrtf(d);
            rx *= d;
            ry *= d;
        }
        // 2) Compute cx', cy'
        s = 0.0f;
        sa = nsvg_sqr(rx) * nsvg_sqr(ry) - nsvg_sqr(rx) * nsvg_sqr(y1p) -
             nsvg_sqr(ry) * nsvg_sqr(x1p);
        sb = nsvg_sqr(rx) * nsvg_sqr(y1p) + nsvg_sqr(ry) * nsvg_sqr(x1p);
        if (sa < 0.0f)
            sa = 0.0f;
        if (sb > 0.0f)
            s = sqrtf(sa / sb);
        if (fa == fs)
            s = -s;
        cxp = s * rx * y1p / ry;
        cyp = s * -ry * x1p / rx;

        // 3) Compute cx,cy from cx',cy'
        cx = (x1 + x2) / 2.0f + cosrx * cxp - sinrx * cyp;
        cy = (y1 + y2) / 2.0f + sinrx * cxp + cosrx * cyp;

        // 4) Calculate theta1, and delta theta.
        ux = (x1p - cxp) / rx;
        uy = (y1p - cyp) / ry;
        vx = (-x1p - cxp) / rx;
        vy = (-y1p - cyp) / ry;
        a1 = nsvg_vecang(1.0f, 0.0f, ux, uy);  // Initial angle
        da = nsvg_vecang(ux, uy, vx, vy);      // Delta angle

        //	if (vecrat(ux,uy,vx,vy) <= -1.0f) da = NSVG_PI;
        //	if (vecrat(ux,uy,vx,vy) >= 1.0f) da = 0;

        if (fs == 0 && da > 0)
            da -= 2 * NSVG_PI;
        else if (fs == 1 && da < 0)
            da += 2 * NSVG_PI;

        // Approximate the arc using cubic spline segments.
        t[0] = cosrx;
        t[1] = sinrx;
        t[2] = -sinrx;
        t[3] = cosrx;
        t[4] = cx;
        t[5] = cy;

        // Split arc into max 90 degree segments.
        // The loop assumes an iteration per end point (including start and end), this +1.
        ndivs = (int)(fabsf(da) / (NSVG_PI * 0.5f) + 1.0f);
        hda = (da / (float)ndivs) / 2.0f;
        // Fix for ticket #179: division by 0: avoid cotangens around 0 (infinite)
        if ((hda < 1e-3f) && (hda > -1e-3f))
            hda *= 0.5f;
        else
            hda = (1.0f - cosf(hda)) / sinf(hda);
        kappa = fabsf(4.0f / 3.0f * hda);
        if (da < 0.0f)
            kappa = -kappa;

        for (i = 0; i <= ndivs; i++)
        {
            a = a1 + da * ((float)i / (float)ndivs);
            dx = cosf(a);
            dy = sinf(a);
            nsvg_xformPoint(&x, &y, dx * rx, dy * ry, t);                       // position
            nsvg_xformVec(&tanx, &tany, -dy * rx * kappa, dx * ry * kappa, t);  // tangent
            if (i > 0)
                nsvg_cubicBezTo(p, px + ptanx, py + ptany, x - tanx, y - tany, x, y);
            px = x;
            py = y;
            ptanx = tanx;
            ptany = tany;
        }

        *cpx = x2;
        *cpy = y2;
    }

    static void nsvg_parsePath(NSVGparser* p, const char** attr)
    {
        const char* s = nullptr;
        char cmd = '\0';
        float args[10];
        int nargs;
        int rargs = 0;
        char initPoint;
        float cpx, cpy, cpx2, cpy2;
        const char* tmp[4];
        char closedFlag;
        int i;
        char item[64];

        for (i = 0; attr[i]; i += 2)
        {
            if (strcmp(attr[i], "d") == 0)
            {
                s = attr[i + 1];
            }
            else
            {
                tmp[0] = attr[i];
                tmp[1] = attr[i + 1];
                tmp[2] = nullptr;
                tmp[3] = nullptr;
                nsvg_parseAttribs(p, tmp);
            }
        }

        if (s)
        {
            nsvg_resetPath(p);
            cpx = 0;
            cpy = 0;
            cpx2 = 0;
            cpy2 = 0;
            initPoint = 0;
            closedFlag = 0;
            nargs = 0;

            while (*s)
            {
                item[0] = '\0';
                if ((cmd == 'A' || cmd == 'a') && (nargs == 3 || nargs == 4))
                    s = nsvg_getNextPathItemWhenArcFlag(s, item);
                if (!*item)
                    s = nsvg_getNextPathItem(s, item);
                if (!*item)
                    break;
                if (cmd != '\0' && nsvg_isCoordinate(item))
                {
                    if (nargs < 10)
                        args[nargs++] = (float)nsvg_atof(item);
                    if (nargs >= rargs)
                    {
                        switch (cmd)
                        {
                            case 'm':
                            case 'M':
                                nsvg_pathMoveTo(p, &cpx, &cpy, args, cmd == 'm' ? 1 : 0);
                                // Moveto can be followed by multiple coordinate pairs,
                                // which should be treated as linetos.
                                cmd = (cmd == 'm') ? 'l' : 'L';
                                rargs = nsvg_getArgsPerElement(cmd);
                                cpx2 = cpx;
                                cpy2 = cpy;
                                initPoint = 1;
                                break;
                            case 'l':
                            case 'L':
                                nsvg_pathLineTo(p, &cpx, &cpy, args, cmd == 'l' ? 1 : 0);
                                cpx2 = cpx;
                                cpy2 = cpy;
                                break;
                            case 'H':
                            case 'h':
                                nsvg_pathHLineTo(p, &cpx, &cpy, args, cmd == 'h' ? 1 : 0);
                                cpx2 = cpx;
                                cpy2 = cpy;
                                break;
                            case 'V':
                            case 'v':
                                nsvg_pathVLineTo(p, &cpx, &cpy, args, cmd == 'v' ? 1 : 0);
                                cpx2 = cpx;
                                cpy2 = cpy;
                                break;
                            case 'C':
                            case 'c':
                                nsvg_pathCubicBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args,
                                                    cmd == 'c' ? 1 : 0);
                                break;
                            case 'S':
                            case 's':
                                nsvg_pathCubicBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args,
                                                         cmd == 's' ? 1 : 0);
                                break;
                            case 'Q':
                            case 'q':
                                nsvg_pathQuadBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args,
                                                   cmd == 'q' ? 1 : 0);
                                break;
                            case 'T':
                            case 't':
                                nsvg_pathQuadBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args,
                                                        cmd == 't' ? 1 : 0);
                                break;
                            case 'A':
                            case 'a':
                                nsvg_pathArcTo(p, &cpx, &cpy, args, cmd == 'a' ? 1 : 0);
                                cpx2 = cpx;
                                cpy2 = cpy;
                                break;
                            default:
                                if (nargs >= 2)
                                {
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
                else
                {
                    cmd = item[0];
                    if (cmd == 'M' || cmd == 'm')
                    {
                        // Commit path.
                        if (p->npts > 0)
                            nsvg_addPath(p, closedFlag);
                        // Start new subpath.
                        nsvg_resetPath(p);
                        closedFlag = 0;
                        nargs = 0;
                    }
                    else if (initPoint == 0)
                    {
                        // Do not allow other commands until initial point has been set (moveTo
                        // called once).
                        cmd = '\0';
                    }
                    if (cmd == 'Z' || cmd == 'z')
                    {
                        closedFlag = 1;
                        // Commit path.
                        if (p->npts > 0)
                        {
                            // Move current point to first point
                            cpx = p->pts[0];
                            cpy = p->pts[1];
                            cpx2 = cpx;
                            cpy2 = cpy;
                            nsvg_addPath(p, closedFlag);
                        }
                        // Start new subpath.
                        nsvg_resetPath(p);
                        nsvg_moveTo(p, cpx, cpy);
                        closedFlag = 0;
                        nargs = 0;
                    }
                    rargs = nsvg_getArgsPerElement(cmd);
                    if (rargs == -1)
                    {
                        // Command not recognized
                        cmd = '\0';
                        rargs = 0;
                    }
                }
            }
            // Commit path.
            if (p->npts)
                nsvg_addPath(p, closedFlag);
        }

        nsvg_addShape(p);
    }

    static void nsvg_parseRect(NSVGparser* p, const char** attr)
    {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
        float rx = -1.0f;  // marks not set
        float ry = -1.0f;
        int i;

        for (i = 0; attr[i]; i += 2)
        {
            if (!nsvg_parseAttr(p, attr[i], attr[i + 1]))
            {
                if (strcmp(attr[i], "x") == 0)
                    x = nsvg_parseCoordinate(p, attr[i + 1], nsvg_actualOrigX(p),
                                             nsvg_actualWidth(p));
                if (strcmp(attr[i], "y") == 0)
                    y = nsvg_parseCoordinate(p, attr[i + 1], nsvg_actualOrigY(p),
                                             nsvg_actualHeight(p));
                if (strcmp(attr[i], "width") == 0)
                    w = nsvg_parseCoordinate(p, attr[i + 1], 0.0f, nsvg_actualWidth(p));
                if (strcmp(attr[i], "height") == 0)
                    h = nsvg_parseCoordinate(p, attr[i + 1], 0.0f, nsvg_actualHeight(p));
                if (strcmp(attr[i], "rx") == 0)
                    rx = fabsf(nsvg_parseCoordinate(p, attr[i + 1], 0.0f, nsvg_actualWidth(p)));
                if (strcmp(attr[i], "ry") == 0)
                    ry = fabsf(nsvg_parseCoordinate(p, attr[i + 1], 0.0f, nsvg_actualHeight(p)));
            }
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

        if (w != 0.0f && h != 0.0f)
        {
            nsvg_resetPath(p);

            if (rx < 0.00001f || ry < 0.0001f)
            {
                nsvg_moveTo(p, x, y);
                nsvg_lineTo(p, x + w, y);
                nsvg_lineTo(p, x + w, y + h);
                nsvg_lineTo(p, x, y + h);
            }
            else
            {
                // Rounded rectangle
                nsvg_moveTo(p, x + rx, y);
                nsvg_lineTo(p, x + w - rx, y);
                nsvg_cubicBezTo(p, x + w - rx * (1 - NSVG_KAPPA90), y, x + w,
                                y + ry * (1 - NSVG_KAPPA90), x + w, y + ry);
                nsvg_lineTo(p, x + w, y + h - ry);
                nsvg_cubicBezTo(p, x + w, y + h - ry * (1 - NSVG_KAPPA90),
                                x + w - rx * (1 - NSVG_KAPPA90), y + h, x + w - rx, y + h);
                nsvg_lineTo(p, x + rx, y + h);
                nsvg_cubicBezTo(p, x + rx * (1 - NSVG_KAPPA90), y + h, x,
                                y + h - ry * (1 - NSVG_KAPPA90), x, y + h - ry);
                nsvg_lineTo(p, x, y + ry);
                nsvg_cubicBezTo(p, x, y + ry * (1 - NSVG_KAPPA90), x + rx * (1 - NSVG_KAPPA90), y,
                                x + rx, y);
            }

            nsvg_addPath(p, 1);

            nsvg_addShape(p);
        }
    }

    static void nsvg_parseCircle(NSVGparser* p, const char** attr)
    {
        float cx = 0.0f;
        float cy = 0.0f;
        float r = 0.0f;
        int i;

        for (i = 0; attr[i]; i += 2)
        {
            if (!nsvg_parseAttr(p, attr[i], attr[i + 1]))
            {
                if (strcmp(attr[i], "cx") == 0)
                    cx = nsvg_parseCoordinate(p, attr[i + 1], nsvg_actualOrigX(p),
                                              nsvg_actualWidth(p));
                if (strcmp(attr[i], "cy") == 0)
                    cy = nsvg_parseCoordinate(p, attr[i + 1], nsvg_actualOrigY(p),
                                              nsvg_actualHeight(p));
                if (strcmp(attr[i], "r") == 0)
                    r = fabsf(nsvg_parseCoordinate(p, attr[i + 1], 0.0f, nsvg_actualLength(p)));
            }
        }

        if (r > 0.0f)
        {
            nsvg_resetPath(p);

            nsvg_moveTo(p, cx + r, cy);
            nsvg_cubicBezTo(p, cx + r, cy + r * NSVG_KAPPA90, cx + r * NSVG_KAPPA90, cy + r, cx,
                            cy + r);
            nsvg_cubicBezTo(p, cx - r * NSVG_KAPPA90, cy + r, cx - r, cy + r * NSVG_KAPPA90, cx - r,
                            cy);
            nsvg_cubicBezTo(p, cx - r, cy - r * NSVG_KAPPA90, cx - r * NSVG_KAPPA90, cy - r, cx,
                            cy - r);
            nsvg_cubicBezTo(p, cx + r * NSVG_KAPPA90, cy - r, cx + r, cy - r * NSVG_KAPPA90, cx + r,
                            cy);

            nsvg_addPath(p, 1);

            nsvg_addShape(p);
        }
    }

    static void nsvg_parseEllipse(NSVGparser* p, const char** attr)
    {
        float cx = 0.0f;
        float cy = 0.0f;
        float rx = 0.0f;
        float ry = 0.0f;
        int i;

        for (i = 0; attr[i]; i += 2)
        {
            if (!nsvg_parseAttr(p, attr[i], attr[i + 1]))
            {
                if (strcmp(attr[i], "cx") == 0)
                    cx = nsvg_parseCoordinate(p, attr[i + 1], nsvg_actualOrigX(p),
                                              nsvg_actualWidth(p));
                if (strcmp(attr[i], "cy") == 0)
                    cy = nsvg_parseCoordinate(p, attr[i + 1], nsvg_actualOrigY(p),
                                              nsvg_actualHeight(p));
                if (strcmp(attr[i], "rx") == 0)
                    rx = fabsf(nsvg_parseCoordinate(p, attr[i + 1], 0.0f, nsvg_actualWidth(p)));
                if (strcmp(attr[i], "ry") == 0)
                    ry = fabsf(nsvg_parseCoordinate(p, attr[i + 1], 0.0f, nsvg_actualHeight(p)));
            }
        }

        if (rx > 0.0f && ry > 0.0f)
        {
            nsvg_resetPath(p);

            nsvg_moveTo(p, cx + rx, cy);
            nsvg_cubicBezTo(p, cx + rx, cy + ry * NSVG_KAPPA90, cx + rx * NSVG_KAPPA90, cy + ry, cx,
                            cy + ry);
            nsvg_cubicBezTo(p, cx - rx * NSVG_KAPPA90, cy + ry, cx - rx, cy + ry * NSVG_KAPPA90,
                            cx - rx, cy);
            nsvg_cubicBezTo(p, cx - rx, cy - ry * NSVG_KAPPA90, cx - rx * NSVG_KAPPA90, cy - ry, cx,
                            cy - ry);
            nsvg_cubicBezTo(p, cx + rx * NSVG_KAPPA90, cy - ry, cx + rx, cy - ry * NSVG_KAPPA90,
                            cx + rx, cy);

            nsvg_addPath(p, 1);

            nsvg_addShape(p);
        }
    }

    static void nsvg_parseLine(NSVGparser* p, const char** attr)
    {
        float x1 = 0.0;
        float y1 = 0.0;
        float x2 = 0.0;
        float y2 = 0.0;
        int i;

        for (i = 0; attr[i]; i += 2)
        {
            if (!nsvg_parseAttr(p, attr[i], attr[i + 1]))
            {
                if (strcmp(attr[i], "x1") == 0)
                    x1 = nsvg_parseCoordinate(p, attr[i + 1], nsvg_actualOrigX(p),
                                              nsvg_actualWidth(p));
                if (strcmp(attr[i], "y1") == 0)
                    y1 = nsvg_parseCoordinate(p, attr[i + 1], nsvg_actualOrigY(p),
                                              nsvg_actualHeight(p));
                if (strcmp(attr[i], "x2") == 0)
                    x2 = nsvg_parseCoordinate(p, attr[i + 1], nsvg_actualOrigX(p),
                                              nsvg_actualWidth(p));
                if (strcmp(attr[i], "y2") == 0)
                    y2 = nsvg_parseCoordinate(p, attr[i + 1], nsvg_actualOrigY(p),
                                              nsvg_actualHeight(p));
            }
        }

        nsvg_resetPath(p);

        nsvg_moveTo(p, x1, y1);
        nsvg_lineTo(p, x2, y2);

        nsvg_addPath(p, 0);

        nsvg_addShape(p);
    }

    static void nsvg_parsePoly(NSVGparser* p, const char** attr, int closeFlag)
    {
        int i;
        const char* s;
        float args[2];
        int nargs, npts = 0;
        char item[64];

        nsvg_resetPath(p);

        for (i = 0; attr[i]; i += 2)
        {
            if (!nsvg_parseAttr(p, attr[i], attr[i + 1]))
            {
                if (strcmp(attr[i], "points") == 0)
                {
                    s = attr[i + 1];
                    nargs = 0;
                    while (*s)
                    {
                        s = nsvg_getNextPathItem(s, item);
                        args[nargs++] = (float)nsvg_atof(item);
                        if (nargs >= 2)
                        {
                            if (npts == 0)
                                nsvg_moveTo(p, args[0], args[1]);
                            else
                                nsvg_lineTo(p, args[0], args[1]);
                            nargs = 0;
                            npts++;
                        }
                    }
                }
            }
        }

        nsvg_addPath(p, (char)closeFlag);

        nsvg_addShape(p);
    }

    static void nsvg_parseSVG(NSVGparser* p, const char** attr)
    {
        int i;
        for (i = 0; attr[i]; i += 2)
        {
            if (!nsvg_parseAttr(p, attr[i], attr[i + 1]))
            {
                if (strcmp(attr[i], "width") == 0)
                {
                    p->image->width = nsvg_parseCoordinate(p, attr[i + 1], 0.0f, 0.0f);
                }
                else if (strcmp(attr[i], "height") == 0)
                {
                    p->image->height = nsvg_parseCoordinate(p, attr[i + 1], 0.0f, 0.0f);
                }
                else if (strcmp(attr[i], "viewBox") == 0)
                {
                    const char* s = attr[i + 1];
                    char buf[64];
                    s = nsvg_parseNumber(s, buf, 64);
                    p->viewMinx = nsvg_atof(buf);
                    while (*s && (nsvg_isspace(*s) || *s == '%' || *s == ','))
                        s++;
                    if (!*s)
                        return;
                    s = nsvg_parseNumber(s, buf, 64);
                    p->viewMiny = nsvg_atof(buf);
                    while (*s && (nsvg_isspace(*s) || *s == '%' || *s == ','))
                        s++;
                    if (!*s)
                        return;
                    s = nsvg_parseNumber(s, buf, 64);
                    p->viewWidth = nsvg_atof(buf);
                    while (*s && (nsvg_isspace(*s) || *s == '%' || *s == ','))
                        s++;
                    if (!*s)
                        return;
                    s = nsvg_parseNumber(s, buf, 64);
                    p->viewHeight = nsvg_atof(buf);
                }
                else if (strcmp(attr[i], "preserveAspectRatio") == 0)
                {
                    if (strstr(attr[i + 1], "none") != nullptr)
                    {
                        // No uniform scaling
                        p->alignType = NSVG_ALIGN_NONE;
                    }
                    else
                    {
                        // Parse X align
                        if (strstr(attr[i + 1], "xMin") != nullptr)
                            p->alignX = NSVG_ALIGN_MIN;
                        else if (strstr(attr[i + 1], "xMid") != nullptr)
                            p->alignX = NSVG_ALIGN_MID;
                        else if (strstr(attr[i + 1], "xMax") != nullptr)
                            p->alignX = NSVG_ALIGN_MAX;
                        // Parse X align
                        if (strstr(attr[i + 1], "yMin") != nullptr)
                            p->alignY = NSVG_ALIGN_MIN;
                        else if (strstr(attr[i + 1], "yMid") != nullptr)
                            p->alignY = NSVG_ALIGN_MID;
                        else if (strstr(attr[i + 1], "yMax") != nullptr)
                            p->alignY = NSVG_ALIGN_MAX;
                        // Parse meet/slice
                        p->alignType = NSVG_ALIGN_MEET;
                        if (strstr(attr[i + 1], "slice") != nullptr)
                            p->alignType = NSVG_ALIGN_SLICE;
                    }
                }
            }
        }
    }

    static void nsvg_parseGradient(NSVGparser* p, const char** attr, signed char type)
    {
        int i;
        NSVGgradientData* grad = (NSVGgradientData*)malloc(sizeof(NSVGgradientData));
        if (grad == nullptr)
            return;
        memset(grad, 0, sizeof(NSVGgradientData));
        grad->units = NSVG_OBJECT_SPACE;
        grad->type = type;
        if (grad->type == NSVGPaintLinearGradient)
        {
            grad->linear.x1 = nsvg_coord(0.0f, NSVG_UNITS_PERCENT);
            grad->linear.y1 = nsvg_coord(0.0f, NSVG_UNITS_PERCENT);
            grad->linear.x2 = nsvg_coord(100.0f, NSVG_UNITS_PERCENT);
            grad->linear.y2 = nsvg_coord(0.0f, NSVG_UNITS_PERCENT);
        }
        else if (grad->type == NSVGPaintRadialGradient)
        {
            grad->radial.cx = nsvg_coord(50.0f, NSVG_UNITS_PERCENT);
            grad->radial.cy = nsvg_coord(50.0f, NSVG_UNITS_PERCENT);
            grad->radial.r = nsvg_coord(50.0f, NSVG_UNITS_PERCENT);
        }

        nsvg_xformIdentity(grad->xform);
        int setfx = 0;
        int setfy = 0;

        for (i = 0; attr[i]; i += 2)
        {
            if (strcmp(attr[i], "id") == 0)
            {
                strncpy(grad->id, attr[i + 1], 63);
                grad->id[63] = '\0';
            }
            else if (!nsvg_parseAttr(p, attr[i], attr[i + 1]))
            {
                if (strcmp(attr[i], "gradientUnits") == 0)
                {
                    if (strcmp(attr[i + 1], "objectBoundingBox") == 0)
                        grad->units = NSVG_OBJECT_SPACE;
                    else
                        grad->units = NSVG_USER_SPACE;
                }
                else if (strcmp(attr[i], "gradientTransform") == 0)
                {
                    nsvg_parseTransform(grad->xform, attr[i + 1]);
                }
                else if (strcmp(attr[i], "cx") == 0)
                {
                    grad->radial.cx = nsvg_parseCoordinateRaw(attr[i + 1]);
                }
                else if (strcmp(attr[i], "cy") == 0)
                {
                    grad->radial.cy = nsvg_parseCoordinateRaw(attr[i + 1]);
                }
                else if (strcmp(attr[i], "r") == 0)
                {
                    grad->radial.r = nsvg_parseCoordinateRaw(attr[i + 1]);
                }
                else if (strcmp(attr[i], "fx") == 0)
                {
                    grad->radial.fx = nsvg_parseCoordinateRaw(attr[i + 1]);
                    setfx = 1;
                }
                else if (strcmp(attr[i], "fy") == 0)
                {
                    grad->radial.fy = nsvg_parseCoordinateRaw(attr[i + 1]);
                    setfy = 1;
                }
                else if (strcmp(attr[i], "x1") == 0)
                {
                    grad->linear.x1 = nsvg_parseCoordinateRaw(attr[i + 1]);
                }
                else if (strcmp(attr[i], "y1") == 0)
                {
                    grad->linear.y1 = nsvg_parseCoordinateRaw(attr[i + 1]);
                }
                else if (strcmp(attr[i], "x2") == 0)
                {
                    grad->linear.x2 = nsvg_parseCoordinateRaw(attr[i + 1]);
                }
                else if (strcmp(attr[i], "y2") == 0)
                {
                    grad->linear.y2 = nsvg_parseCoordinateRaw(attr[i + 1]);
                }
                else if (strcmp(attr[i], "spreadMethod") == 0)
                {
                    if (strcmp(attr[i + 1], "pad") == 0)
                        grad->spread = NSVGSpreadPad;
                    else if (strcmp(attr[i + 1], "reflect") == 0)
                        grad->spread = NSVGSpreadReflect;
                    else if (strcmp(attr[i + 1], "repeat") == 0)
                        grad->spread = NSVGSpreadRepeat;
                }
                else if (strcmp(attr[i], "xlink:href") == 0)
                {
                    const char* href = attr[i + 1];
                    strncpy(grad->ref, href + 1, 62);
                    grad->ref[62] = '\0';
                }
            }
        }

        if (grad->type == NSVGPaintRadialGradient && setfx == 0)
            grad->radial.fx = grad->radial.cx;

        if (grad->type == NSVGPaintRadialGradient && setfy == 0)
            grad->radial.fy = grad->radial.cy;

        grad->next = p->gradients;
        p->gradients = grad;
    }

    static void nsvg_parseGradientStop(NSVGparser* p, const char** attr)
    {
        NSVGattrib* curAttr = nsvg_getAttr(p);
        NSVGgradientData* grad;
        NSVGgradientStop* stop;
        int i, idx;

        curAttr->stopOffset = 0;
        curAttr->stopColor = 0;
        curAttr->stopOpacity = 1.0f;

        for (i = 0; attr[i]; i += 2)
            nsvg_parseAttr(p, attr[i], attr[i + 1]);

        // Add stop to the last gradient.
        grad = p->gradients;
        if (grad == nullptr)
            return;

        grad->nstops++;
        grad->stops = (NSVGgradientStop*)realloc(grad->stops,
                                                 sizeof(NSVGgradientStop) * grad->nstops);
        if (grad->stops == nullptr)
            return;

        // Insert
        idx = grad->nstops - 1;
        for (i = 0; i < grad->nstops - 1; i++)
        {
            if (curAttr->stopOffset < grad->stops[i].offset)
            {
                idx = i;
                break;
            }
        }
        if (idx != grad->nstops - 1)
            for (i = grad->nstops - 1; i > idx; i--)
                grad->stops[i] = grad->stops[i - 1];

        stop = &grad->stops[idx];
        stop->color = curAttr->stopColor;
        stop->color |= (unsigned int)(curAttr->stopOpacity * 255) << 24;
        stop->offset = curAttr->stopOffset;
    }

    static void nsvg_startElement(void* ud, const char* el, const char** attr)
    {
        NSVGparser* p = (NSVGparser*)ud;

        if (p->defsFlag)
        {
            // Skip everything but gradients in defs
            if (strcmp(el, "linearGradient") == 0)
                nsvg_parseGradient(p, attr, NSVGPaintLinearGradient);
            else if (strcmp(el, "radialGradient") == 0)
                nsvg_parseGradient(p, attr, NSVGPaintRadialGradient);
            else if (strcmp(el, "stop") == 0)
                nsvg_parseGradientStop(p, attr);
            return;
        }

        if (strcmp(el, "g") == 0)
        {
            nsvg_pushAttr(p);
            nsvg_parseAttribs(p, attr);
        }
        else if (strcmp(el, "path") == 0)
        {
            if (p->pathFlag)  // Do not allow nested paths.
                return;
            nsvg_pushAttr(p);
            nsvg_parsePath(p, attr);
            nsvg_popAttr(p);
        }
        else if (strcmp(el, "rect") == 0)
        {
            nsvg_pushAttr(p);
            nsvg_parseRect(p, attr);
            nsvg_popAttr(p);
        }
        else if (strcmp(el, "circle") == 0)
        {
            nsvg_pushAttr(p);
            nsvg_parseCircle(p, attr);
            nsvg_popAttr(p);
        }
        else if (strcmp(el, "ellipse") == 0)
        {
            nsvg_pushAttr(p);
            nsvg_parseEllipse(p, attr);
            nsvg_popAttr(p);
        }
        else if (strcmp(el, "line") == 0)
        {
            nsvg_pushAttr(p);
            nsvg_parseLine(p, attr);
            nsvg_popAttr(p);
        }
        else if (strcmp(el, "polyline") == 0)
        {
            nsvg_pushAttr(p);
            nsvg_parsePoly(p, attr, 0);
            nsvg_popAttr(p);
        }
        else if (strcmp(el, "polygon") == 0)
        {
            nsvg_pushAttr(p);
            nsvg_parsePoly(p, attr, 1);
            nsvg_popAttr(p);
        }
        else if (strcmp(el, "linearGradient") == 0)
        {
            nsvg_parseGradient(p, attr, NSVGPaintLinearGradient);
        }
        else if (strcmp(el, "radialGradient") == 0)
        {
            nsvg_parseGradient(p, attr, NSVGPaintRadialGradient);
        }
        else if (strcmp(el, "stop") == 0)
        {
            nsvg_parseGradientStop(p, attr);
        }
        else if (strcmp(el, "defs") == 0)
        {
            p->defsFlag = 1;
        }
        else if (strcmp(el, "svg") == 0)
        {
            nsvg_parseSVG(p, attr);
        }
    }

    static void nsvg_endElement(void* ud, const char* el)
    {
        NSVGparser* p = (NSVGparser*)ud;

        if (strcmp(el, "g") == 0)
            nsvg_popAttr(p);
        else if (strcmp(el, "path") == 0)
            p->pathFlag = 0;
        else if (strcmp(el, "defs") == 0)
            p->defsFlag = 0;
    }

    static void nsvg_content(void* ud, const char* s)
    {
        NSVG_NOTUSED(ud);
        NSVG_NOTUSED(s);
        // empty
    }

    static void nsvg_imageBounds(NSVGparser* p, float* bounds)
    {
        NSVGshape* shape;
        shape = p->image->shapes;
        if (shape == nullptr)
        {
            bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0;
            return;
        }
        bounds[0] = shape->bounds[0];
        bounds[1] = shape->bounds[1];
        bounds[2] = shape->bounds[2];
        bounds[3] = shape->bounds[3];
        for (shape = shape->next; shape != nullptr; shape = shape->next)
        {
            bounds[0] = nsvg_minf(bounds[0], shape->bounds[0]);
            bounds[1] = nsvg_minf(bounds[1], shape->bounds[1]);
            bounds[2] = nsvg_maxf(bounds[2], shape->bounds[2]);
            bounds[3] = nsvg_maxf(bounds[3], shape->bounds[3]);
        }
    }

    static float nsvg_viewAlign(float content, float container, int type)
    {
        if (type == NSVG_ALIGN_MIN)
            return 0;
        else if (type == NSVG_ALIGN_MAX)
            return container - content;
        // mid
        return (container - content) * 0.5f;
    }

    static void nsvg_scaleGradient(NSVGgradient* grad, float tx, float ty, float sx, float sy)
    {
        float t[6];
        nsvg_xformSetTranslation(t, tx, ty);
        nsvg_xformMultiply(grad->xform, t);

        nsvg_xformSetScale(t, sx, sy);
        nsvg_xformMultiply(grad->xform, t);
    }

    static void nsvg_scaleToViewbox(NSVGparser* p, const char* units)
    {
        NSVGshape* shape;
        NSVGpath* path;
        float tx, ty, sx, sy, us, bounds[4], t[6], avgs;
        int i;
        float* pt;

        // Guess image size if not set completely.
        nsvg_imageBounds(p, bounds);

        if (p->viewWidth == 0)
        {
            if (p->image->width > 0)
            {
                p->viewWidth = p->image->width;
            }
            else
            {
                p->viewMinx = bounds[0];
                p->viewWidth = bounds[2] - bounds[0];
            }
        }
        if (p->viewHeight == 0)
        {
            if (p->image->height > 0)
            {
                p->viewHeight = p->image->height;
            }
            else
            {
                p->viewMiny = bounds[1];
                p->viewHeight = bounds[3] - bounds[1];
            }
        }
        if (p->image->width == 0)
            p->image->width = p->viewWidth;
        if (p->image->height == 0)
            p->image->height = p->viewHeight;

        tx = -p->viewMinx;
        ty = -p->viewMiny;
        sx = p->viewWidth > 0 ? p->image->width / p->viewWidth : 0;
        sy = p->viewHeight > 0 ? p->image->height / p->viewHeight : 0;
        // Unit scaling
        us = 1.0f / nsvg_convertToPixels(p, nsvg_coord(1.0f, nsvg_parseUnits(units)), 0.0f, 1.0f);

        // Fix aspect ratio
        if (p->alignType == NSVG_ALIGN_MEET)
        {
            // fit whole image into viewbox
            sx = sy = nsvg_minf(sx, sy);
            tx += nsvg_viewAlign(p->viewWidth * sx, p->image->width, p->alignX) / sx;
            ty += nsvg_viewAlign(p->viewHeight * sy, p->image->height, p->alignY) / sy;
        }
        else if (p->alignType == NSVG_ALIGN_SLICE)
        {
            // fill whole viewbox with image
            sx = sy = nsvg_maxf(sx, sy);
            tx += nsvg_viewAlign(p->viewWidth * sx, p->image->width, p->alignX) / sx;
            ty += nsvg_viewAlign(p->viewHeight * sy, p->image->height, p->alignY) / sy;
        }

        // Transform
        sx *= us;
        sy *= us;
        avgs = (sx + sy) / 2.0f;
        for (shape = p->image->shapes; shape != nullptr; shape = shape->next)
        {
            shape->bounds[0] = (shape->bounds[0] + tx) * sx;
            shape->bounds[1] = (shape->bounds[1] + ty) * sy;
            shape->bounds[2] = (shape->bounds[2] + tx) * sx;
            shape->bounds[3] = (shape->bounds[3] + ty) * sy;
            for (path = shape->paths; path != nullptr; path = path->next)
            {
                path->bounds[0] = (path->bounds[0] + tx) * sx;
                path->bounds[1] = (path->bounds[1] + ty) * sy;
                path->bounds[2] = (path->bounds[2] + tx) * sx;
                path->bounds[3] = (path->bounds[3] + ty) * sy;
                for (i = 0; i < path->npts; i++)
                {
                    pt = &path->pts[i * 2];
                    pt[0] = (pt[0] + tx) * sx;
                    pt[1] = (pt[1] + ty) * sy;
                }
            }

            if (shape->fill.type == NSVGPaintLinearGradient ||
                shape->fill.type == NSVGPaintRadialGradient)
            {
                nsvg_scaleGradient(shape->fill.gradient, tx, ty, sx, sy);
                memcpy(t, shape->fill.gradient->xform, sizeof(float) * 6);
                nsvg_xformInverse(shape->fill.gradient->xform, t);
            }
            if (shape->stroke.type == NSVGPaintLinearGradient ||
                shape->stroke.type == NSVGPaintRadialGradient)
            {
                nsvg_scaleGradient(shape->stroke.gradient, tx, ty, sx, sy);
                memcpy(t, shape->stroke.gradient->xform, sizeof(float) * 6);
                nsvg_xformInverse(shape->stroke.gradient->xform, t);
            }

            shape->stroke_width *= avgs;
            shape->stroke_dash_offset *= avgs;
            for (i = 0; i < shape->stroke_dash_count; i++)
                shape->stroke_dash_array[i] *= avgs;
        }
    }

    static void nsvg_createGradients(NSVGparser* p)
    {
        NSVGshape* shape;

        for (shape = p->image->shapes; shape != nullptr; shape = shape->next)
        {
            if (shape->fill.type == NSVGPaintUndef)
            {
                if (shape->fill_gradient[0] != '\0')
                {
                    float inv[6], localBounds[4];
                    nsvg_xformInverse(inv, shape->xform);
                    nsvg_getLocalBounds(localBounds, shape, inv);
                    shape->fill.gradient = nsvg_createGradient(p, shape->fill_gradient, localBounds,
                                                               shape->xform, &shape->fill.type);
                }
                if (shape->fill.type == NSVGPaintUndef)
                    shape->fill.type = NSVGPaintNone;
            }
            if (shape->stroke.type == NSVGPaintUndef)
            {
                if (shape->stroke_gradient[0] != '\0')
                {
                    float inv[6], localBounds[4];
                    nsvg_xformInverse(inv, shape->xform);
                    nsvg_getLocalBounds(localBounds, shape, inv);
                    shape->stroke.gradient = nsvg_createGradient(
                        p, shape->stroke_gradient, localBounds, shape->xform, &shape->stroke.type);
                }
                if (shape->stroke.type == NSVGPaintUndef)
                    shape->stroke.type = NSVGPaintNone;
            }
        }
    }

    NSVGimage* nsvg_parse(char* input, const char* units, float dpi)
    {
        NSVGparser* p;
        NSVGimage* ret = nullptr;

        p = nsvg_createParser();
        if (p == nullptr)
            return nullptr;
        p->dpi = dpi;

        nsvg_parse_xml(input, nsvg_startElement, nsvg_endElement, nsvg_content, p);

        // Create gradients after all definitions have been parsed
        nsvg_createGradients(p);

        // Scale to viewBox
        nsvg_scaleToViewbox(p, units);

        ret = p->image;
        p->image = nullptr;

        nsvg_deleteParser(p);

        return ret;
    }

    NSVGimage* nsvg_parse_from_file(const char* filename, const char* units, float dpi)
    {
        FILE* fp = nullptr;
        size_t size;
        char* data = nullptr;
        NSVGimage* image = nullptr;

        fp = fopen(filename, "rb");
        if (!fp)
            goto error;
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = (char*)malloc(size + 1);
        if (data == nullptr)
            goto error;
        if (fread(data, 1, size, fp) != size)
            goto error;
        data[size] = '\0';  // Must be null terminated.
        fclose(fp);
        image = nsvg_parse(data, units, dpi);
        free(data);

        return image;

    error:
        if (fp)
            fclose(fp);
        if (data)
            free(data);
        if (image)
            nsvg_delete(image);
        return nullptr;
    }

    NSVGpath* nsvg_duplicate_path(NSVGpath* p)
    {
        NSVGpath* res = nullptr;

        if (p == nullptr)
            return nullptr;

        res = (NSVGpath*)malloc(sizeof(NSVGpath));
        if (res == nullptr)
            goto error;
        memset(res, 0, sizeof(NSVGpath));

        res->pts = (float*)malloc(p->npts * 2 * sizeof(float));
        if (res->pts == nullptr)
            goto error;
        memcpy(res->pts, p->pts, p->npts * sizeof(float) * 2);
        res->npts = p->npts;

        memcpy(res->bounds, p->bounds, sizeof(p->bounds));

        res->closed = p->closed;

        return res;

    error:
        if (res != nullptr)
        {
            free(res->pts);
            free(res);
        }
        return nullptr;
    }

    void nsvg_delete(NSVGimage* image)
    {
        NSVGshape *snext, *shape;
        if (image == nullptr)
            return;
        shape = image->shapes;
        while (shape != nullptr)
        {
            snext = shape->next;
            nsvg_deletePaths(shape->paths);
            nsvg_deletePaint(&shape->fill);
            nsvg_deletePaint(&shape->stroke);
            free(shape);
            shape = snext;
        }
        free(image);
    }
}
