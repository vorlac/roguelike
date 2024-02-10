#pragma once

#include <cstdint>

#include "ds/point.hpp"

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable : 4201)  // nonstandard extension used : nameless struct/union
#endif

namespace rl::nvg {

    struct NVGcontext;

    struct NVGcolor
    {
        union
        {
            float rgba[4];

            struct
            {
                float r;
                float g;
                float b;
                float a;
            };
        };
    };

    struct NVGpaint
    {
        float xform[6];
        float extent[2];
        float radius;
        float feather;
        NVGcolor inner_color;
        NVGcolor outer_color;
        int32_t image;
    };

    enum NVGwinding {
        NVGccw = 1,  // Winding for solid shapes
        NVGcw = 2,   // Winding for holes
    };

    enum NVGsolidity {
        NVGSolid = 1,  // CCW
        NVGHole = 2,   // CW
    };

    enum NVGlineCap {
        NVGButt,
        NVGRound,
        NVGSquare,
        NVGBevel,
        NVGMiter,
    };

    enum NVGalign {
        // Horizontal align
        NVGAlignLeft = 1 << 0,    // Default, align text horizontally to left.
        NVGAlignCenter = 1 << 1,  // Align text horizontally to center.
        NVGAlignRight = 1 << 2,   // Align text horizontally to right.

        // Vertical align
        NVGAlignTop = 1 << 3,       // Align text vertically to top.
        NVGAlignMiddle = 1 << 4,    // Align text vertically to middle.
        NVGAlignBottom = 1 << 5,    // Align text vertically to bottom.
        NVGAlignBaseline = 1 << 6,  // Default, align text vertically to baseline.
    };

    enum NVGblendFactor {
        NVGZero = 1 << 0,
        NVGOne = 1 << 1,
        NVGSrcColor = 1 << 2,
        NVGOneMinusSrcColor = 1 << 3,
        NVGDstColor = 1 << 4,
        NVGOneMinusDstColor = 1 << 5,
        NVGSrcAlpha = 1 << 6,
        NVGOneMinusSrcAlpha = 1 << 7,
        NVGDstAlpha = 1 << 8,
        NVGOneMinusDstAlpha = 1 << 9,
        NVGSrcAlphaSaturate = 1 << 10,
    };

    enum NVGcompositeOperation {
        NVGSourceOver,
        NVGSourceIn,
        NVGSourceOut,
        NVGAtop,
        NVGDestinationOver,
        NVGDestinationIn,
        NVGDestinationOut,
        NVGDestinationAtop,
        NVGLighter,
        NVGCopy,
        NVGXor,
    };

    struct NVGcompositeOperationState
    {
        int32_t src_rgb;
        int32_t dst_rgb;
        int32_t src_alpha;
        int32_t dst_alpha;
    };

    struct NVGglyphPosition
    {
        const char* str;  // Position of the glyph in the input string.
        float x;          // The x-coordinate of the logical glyph position.
        float minx;
        float maxx;
    };

    struct NVGtextRow
    {
        const char* start;  // Pointer to the input text where the row starts.
        const char* end;    // Pointer to the input text where the row ends (one past the last
                            // character).
        const char* next;   // Pointer to the beginning of the next row.
        float width;        // Logical width of the row.
        float minx, maxx;   // Actual bounds of the row. Logical with and bounds can differ
                            // because of kerning and some parts over extending.
    };

    enum NVGimageFlags {
        NVGImageGenerateMipmaps = 1 << 0,  // Generate mipmaps during creation of the image.
        NVGImageRepeatx = 1 << 1,          // Repeat image in X direction.
        NVGImageRepeaty = 1 << 2,          // Repeat image in Y direction.
        NVGImageFlipy = 1 << 3,            // Flips (inverses) image in Y direction when rendered.
        NVGImagePremultiplied = 1 << 4,    // Image data has premultiplied alpha.
        NVGImageNearest = 1 << 5,          // Image interpolation is Nearest instead Linear
    };

    // Begin drawing a new frame
    // Calls to nanovg drawing API should be wrapped in BeginFrame() & EndFrame()
    // BeginFrame() defines the size of the window to render to in relation currently
    // set viewport (i.e. glViewport on GL backends). Device pixel ration allows to
    // control the rendering on Hi-DPI devices.
    // For example, GLFW returns two dimension for an opened window: window size and
    // frame buffer size. In that case you would set windowWidth/Height to the window size
    // devicePixelRatio to: frameBufferWidth / windowWidth.
    void begin_frame(NVGcontext* ctx, float window_width, float window_height,
                     float device_pixel_ratio);

