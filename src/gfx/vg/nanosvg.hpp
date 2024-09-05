#pragma once

namespace rl::nvg::svg {

    enum NSVGpaintType {
        NSVGPaintUndef = -1,
        NSVGPaintNone = 0,
        NSVGPaintColor = 1,
        NSVGPaintLinearGradient = 2,
        NSVGPaintRadialGradient = 3
    };

    enum NSVGspreadType {
        NSVGSpreadPad = 0,
        NSVGSpreadReflect = 1,
        NSVGSpreadRepeat = 2
    };

    enum NSVGlineJoin {
        NSVGJoinMiter = 0,
        NSVGJoinRound = 1,
        NSVGJoinBevel = 2
    };

    enum NSVGlineCap {
        NSVGCapButt = 0,
        NSVGCapRound = 1,
        NSVGCapSquare = 2
    };

    enum NSVGfillRule {
        NSVGFillruleNonzero = 0,
        NSVGFillruleEvenodd = 1
    };

    enum NSVGflags {
        NSVGFlagsVisible = 0x01
    };

    struct NSVGgradientStop {
        unsigned int color;
        float offset;
    };

    struct NSVGgradient {
        float xform[6];
        char spread;
        float fx, fy;
        int nstops;
        NSVGgradientStop stops[1];
    };

    struct NSVGpaint {
        signed char type;

        union {
            unsigned int color;
            NSVGgradient* gradient;
        };
    };

    struct NSVGpath {
        float* pts;       // Cubic bezier points: x0,y0, [cpx1,cpx1,cpx2,cpy2,x1,y1], ...
        int npts;         // Total number of bezier points.
        char closed;      // Flag indicating if shapes should be treated as closed.
        float bounds[4];  // Tight bounding box of the shape [minx,miny,maxx,maxy].
        NSVGpath* next;   // Pointer to next path, or NULL if last element.
    };

    struct NSVGshape {
        char id[64];                 // Optional 'id' attr of the shape or its group
        NSVGpaint fill;              // Fill paint
        NSVGpaint stroke;            // Stroke paint
        float opacity;               // Opacity of the shape.
        float stroke_width;          // Stroke width (scaled).
        float stroke_dash_offset;    // Stroke dash offset (scaled).
        float stroke_dash_array[8];  // Stroke dash array (scaled).
        char stroke_dash_count;      // Number of dash values in dash array.
        char stroke_line_join;       // Stroke join type.
        char stroke_line_cap;        // Stroke cap type.
        float miter_limit;           // Miter limit
        char fill_rule;              // Fill rule, see NSVGfillRule.
        unsigned char flags;         // Logical or of NSVG_FLAGS_* flags
        float bounds[4];             // Tight bounding box of the shape [minx,miny,maxx,maxy].
        char fill_gradient[64];      // Optional 'id' of fill gradient
        char stroke_gradient[64];    // Optional 'id' of stroke gradient
        float xform[6];              // Root transformation for fill/stroke gradient
        NSVGpath* paths;             // Linked list of paths in the image.
        NSVGshape* next;             // Pointer to next shape, or NULL if last element.
    };

    struct NSVGimage {
        float width;        // Width of the image.
        float height;       // Height of the image.
        NSVGshape* shapes;  // Linked list of shapes in the image.
    };

    // Parses SVG file from a file, returns SVG image as paths.
    NSVGimage* nsvg_parse_from_file(const char* filename, const char* units, float dpi);

    // Parses SVG file from a null terminated string, returns SVG image as paths.
    // Important note: changes the string.
    NSVGimage* nsvg_parse(char* input, const char* units, float dpi);

    // Duplicates a path.
    NSVGpath* nsvg_duplicate_path(const NSVGpath* p);

    // Deletes an image.
    void nsvg_delete(NSVGimage* image);
}