    // Cancels drawing the current frame.
    void cancel_frame(const NVGcontext* ctx);

    // Ends drawing flushing remaining render state.
    void end_frame(NVGcontext* ctx);

    //
    // Composite operation
    //
    // The composite operations in NanoVG are modeled after HTML Canvas API, and
    // the blend func is based on OpenGL (see corresponding manuals for more info).
    // The colors in the blending state have premultiplied alpha.

    // Sets the composite operation. The op parameter should be one of NVGcompositeOperation.
    void global_composite_operation(NVGcontext* ctx, int32_t op);

    // Sets the composite operation with custom pixel arithmetic. The parameters should be one
    // of NVGblendFactor.
    void global_composite_blend_func(NVGcontext* ctx, int32_t sfactor, int32_t dfactor);

    // Sets the composite operation with custom pixel arithmetic for RGB and alpha components
    // separately. The parameters should be one of NVGblendFactor.
    void global_composite_blend_func_separate(NVGcontext* ctx, int32_t src_rgb, int32_t dst_rgb,
                                              int32_t src_alpha, int32_t dst_alpha);

    //
    // Color utils
    //
    // Colors in NanoVG are stored as unsigned ints in ABGR format.

    // Returns a color value from red, green, blue values. Alpha will be set to 255 (1.0f).
    // NVGcolor rgb(uint8_t r, uint8_t g, uint8_t b);

    constexpr NVGcolor rgba_f(const float r, const float g, const float b, const float a)
    {
        return NVGcolor{
            r,
            g,
            b,
            a,
        };
    }

    // Returns a color value from red, green, blue values. Alpha will be set to 1.0f.
    constexpr NVGcolor rgb_f(const float r, const float g, const float b)
    {
        return rgba_f(r, g, b, 1.0f);
    }

    consteval NVGcolor rgba(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
    {
        return {
            static_cast<float>(r) / 255.0f,
            static_cast<float>(g) / 255.0f,
            static_cast<float>(b) / 255.0f,
            static_cast<float>(a) / 255.0f,
        };
    }

    consteval NVGcolor rgb(const uint8_t r, const uint8_t g, const uint8_t b)
    {
        return rgba(r, g, b, 255);
    }

    // Returns a color value from red, green, blue and alpha values.

    // Linearly interpolates from color c0 to c1, and returns resulting color value.
    NVGcolor lerp_rgba(NVGcolor c0, NVGcolor c1, float u);

    // Sets transparency of a color value.
    NVGcolor trans_rgba(NVGcolor c0, uint8_t a);

    // Sets transparency of a color value.
    NVGcolor trans_rgba_f(NVGcolor c0, float a);

    // Returns color value specified by hue, saturation and lightness.
    // HSL values are all in range [0..1], alpha will be set to 255.
    NVGcolor hsl(float h, float s, float l);

    // Returns color value specified by hue, saturation and lightness and alpha.
    // HSL values are all in range [0..1], alpha in range [0..255]
    NVGcolor hsla(float h, float s, float l, uint8_t a);

    //
    // State Handling
    //
    // NanoVG contains state which represents how paths will be rendered.
    // The state contains transform, fill and stroke styles, text and font styles,
    // and scissor clipping.

    // Pushes and saves the current render state into a state stack.
    // A matching Restore() must be used to restore the state.
    void save(NVGcontext* ctx);

    // Pops and restores current render state.
    void restore(NVGcontext* ctx);

    // Resets current render state to default values. Does not affect the render state stack.
    void reset(NVGcontext* ctx);

    //
    // Render styles
    //
    // Fill and stroke render style can be either a solid color or a paint which is a gradient
    // or a pattern. Solid color is simply defined as a color value, different kinds of paints
    // can be created using LinearGradient(), BoxGradient(), RadialGradient() and
    // ImagePattern().
    //
    // Current render style can be saved and restored using Save() and Restore().

    // Sets whether to draw antialias for Stroke() and Fill(). It's enabled by default.
    void shape_anti_alias(NVGcontext* ctx, int32_t enabled);

    // Sets current stroke style to a solid color.
    void stroke_color(NVGcontext* ctx, NVGcolor color);

    // Sets current stroke style to a paint, which can be a one of the gradients or a pattern.
    void stroke_paint(NVGcontext* ctx, const NVGpaint paint);

    // Sets current fill style to a solid color.
    void fill_color(NVGcontext* ctx, NVGcolor color);

    // Sets current fill style to a paint, which can be a one of the gradients or a pattern.
    void fill_paint(NVGcontext* ctx, const NVGpaint paint);

    // Sets the miter limit of the stroke style.
    // Miter limit controls when a sharp corner is beveled.
    void miter_limit(NVGcontext* ctx, float limit);

    // Sets the stroke width of the stroke style.
    void stroke_width(NVGcontext* ctx, float width);

    // Sets how the end of the line (cap) is drawn,
    // Can be one of: NVG_BUTT (default), NVG_ROUND, NVG_SQUARE.
    void line_cap(NVGcontext* ctx, int32_t cap);

    // Sets how sharp path corners are drawn.
    // Can be one of NVG_MITER (default), NVG_ROUND, NVG_BEVEL.
    void line_join(NVGcontext* ctx, int32_t join);

    // Sets the transparency applied to all rendered shapes.
    // Already transparent paths will get proportionally more transparent as well.
    void global_alpha(NVGcontext* ctx, float alpha);

    //
    // Transforms
    //
    // The paths, gradients, patterns and scissor region are transformed by an transformation
    // matrix at the time when they are passed to the API.
    // The current transformation matrix is a affine matrix:
    //   [sx kx tx]
    //   [ky sy ty]
    //   [ 0  0  1]
    // Where: sx,sy define scaling, kx,ky skewing, and tx,ty translation.
    // The last row is assumed to be 0,0,1 and is not stored.
    //
    // Apart from ResetTransform(), each transformation function first creates
    // specific transformation matrix and pre-multiplies the current transformation by it.
    //
    // Current coordinate system (transformation) can be saved and restored using Save() and
    // Restore().

    // Resets current transform to a identity matrix.
    void reset_transform(NVGcontext* ctx);

    // Premultiplies current coordinate system by specified matrix.
    // The parameters are interpreted as matrix as follows:
    //   [a c e]
    //   [b d f]
    //   [0 0 1]
    void transform(NVGcontext* ctx, float a, float b, float c, float d, float e, float f);

    // Translates current coordinate system.
    void translate(NVGcontext* ctx, float x, float y);
    void translate(NVGcontext* ctx, ds::vector2<f32>&& local_offset);

    // Rotates current coordinate system. Angle is specified in radians.
    void rotate(NVGcontext* ctx, float angle);

    // Skews the current coordinate system along X axis. Angle is specified in radians.
    void skew_x(NVGcontext* ctx, float angle);

    // Skews the current coordinate system along Y axis. Angle is specified in radians.
    void skew_y(NVGcontext* ctx, float angle);

    // Scales the current coordinate system.
    void scale(NVGcontext* ctx, float x, float y);

    // Stores the top part (a-f) of the current transformation matrix in to the specified
    // buffer.
    //   [a c e]
    //   [b d f]
    //   [0 0 1]
    // There should be space for 6 floats in the return buffer for the values a-f.
    void current_transform(NVGcontext* ctx, float* xform);

    // The following functions can be used to make calculations on 2x3 transformation matrices.
    // A 2x3 matrix is represented as float[6].

    // Sets the transform to identity matrix.
    void transform_identity(float* dst);

    // Sets the transform to translation matrix matrix.
    void transform_translate(float* dst, float tx, float ty);
    void transform_translate(float* t, ds::vector2<f32>&& translation);

    // Sets the transform to scale matrix.
    void transform_scale(float* dst, float sx, float sy);

    // Sets the transform to rotate matrix. Angle is specified in radians.
    void transform_rotate(float* dst, float a);

    // Sets the transform to skew-x matrix. Angle is specified in radians.
    void transform_skew_x(float* dst, float a);

    // Sets the transform to skew-y matrix. Angle is specified in radians.
    void transform_skew_y(float* dst, float a);

    // Sets the transform to the result of multiplication of two transforms, of A = A*B.
    void transform_multiply(float* dst, const float* src);

    // Sets the transform to the result of multiplication of two transforms, of A = B*A.
    void transform_premultiply(float* dst, const float* src);

    // Sets the destination to inverse of specified transform.
    // Returns 1 if the inverse could be calculated, else 0.
    int32_t transform_inverse(float* dst, const float* src);

    // Transform a point by given transform.
    void transform_point(float* dstx, float* dsty, const float* xform, float srcx, float srcy);

    // Converts degrees to radians and vice versa.
    float deg_to_rad(float deg);
    float rad_to_deg(float rad);

    //
    // Images
    //
    // NanoVG allows you to load jpg, png, psd, tga, pic and gif files to be used for rendering.
    // In addition you can upload your own image. The image loading is provided by stb_image.
    // The parameter imageFlags is combination of flags defined in NVGimageFlags.

    // Creates image by loading it from the disk from specified file name.
    // Returns handle to the image.
    int32_t create_image(const NVGcontext* ctx, const char* filename, int32_t imageFlags);

    // Creates image by loading it from the specified chunk of memory.
    // Returns handle to the image.
    int32_t create_image_mem(const NVGcontext* ctx, int32_t imageFlags, const uint8_t* data,
                             int32_t ndata);

    // Creates image from specified image data.
    // Returns handle to the image.
    int32_t create_image_rgba(const NVGcontext* ctx, int32_t w, int32_t h, int32_t imageFlags,
                              const uint8_t* data);

    int32_t create_image_alpha(const NVGcontext* ctx, int32_t w, int32_t h, int32_t imageFlags,
                               const uint8_t* data);

    // Updates image data specified by image handle.
    void update_image(const NVGcontext* ctx, int32_t image, const uint8_t* data);

    // Returns the dimensions of a created image.
    void image_size(const NVGcontext* ctx, int32_t image, int32_t* w, int32_t* h);

    // Deletes created image.
    void delete_image(const NVGcontext* ctx, int32_t image);

    //
    // Paints
    //
    // NanoVG supports four types of paints: linear gradient, box gradient, radial gradient and
    // image pattern. These can be used as paints for strokes and fills.

    // Creates and returns a linear gradient. Parameters (sx,sy)-(ex,ey) specify the start and
    // end coordinates of the linear gradient, icol specifies the start color and ocol the end
    // color. The gradient is transformed by the current transform when it is passed to
    // FillPaint() or StrokePaint().
    NVGpaint linear_gradient(NVGcontext* ctx, float sx, float sy, float ex, float ey, NVGcolor icol,
                             NVGcolor ocol);

    // Creates and returns a box gradient. Box gradient is a feathered rounded rectangle, it is
    // useful for rendering drop shadows or highlights for boxes. Parameters (x,y) define the
    // top-left corner of the rectangle, (w,h) define the size of the rectangle, r defines the
    // corner radius, and f feather. Feather defines how blurry the border of the rectangle is.
    // Parameter icol specifies the inner color and ocol the outer color of the gradient. The
    // gradient is transformed by the current transform when it is passed to FillPaint() or
    // StrokePaint().
    NVGpaint box_gradient(NVGcontext* ctx, float x, float y, float w, float h, float r, float f,
                          NVGcolor icol, NVGcolor ocol);

    // Creates and returns a radial gradient. Parameters (cx,cy) specify the center, inr and
    // outr specify the inner and outer radius of the gradient, icol specifies the start color
    // and ocol the end color. The gradient is transformed by the current transform when it is
    // passed to FillPaint() or StrokePaint().
    NVGpaint radial_gradient(NVGcontext* ctx, float cx, float cy, float inr, float outr,
                             NVGcolor icol, NVGcolor ocol);

    // Creates and returns an image pattern. Parameters (ox,oy) specify the left-top location of
    // the image pattern, (ex,ey) the size of one image, angle rotation around the top-left
    // corner, image is handle to the image to render. The gradient is transformed by the
    // current transform when it is passed to FillPaint() or StrokePaint().
    NVGpaint image_pattern(NVGcontext* ctx, float cx, float cy, float w, float h, float angle,
                           int32_t image, float alpha);

    //
    // Scissoring
    //
    // Scissoring allows you to clip the rendering into a rectangle. This is useful for various
    // user interface cases like rendering a text edit or a timeline.

    // Sets the current scissor rectangle.
    // The scissor rectangle is transformed by the current transform.
    void scissor(NVGcontext* ctx, float x, float y, float w, float h);

    // Intersects current scissor rectangle with the specified rectangle.
    // The scissor rectangle is transformed by the current transform.
    // Note: in case the rotation of previous scissor rect differs from
    // the current one, the intersection will be done between the specified
    // rectangle and the previous scissor rectangle transformed in the current
    // transform space. The resulting shape is always rectangle.
    void intersect_scissor(NVGcontext* ctx, float x, float y, float w, float h);

    // Reset and disables scissoring.
    void reset_scissor(NVGcontext* ctx);

    //
    // Paths
    //
    // Drawing a new shape starts with BeginPath(), it clears all the currently defined
    // paths. Then you define one or more paths and sub-paths which describe the shape. The are
    // functions to draw common shapes like rectangles and circles, and lower level step-by-step
    // functions, which allow to define a path curve by curve.
    //
    // NanoVG uses even-odd fill rule to draw the shapes. Solid shapes should have counter
    // clockwise winding and holes should have counter clockwise order. To specify winding of a
    // path you can call PathWinding(). This is useful especially for the common shapes,
    // which are drawn CCW.
    //
    // Finally you can fill the path using current fill style by calling Fill(), and stroke
    // it with current stroke style by calling Stroke().
    //
    // The curve segments and sub-paths are transformed by the current transform.

    // Clears the current path and sub-paths.
    void begin_path(NVGcontext* ctx);

    // Starts new sub-path with specified point as first point.
    void move_to(NVGcontext* ctx, float x, float y);

    // Adds line segment from the last point in the path to the specified point.
    void line_to(NVGcontext* ctx, float x, float y);

    // Adds cubic bezier segment from last point in the path via two control points to the
    // specified point.
    void bezier_to(NVGcontext* ctx, float c1_x, float c1_y, float c2_x, float c2_y, float x,
                   float y);

    // Adds quadratic bezier segment from last point in the path via a control point to the
    // specified point.
    void quad_to(NVGcontext* ctx, float cx, float cy, float x, float y);

    // Adds an arc segment at the corner defined by the last path point, and two specified
    // points.
    void arc_to(NVGcontext* ctx, float x1, float y1, float x2, float y2, float radius);

    // Closes current sub-path with a line segment.
    void close_path(NVGcontext* ctx);

    // Sets the current sub-path winding, see NVGwinding and NVGsolidity.
    void path_winding(NVGcontext* ctx, int32_t dir);

    // Creates new circle arc shaped sub-path. The arc center is at cx,cy, the arc radius is r,
    // and the arc is drawn from angle a0 to a1, and swept in direction dir (NVGccw, or
    // NVGcw). Angles are specified in radians.
    void arc(NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int32_t dir);

    void barc(NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int32_t dir,
              int32_t join);

    // Creates new rectangle shaped sub-path.
    void rect(NVGcontext* ctx, float x, float y, float w, float h);

    // Creates new rounded rectangle shaped sub-path.
    void rounded_rect(NVGcontext* ctx, float x, float y, float w, float h, float r);

    // Creates new rounded rectangle shaped sub-path with varying radii for each corner.
    void rounded_rect_varying(NVGcontext* ctx, float x, float y, float w, float h,
                              float rad_top_left, float rad_top_right, float rad_bottom_right,
                              float rad_bottom_left);

    // Creates new ellipse shaped sub-path.
    void ellipse(NVGcontext* ctx, float cx, float cy, float rx, float ry);

    // Creates new circle shaped sub-path.
    void circle(NVGcontext* ctx, float cx, float cy, float r);

    // Fills the current path with current fill style.
    void fill(NVGcontext* ctx);

    // Fills the current path with current stroke style.
    void stroke(NVGcontext* ctx);

    //
    // Text
    //
    // NanoVG allows you to load .ttf files and use the font to render text.
    //
    // The appearance of the text can be defined by setting the current text style
    // and by specifying the fill color. Common text and font settings such as
    // font size, letter spacing and text align are supported. Font blur allows you
    // to create simple text effects such as drop shadows.
    //
    // At render time the font face can be set based on the font handles or name.
    //
    // Font measure functions return values in local space, the calculations are
    // carried in the same resolution as the final rendering. This is done because
    // the text glyph positions are snapped to the nearest pixels sharp rendering.
    //
    // The local space means that values are not rotated or scale as per the current
    // transformation. For example if you set font size to 12, which would mean that
    // line height is 16, then regardless of the current scaling and rotation, the
    // returned line height is always 16. Some measures may vary because of the scaling
    // since aforementioned pixel snapping.
    //
    // While this may sound a little odd, the setup allows you to always render the
    // same way regardless of scaling. I.e. following works regardless of scaling:
    //
    //		const char* txt = "Text me up.";
    //		TextBounds(vg, x,y, txt, NULL, bounds);
    //		BeginPath(vg);
    //		RoundedRect(vg, bounds[0],bounds[1], bounds[2]-bounds[0], bounds[3]-bounds[1]);
    //		Fill(vg);
    //
    // Note: currently only solid color fill is supported for text.

    // Creates font by loading it from the disk from specified file name.
    // Returns handle to the font.
    int32_t create_font(const NVGcontext* ctx, const char* name, const char* filename);

    // fontIndex specifies which font face to load from a .ttf/.ttc file.
    int32_t create_font_at_index(const NVGcontext* ctx, const char* name, const char* filename,
                                 int32_t font_index);

    // Creates font by loading it from the specified memory chunk.
    // Returns handle to the font.
    int32_t create_font_mem(const NVGcontext* ctx, const char* name, uint8_t* data, int32_t ndata,
                            int32_t free_data);
    int32_t create_font_mem(const NVGcontext* ctx, const std::string_view& name,
                            const std::basic_string_view<u8>& font_data) noexcept;

    // fontIndex specifies which font face to load from a .ttf/.ttc file.
    int32_t create_font_mem_at_index(const NVGcontext* ctx, const char* name, uint8_t* data,
                                     int32_t ndata, int32_t free_data, int32_t font_index);

    // Finds a loaded font of specified name, and returns handle to it, or -1 if the font is not
    // found.
    int32_t find_font(const NVGcontext* ctx, const char* name);

    // Adds a fallback font by handle.
    int32_t add_fallback_font_id(const NVGcontext* ctx, int32_t base_font, int32_t fallback_font);

    // Adds a fallback font by name.
    int32_t add_fallback_font(const NVGcontext* ctx, const char* base_font,
                              const char* fallback_font);

    // Resets fallback fonts by handle.
    void reset_fallback_fonts_id(const NVGcontext* ctx, int32_t base_font);

    // Resets fallback fonts by name.
    void reset_fallback_fonts(const NVGcontext* ctx, const char* base_font);

    // Sets the font size of current text style.
    void font_size(NVGcontext* ctx, float size);

    // Sets the blur of current text style.
    void font_blur(NVGcontext* ctx, float blur);

    // Sets the letter spacing of current text style.
    void text_letter_spacing(NVGcontext* ctx, float spacing);

    // Sets the proportional line height of current text style. The line height is specified as
    // multiple of font size.
    void text_line_height(NVGcontext* ctx, float line_height);

    // Sets the text align of current text style, see NVGalign for options.
    void text_align(NVGcontext* ctx, int32_t align);

    // Sets the font face based on specified id of current text style.
    void font_face_id(NVGcontext* ctx, int32_t font);

    // Sets the font face based on specified name of current text style.
    void font_face(NVGcontext* ctx, const char* font);
    void font_face(NVGcontext* ctx, const std::string_view& font);

    // Draws text string at specified location. If end is specified only the sub-string up to
    // the end is drawn.
    float text(NVGcontext* ctx, float x, float y, const char* string, const char* end = nullptr);

    // Draws multi-line text string at specified location wrapped at the specified width. If end
    // is specified only the sub-string up to the end is drawn. White space is stripped at the
    // beginning of the rows, the text is split at word boundaries or when new-line characters
    // are encountered. Words longer than the max width are slit at nearest character (i.e. no
    // hyphenation).
    void text_box(NVGcontext* ctx, float x, float y, float break_row_width, const char* string,
                  const char* end = nullptr);

    // Measures the specified text string. Parameter bounds should be a pointer to float[4],
    // if the bounding box of the text should be returned. The bounds value are [xmin,ymin,
    // xmax,ymax] Returns the horizontal advance of the measured text (i.e. where the next
    // character should drawn). Measured values are returned in local coordinate space.
    float text_bounds(NVGcontext* ctx, float x, float y, const char* string,
                      const char* end = nullptr, float* bounds = nullptr);

    // Measures the specified multi-text string. Parameter bounds should be a pointer to
    // float[4], if the bounding box of the text should be returned. The bounds value are
    // [xmin,ymin, xmax,ymax] Measured values are returned in local coordinate space.
    void text_box_bounds(NVGcontext* ctx, float x, float y, float break_row_width,
                         const char* string, const char* end, float* bounds);

    // Calculates the glyph x positions of the specified text. If end is specified only the
    // sub-string will be used. Measured values are returned in local coordinate space.
    int32_t text_glyph_positions(NVGcontext* ctx, float x, float y, const char* string,
                                 const char* end, NVGglyphPosition* positions,
                                 int32_t max_positions);

    // Returns the vertical metrics based on the current text style.
    // Measured values are returned in local coordinate space.
    void text_metrics(NVGcontext* ctx, float* ascender, float* descender, float* lineh);

    // Breaks the specified text into lines. If end is specified only the sub-string will be
    // used. White space is stripped at the beginning of the rows, the text is split at word
    // boundaries or when new-line characters are encountered. Words longer than the max width
    // are slit at nearest character (i.e. no hyphenation).
    int32_t text_break_lines(NVGcontext* ctx, const char* string, const char* end,
                             float break_row_width, NVGtextRow* rows, int32_t max_rows);

    //
    // Internal Render API
    //
    enum NVGtexture {
        NVGTextureAlpha = 0x01,
        NVGTextureRgba = 0x02,
    };

    struct NVGscissor
    {
        float xform[6];
        float extent[2];
    };

    struct NVGvertex
    {
        float x;
        float y;
        float u;
        float v;
    };

    struct NVGpath
    {
        int32_t first;
        int32_t count;
        uint8_t closed;
        int32_t nbevel;
        NVGvertex* fill;
        int32_t nfill;
        NVGvertex* stroke;
        int32_t nstroke;
        int32_t winding;
        int32_t convex;
    };
    typedef struct NVGpath NVGpath;

    struct NVGparams
    {
        void* user_ptr = nullptr;

        int32_t edge_anti_alias;

        int32_t (*render_create)(void* uptr);
        int32_t (*render_create_texture)(void* uptr, int32_t type, int32_t w, int32_t h,
                                         int32_t image_flags, const uint8_t* data);

        int32_t (*render_delete_texture)(void* uptr, int32_t image);
        int32_t (*render_update_texture)(void* uptr, int32_t image, int32_t x, int32_t y, int32_t w,
                                         int32_t h, const uint8_t* data);

        int32_t (*render_get_texture_size)(void* uptr, int32_t image, int32_t* w, int32_t* h);

        void (*render_viewport)(void* uptr, float width, float height, float device_pixel_ratio);
        void (*render_cancel)(void* uptr);
        void (*render_flush)(void* uptr);

        void (*render_fill)(void* uptr, const NVGpaint* paint,
                            NVGcompositeOperationState composite_operation,
                            const NVGscissor* scissor, float fringe, const float* bounds,
                            const NVGpath* paths, int32_t npaths);
        void (*render_stroke)(void* uptr, const NVGpaint* paint,
                              NVGcompositeOperationState composite_operation,
                              const NVGscissor* scissor, float fringe, float stroke_width,
                              const NVGpath* paths, int32_t npaths);
        void (*render_triangles)(
            void* uptr, const NVGpaint* paint, NVGcompositeOperationState composite_operation,
            const NVGscissor* scissor, const NVGvertex* verts, int32_t nverts, float fringe);

        void (*render_delete)(void* uptr);
    };

    // Constructor and destructor, called by the render back-end.
    NVGcontext* create_internal(const NVGparams* params);
    void delete_internal(NVGcontext* ctx);

    NVGparams* internal_params(NVGcontext* ctx);

    // Debug function to dump cached path data.
    void debug_dump_path_cache(const NVGcontext* ctx);

#ifdef _MSC_VER
  #pragma warning(pop)
#endif

}
