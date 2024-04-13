#pragma once

#include <cstdint>

#include "ds/color.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "graphics/vg/fontstash.hpp"

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable : 4201)  // nonstandard extension used : nameless struct/union
#endif

namespace rl::nvg {
    constexpr i32 MaxNVGStates{ 64 };
    // Length proportional to radius of a
    // cubic bezier handle for 90deg arcs.
    constexpr f32 NVGKappa90{ 0.5522847493f };

    enum class CompositeOperation {
        SourceOver,
        SourceIn,
        SourceOut,
        Atop,
        DestinationOver,
        DestinationIn,
        DestinationOut,
        DestinationAtop,
        Lighter,
        Copy,
        Xor,
    };

    enum class TextureProperty {
        None = 0,
        Alpha = 1 << 0,
        RGBA = 1 << 1,
    };

    enum class ShapeWinding {
        None = 0,
        CounterClockwise = 1,  // ShapeWinding for solid shapes
        Clockwise = 2,         // ShapeWinding for holes
    };

    enum class Solidity {
        Solid = 1,  // CCW
        Hole = 2,   // CW
    };

    enum class LineCap {
        Butt,
        Round,
        Square,
        Bevel,
        Miter,
    };

    enum class Align {
        None = 0,
        HLeft = 1 << 0,      // Default, align text horizontally to left.
        HCenter = 1 << 1,    // Align text horizontally to center.
        HRight = 1 << 2,     // Align text horizontally to right.
        VTop = 1 << 3,       // Align text vertically to top.
        VMiddle = 1 << 4,    // Align text vertically to middle.
        VBottom = 1 << 5,    // Align text vertically to bottom.
        VBaseline = 1 << 6,  // Default, align text vertically to baseline.
    };

    enum class BlendFactor {
        Zero = 1 << 0,
        One = 1 << 1,
        SrcColor = 1 << 2,
        OneMinusSrcColor = 1 << 3,
        DstColor = 1 << 4,
        OneMinusDstColor = 1 << 5,
        SrcAlpha = 1 << 6,
        OneMinusSrcAlpha = 1 << 7,
        DstAlpha = 1 << 8,
        OneMinusDstAlpha = 1 << 9,
        SrcAlphaSaturate = 1 << 10,
    };

    enum class ImageFlags {
        None = 0,
        NVGImageGenerateMipmaps = 1 << 0,  // Generate mipmaps during creation of the image.
        NVGImageRepeatX = 1 << 1,          // Repeat image in X direction.
        NVGImageRepeatY = 1 << 2,          // Repeat image in Y direction.
        NVGImageFlipY = 1 << 3,            // Flips (inverses) image in Y direction when rendered.
        PreMultiplied = 1 << 4,            // Image data has pre-multiplied alpha.
        NVGImageNearest = 1 << 5,          // Image interpolation is Nearest instead Linear

        NoDelete = 1 << 16,  // OpenGL only
    };

    enum {
        NvgInitFontimageSize = 512,
        NvgMaxFontimageSize = 2048,
        NvgMaxFontimages = 4
    };

    struct ScissorParams
    {
        f32 xform[6]{};
        f32 extent[2]{};
    };

    struct Vertex
    {
        f32 x{ 0.0f };
        f32 y{ 0.0f };
        f32 u{ 0.0f };
        f32 v{ 0.0f };
    };

    struct NVGpath
    {
        i32 first{ 0 };
        i32 count{ 0 };
        uint8_t closed{ 0 };
        i32 nbevel{ 0 };
        Vertex* fill{ nullptr };
        i32 nfill{ 0 };
        Vertex* stroke{ nullptr };
        i32 nstroke{ 0 };
        ShapeWinding winding{ ShapeWinding::None };
        i32 convex{ 0 };
    };

    struct CompositeOperationState
    {
        BlendFactor src_rgb{};
        BlendFactor dst_rgb{};
        BlendFactor src_alpha{};
        BlendFactor dst_alpha{};
    };

    struct PaintStyle
    {
        f32 xform[6]{};
        f32 extent[2]{};
        f32 radius{ 0.0f };
        f32 feather{ 0.0f };
        ds::color<f32> inner_color{ 0.0f, 0.0f, 0.0f, 0.0f };
        ds::color<f32> outer_color{ 0.0f, 0.0f, 0.0f, 0.0f };
        i32 image{ 0 };
    };

    struct State
    {
        CompositeOperationState composite_operation{};
        bool shape_anti_alias{ false };
        PaintStyle fill{};
        PaintStyle stroke{};
        f32 stroke_width{ 0.0f };
        f32 miter_limit{ 0.0f };
        LineCap line_join{ LineCap::Butt };
        LineCap line_cap{ LineCap::Butt };
        f32 alpha{ 0.0f };
        f32 xform[6]{};
        ScissorParams scissor{};
        f32 font_size{ 0.0f };
        f32 letter_spacing{ 0.0f };
        f32 line_height{ 0.0f };
        f32 font_blur{ 0.0f };
        Align text_align{ Align::None };
        i32 font_id{ 0 };
    };

    struct Params
    {
        void* user_ptr{ nullptr };
        bool edge_anti_alias{ false };

        i32 (*render_create)(void* uptr);
        i32 (*render_create_texture)(void* uptr, TextureProperty type, i32 w, i32 h,
                                     ImageFlags image_flags, const uint8_t* data);
        i32 (*render_delete_texture)(void* uptr, i32 image);
        i32 (*render_update_texture)(void* uptr, i32 image, i32 x, i32 y, i32 w, i32 h,
                                     const uint8_t* data);
        i32 (*render_get_texture_size)(void* uptr, i32 image, f32* w, f32* h);

        void (*render_viewport)(void* uptr, f32 width, f32 height, f32 device_pixel_ratio);
        void (*render_cancel)(void* uptr);
        void (*render_flush)(void* uptr);
        void (*render_fill)(void* uptr, const PaintStyle* paint,
                            CompositeOperationState composite_operation,
                            const ScissorParams* scissor, f32 fringe, const f32* bounds,
                            const NVGpath* paths, i32 npaths);
        void (*render_stroke)(void* uptr, const PaintStyle* paint,
                              CompositeOperationState composite_operation,
                              const ScissorParams* scissor, f32 fringe, f32 stroke_width,
                              const NVGpath* paths, i32 npaths);
        void (*render_triangles)(
            void* uptr, const PaintStyle* paint, CompositeOperationState composite_operation,
            const ScissorParams* scissor, const Vertex* verts, i32 nverts, f32 fringe);

        void (*render_delete)(void* uptr);
    };

    struct Point
    {
        f32 x{ 0.0f };
        f32 y{ 0.0f };
        f32 dx{ 0.0f };
        f32 dy{ 0.0f };
        f32 len{ 0.0f };
        f32 dmx{ 0.0f };
        f32 dmy{ 0.0f };
        uint8_t flags{ 0 };
    };

    struct PathCache
    {
        Point* points{ nullptr };
        i32 npoints{ 0 };
        i32 cpoints{ 0 };
        NVGpath* paths{ nullptr };
        i32 npaths{ 0 };
        i32 cpaths{ 0 };
        Vertex* verts{ nullptr };
        i32 nverts{ 0 };
        i32 cverts{ 0 };
        f32 bounds[4]{};
    };

    struct Context
    {
        Params params{};
        f32* commands{ nullptr };
        i32 ccommands{ 0 };
        i32 ncommands{ 0 };
        f32 commandx{ 0.0f };
        f32 commandy{ 0.0f };
        State states[MaxNVGStates]{};
        i32 nstates{ 0 };
        PathCache* cache{ nullptr };
        f32 tess_tol{ 0.0f };
        f32 dist_tol{ 0.0f };
        f32 fringe_width{ 0.0f };
        f32 device_px_ratio{ 0.0f };
        FONScontext* fs{ nullptr };
        i32 font_images[NvgMaxFontimages]{};
        i32 font_image_idx{ 0 };
        i32 draw_call_count{ 0 };
        i32 fill_tri_count{ 0 };
        i32 stroke_tri_count{ 0 };
        i32 text_tri_count{ 0 };
    };

    struct GlyphPosition
    {
        const char* str{ nullptr };  // Position of the glyph in the input string.
        f32 x{ 0.0f };               // The x-coordinate of the logical glyph position.
        f32 min_x{ 0.0f };
        f32 max_x{ 0.0f };
    };

    struct TextRow
    {
        const char* start{ nullptr };  // Pointer to the input text where the row starts.
        const char* end{ nullptr };    // Pointer to the input text where the row ends (one past the
                                       // last character).
        const char* next{ nullptr };   // Pointer to the beginning of the next row.
        f32 width{ 0.0f };             // Logical width of the row.
        f32 min_x{ 0.0f };
        f32 max_x{ 0.0f };
        // because of kerning and some parts over extending.
    };

    // Begin drawing a new frame
    // Calls to nanovg drawing API should be wrapped in BeginFrame() & EndFrame()
    // BeginFrame() defines the size of the window to render to in relation currently
    // set viewport (i.e. glViewport on GL backends). Device pixel ration allows to
    // control the rendering on Hi-DPI devices.
    // For example, GLFW returns two dimension for an opened window: window size and
    // frame buffer size. In that case you would set windowWidth/Height to the window size
    // devicePixelRatio to: frameBufferWidth / windowWidth.
    void begin_frame(Context* ctx, f32 window_width, f32 window_height, f32 device_pixel_ratio);

    // Cancels drawing the current frame.
    void cancel_frame(const Context* ctx);

    // Ends drawing flushing remaining render state.
    void end_frame(Context* ctx);

    //
    // Composite operation
    //
    // The composite operations in NanoVG are modeled after HTML Canvas API, and
    // the blend func is based on OpenGL (see corresponding manuals for more info).
    // The colors in the blending state have premultiplied alpha.

    // Sets the composite operation. The op parameter should be one of CompositeOperation.
    void global_composite_operation(Context* ctx, CompositeOperation op);

    // Sets the composite operation with custom pixel arithmetic. The parameters should be one
    // of NVGBlendFactor.
    void global_composite_blend_func(Context* ctx, BlendFactor sfactor, BlendFactor dfactor);

    // Sets the composite operation with custom pixel arithmetic for RGB and alpha components
    // separately. The parameters should be one of NVGblendFactor.
    void global_composite_blend_func_separate(Context* ctx, BlendFactor src_rgb,
                                              BlendFactor dst_rgb, BlendFactor src_alpha,
                                              BlendFactor dst_alpha);

    //
    // Color utils
    //
    // Colors in NanoVG are stored as unsigned ints in ABGR format.

    // Returns a color value from red, green, blue values. Alpha will be set to 255 (1.0f).
    // ds::color<f32> rgb(uint8_t r, uint8_t g, uint8_t b);

    // Linearly interpolates from color c0 to c1, and returns resulting color value.
    ds::color<f32> lerp_rgba(const ds::color<f32>& c0, const ds::color<f32>& c1, f32 u);

    // Sets transparency of a color value.
    ds::color<f32> trans_rgba(ds::color<f32> c0, uint8_t a);

    // Sets transparency of a color value.
    ds::color<f32> trans_rgba_f(ds::color<f32> c0, f32 a);

    // Returns color value specified by hue, saturation and lightness.
    // HSL values are all in range [0..1], alpha will be set to 255.
    ds::color<f32> hsl(f32 h, f32 s, f32 l);

    // Returns color value specified by hue, saturation and lightness and alpha.
    // HSL values are all in range [0..1], alpha in range [0..255]
    ds::color<f32> hsla(f32 h, f32 s, f32 l, uint8_t a);

    //
    // State Handling
    //
    // NanoVG contains state which represents how paths will be rendered.
    // The state contains transform, fill and stroke styles, text and font styles,
    // and scissor clipping.

    // Pushes and saves the current render state into a state stack.
    // A matching Restore() must be used to restore the state.
    void save(Context* ctx);

    // Pops and restores current render state.
    void restore(Context* ctx);

    // Resets current render state to default values. Does not affect the render state stack.
    void reset(Context* ctx);

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
    void shape_anti_alias(Context* ctx, bool enabled);

    // Sets current stroke style to a solid color.
    void stroke_color(Context* ctx, const ds::color<f32>& color);

    // Sets current stroke style to a paint, which can be a one of the gradients or a pattern.
    void stroke_paint(Context* ctx, const PaintStyle& paint);

    // Sets current fill style to a solid color.
    void fill_color(Context* ctx, const ds::color<f32>& color);

    // Sets current fill style to a paint, which can be a one of the gradients or a pattern.
    void fill_paint(Context* ctx, const PaintStyle& paint);
    void fill_paint(Context* ctx, PaintStyle&& paint);

    // Sets the miter limit of the stroke style.
    // Miter limit controls when a sharp corner is beveled.
    void miter_limit(Context* ctx, f32 limit);

    // Sets the stroke width of the stroke style.
    void stroke_width(Context* ctx, f32 width);

    // Sets how the end of the line (cap) is drawn,
    // Can be one of: NVG_BUTT (default), NVG_ROUND, NVG_SQUARE.
    void line_cap(Context* ctx, LineCap cap);

    // Sets how sharp path corners are drawn.
    // Can be one of NVG_MITER (default), NVG_ROUND, NVG_BEVEL.
    void line_join(Context* ctx, LineCap join);

    // Sets the transparency applied to all rendered shapes.
    // Already transparent paths will get proportionally more transparent as well.
    void global_alpha(Context* ctx, f32 alpha);

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
    void reset_transform(Context* ctx);

    // Premultiplies current coordinate system by specified matrix.
    // The parameters are interpreted as matrix as follows:
    //   [a c e]
    //   [b d f]
    //   [0 0 1]
    void transform(Context* ctx, f32 a, f32 b, f32 c, f32 d, f32 e, f32 f);

    // Translates current coordinate system.
    void translate(Context* ctx, f32 x, f32 y);
    void translate(Context* ctx, const ds::vector2<f32>& local_offset);

    // Rotates current coordinate system. Angle is specified in radians.
    void rotate(Context* ctx, f32 angle);

    // Skews the current coordinate system along X axis. Angle is specified in radians.
    void skew_x(Context* ctx, f32 angle);

    // Skews the current coordinate system along Y axis. Angle is specified in radians.
    void skew_y(Context* ctx, f32 angle);

    // Scales the current coordinate system.
    void scale(Context* ctx, f32 x, f32 y);

    // Stores the top part (a-f) of the current transformation matrix in to the specified
    // buffer.
    //   [a c e]
    //   [b d f]
    //   [0 0 1]
    // There should be space for 6 f32s in the return buffer for the values a-f.
    void current_transform(Context* ctx, f32* xform);

    // The following functions can be used to make calculations on 2x3 transformation matrices.
    // A 2x3 matrix is represented as f32[6].

    // Sets the transform to identity matrix.
    void transform_identity(f32* dst);

    // Sets the transform to translation matrix matrix.
    void transform_translate(f32* dst, f32 tx, f32 ty);
    void transform_translate(f32* t, const ds::vector2<f32>& translation);

    // Sets the transform to scale matrix.
    void transform_scale(f32* dst, f32 sx, f32 sy);

    // Sets the transform to rotate matrix. Angle is specified in radians.
    void transform_rotate(f32* dst, f32 a);

    // Sets the transform to skew-x matrix. Angle is specified in radians.
    void transform_skew_x(f32* dst, f32 a);

    // Sets the transform to skew-y matrix. Angle is specified in radians.
    void transform_skew_y(f32* dst, f32 a);

    // Sets the transform to the result of multiplication of two transforms, of A = A*B.
    void transform_multiply(f32* dst, const f32* src);

    // Sets the transform to the result of multiplication of two transforms, of A = B*A.
    void transform_premultiply(f32* dst, const f32* src);

    // Sets the destination to inverse of specified transform.
    // Returns 1 if the inverse could be calculated, else 0.
    i32 transform_inverse(f32* dst, const f32* src);

    // Transform a point by given transform.
    void transform_point(f32* dstx, f32* dsty, const f32* xform, f32 srcx, f32 srcy);
    ds::point<f32> transform_point(Context* ctx, const ds::point<f32>& src_pt);

    // Converts degrees to radians and vice versa.
    f32 deg_to_rad(f32 deg);
    f32 rad_to_deg(f32 rad);

    //
    // Images
    //
    // NanoVG allows you to load jpg, png, psd, tga, pic and gif files to be used for rendering.
    // In addition you can upload your own image. The image loading is provided by stb_image.
    // The parameter imageFlags is combination of flags defined in ImageFlags.

    // Creates image by loading it from the disk from specified file name.
    // Returns handle to the image.
    i32 create_image(const Context* ctx, const char* filename, ImageFlags image_flags);

    // Creates image by loading it from the specified chunk of memory. Returns handle to the image.
    i32 create_image_mem(const Context* ctx, ImageFlags image_flags, const uint8_t* data, i32 ndata);

    // Creates image from specified image data. Returns handle to the image.
    i32 create_image_rgba(const Context* ctx, i32 w, i32 h, ImageFlags image_flags,
                          const uint8_t* data);

    i32 create_image_alpha(const Context* ctx, i32 w, i32 h, ImageFlags image_flags,
                           const uint8_t* data);

    // Updates image data specified by image handle.
    void update_image(const Context* ctx, i32 image, const uint8_t* data);

    // Returns the dimensions of a created image.
    void image_size(const Context* ctx, i32 image, f32* w, f32* h);
    ds::dims<f32> image_size(const Context* ctx, i32 image);

    // Deletes created image.
    void delete_image(const Context* ctx, i32 image);

    //
    // Paints
    //
    // NanoVG supports four types of paints: linear gradient, box gradient, radial gradient and
    // image pattern. These can be used as paints for strokes and fills.

    // Creates and returns a linear gradient. Parameters (sx,sy)-(ex,ey) specify the start and
    // end coordinates of the linear gradient, icol specifies the start color and ocol the end
    // color. The gradient is transformed by the current transform when it is passed to
    // FillPaint() or StrokePaint().
    PaintStyle linear_gradient(Context* ctx, f32 sx, f32 sy, f32 ex, f32 ey,
                               const ds::color<f32>& inner_color,
                               const ds::color<f32>& outer_color);

    // Creates and returns a box gradient. Box gradient is a feathered rounded rectangle, it is
    // useful for rendering drop shadows or highlights for boxes. Parameters (x,y) define the
    // top-left corner of the rectangle, (w,h) define the size of the rectangle, r defines the
    // corner radius, and f feather. Feather defines how blurry the border of the rectangle is.
    // Parameter icol specifies the inner color and ocol the outer color of the gradient. The
    // gradient is transformed by the current transform when it is passed to FillPaint() or
    // StrokePaint().
    PaintStyle box_gradient(Context* ctx, f32 x, f32 y, f32 w, f32 h, f32 r, f32 f,
                            const ds::color<f32>& icol, const ds::color<f32>& ocol);

    PaintStyle box_gradient(Context* ctx, ds::rect<f32>&& rect, f32 corner_radius, f32 feather_blur,
                            const ds::color<f32>& inner_color,
                            const ds::color<f32>& outer_gradient_color);

    // Creates and returns a radial gradient. Parameters (cx,cy) specify the center, inr and
    // outr specify the inner and outer radius of the gradient, inner_color specifies the start
    // color and outer_color the end color. The gradient is transformed by the current transform
    // when it is passed to FillPaint() or StrokePaint().
    PaintStyle radial_gradient(Context* ctx, f32 cx, f32 cy, f32 inr, f32 outr,
                               const ds::color<f32>& inner_color,
                               const ds::color<f32>& outer_color);

    // Creates and returns an image pattern. Parameters (ox,oy) specify the left-top location of
    // the image pattern, (ex,ey) the size of one image, angle rotation around the top-left
    // corner, image is handle to the image to render. The gradient is transformed by the
    // current transform when it is passed to FillPaint() or StrokePaint().
    PaintStyle image_pattern(Context* ctx, f32 cx, f32 cy, f32 w, f32 h, f32 angle, i32 image,
                             f32 alpha);

    //
    // Scissoring
    //
    // Scissoring allows you to clip the rendering into a rectangle. This is useful for various
    // user interface cases like rendering a text edit or a timeline.

    // Sets the current scissor rectangle.
    // The scissor rectangle is transformed by the current transform.
    void scissor(Context* ctx, f32 x, f32 y, f32 w, f32 h);

    // Intersects current scissor rectangle with the specified rectangle.
    // The scissor rectangle is transformed by the current transform.
    // Note: in case the rotation of previous scissor rect differs from
    // the current one, the intersection will be done between the specified
    // rectangle and the previous scissor rectangle transformed in the current
    // transform space. The resulting shape is always rectangle.
    void intersect_scissor(Context* ctx, f32 x, f32 y, f32 w, f32 h);

    // Reset and disables scissoring.
    void reset_scissor(Context* ctx);

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
    void begin_path(Context* ctx);

    // Starts new sub-path with specified point as first point.
    void move_to(Context* ctx, f32 x, f32 y);

    // Adds line segment from the last point in the path to the specified point.
    void line_to(Context* ctx, f32 x, f32 y);

    // Adds cubic bezier segment from last point in the path via two control points to the
    // specified point.
    void bezier_to(Context* ctx, f32 c1_x, f32 c1_y, f32 c2_x, f32 c2_y, f32 x, f32 y);

    // Adds quadratic bezier segment from last point in the path via a control point to the
    // specified point.
    void quad_to(Context* ctx, f32 cx, f32 cy, f32 x, f32 y);

    // Adds an arc segment at the corner defined by the last path point, and two specified
    // points.
    void arc_to(Context* ctx, f32 x1, f32 y1, f32 x2, f32 y2, f32 radius);

    // Closes current sub-path with a line segment.
    void close_path(Context* ctx);

    // Sets the current sub-path winding, see ShapeWinding and Solidity.
    void path_winding(Context* ctx, Solidity dir);

    // Creates new circle arc shaped sub-path. The arc center is at cx,cy, the arc radius is r,
    // and the arc is drawn from angle a0 to a1, and swept in direction dir (CounterClockwise,
    // or Clockwise). Angles are specified in radians.
    void arc(Context* ctx, f32 cx, f32 cy, f32 r, f32 a0, f32 a1, ShapeWinding dir);

    void barc(Context* ctx, f32 cx, f32 cy, f32 r, f32 a0, f32 a1, ShapeWinding dir, i32 join);

    // Creates new rectangle shaped sub-path.
    void rect(Context* ctx, f32 x, f32 y, f32 w, f32 h);

    // Creates new rounded rectangle shaped sub-path.
    void rounded_rect(Context* ctx, const ds::rect<f32>& rect, f32 radius);
    void rounded_rect(Context* ctx, f32 x, f32 y, f32 w, f32 h, f32 r);

    // Creates new rounded rectangle shaped sub-path with varying radii for each corner.
    void rounded_rect_varying(Context* ctx, f32 x, f32 y, f32 w, f32 h, f32 rad_top_left,
                              f32 rad_top_right, f32 rad_bottom_right, f32 rad_bottom_left);

    // Creates new ellipse shaped sub-path.
    void ellipse(Context* ctx, f32 cx, f32 cy, f32 rx, f32 ry);

    // Creates new circle shaped sub-path.
    void circle(Context* ctx, f32 cx, f32 cy, f32 r);

    // Fills the current path with current fill style.
    void fill(Context* ctx);

    // Fills the current path with current stroke style.
    void stroke(Context* ctx);

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
    i32 create_font(const Context* ctx, const char* name, const char* filename);

    // fontIndex specifies which font face to load from a .ttf/.ttc file.
    i32 create_font_at_index(const Context* ctx, const char* name, const char* filename,
                             i32 font_index);

    // Creates font by loading it from the specified memory chunk.
    // Returns handle to the font.
    i32 create_font_mem(const Context* ctx, const char* name, uint8_t* data, i32 ndata,
                        i32 free_data);
    i32 create_font_mem(const Context* ctx, const std::string_view& name,
                        const std::basic_string_view<u8>& font_data) noexcept;

    // fontIndex specifies which font face to load from a .ttf/.ttc file.
    i32 create_font_mem_at_index(const Context* ctx, const char* name, uint8_t* data, i32 ndata,
                                 i32 free_data, i32 font_index);

    // Finds a loaded font of specified name, and returns handle to it, or -1 if the font is not
    // found.
    i32 find_font_(const Context* ctx, const char* name);

    // Adds a fallback font by handle.
    i32 add_fallback_font_id(const Context* ctx, i32 base_font, i32 fallback_font);

    // Adds a fallback font by name.
    i32 add_fallback_font(const Context* ctx, const char* base_font, const char* fallback_font);

    // Resets fallback fonts by handle.
    void reset_fallback_fonts_id(const Context* ctx, i32 base_font);

    // Resets fallback fonts by name.
    void reset_fallback_fonts(const Context* ctx, const char* base_font);

    // Sets the font size of current text style.
    void set_font_size(Context* ctx, f32 size);

    // Sets the blur of current text style.
    void font_blur_(Context* ctx, f32 blur);

    // Sets the letter spacing of current text style.
    void text_letter_spacing_(Context* ctx, f32 spacing);

    // Sets the proportional line height of current text style. The line height is specified as
    // multiple of font size.
    void text_line_height_(Context* ctx, f32 line_height);

    // Sets the text align of current text style, see Align for options.
    void set_text_align(Context* ctx, Align align);

    // Sets the font face based on specified id of current text style.
    void font_face_id_(Context* ctx, i32 font);

    // Sets the font face based on specified name of current text style.
    void set_font_face(Context* ctx, const char* font);
    void set_font_face(Context* ctx, const std::string_view& font);
    void set_font_face(Context* ctx, const std::string& font);

    // Draws text string at specified location. If end is specified only the sub-string up to
    // the end is drawn.
    f32 text_(Context* ctx, f32 x, f32 y, const char* string, const char* end = nullptr);

    // Draws multi-line text string at specified location wrapped at the specified width. If end
    // is specified only the sub-string up to the end is drawn. White space is stripped at the
    // beginning of the rows, the text is split at word boundaries or when new-line characters
    // are encountered. Words longer than the max width are slit at nearest character (i.e. no
    // hyphenation).
    void text_box_(Context* ctx, f32 x, f32 y, f32 break_row_width, const char* string,
                   const char* end = nullptr);

    // Measures the specified text string. Parameter bounds should be a pointer to f32[4],
    // if the bounding box of the text should be returned. The bounds value are [xmin,ymin,
    // xmax,ymax] Returns the horizontal advance of the measured text (i.e. where the next
    // character should drawn). Measured values are returned in local coordinate space.
    f32 text_bounds_(Context* ctx, ds::point<f32>&& pos, std::string&& text);
    f32 text_bounds_(Context* ctx, f32 x, f32 y, const char* text, const char* end = nullptr,
                     f32* bounds = nullptr);

    // Measures the specified multi-text string. Parameter bounds should be a pointer to
    // f32[4], if the bounding box of the text should be returned. The bounds value are
    // [xmin,ymin, xmax,ymax] Measured values are returned in local coordinate space.
    void text_box_bounds_(Context* ctx, f32 x, f32 y, f32 break_row_width, const char* string,
                          const char* end, f32* bounds);

    // Calculates the glyph x positions of the specified text. If end is specified only the
    // sub-string will be used. Measured values are returned in local coordinate space.
    i32 text_glyph_positions_(Context* ctx, f32 x, f32 y, const char* string, const char* end,
                              GlyphPosition* positions, i32 max_positions);

    // Returns the vertical metrics based on the current text style.
    // Measured values are returned in local coordinate space.
    void text_metrics_(Context* ctx, f32* ascender, f32* descender, f32* lineh);

    // Breaks the specified text into lines. If end is specified only the sub-string will be
    // used. White space is stripped at the beginning of the rows, the text is split at word
    // boundaries or when new-line characters are encountered. Words longer than the max width
    // are slit at nearest character (i.e. no hyphenation).
    i32 text_break_lines_(Context* ctx, const char* string, const char* end, f32 break_row_width,
                          TextRow* rows, i32 max_rows);

    // Constructor and destructor, called by the render back-end.
    Context* create_internal(const Params* params);
    void delete_internal(Context* ctx);

    Params* internal_params(Context* ctx);

    // Debug function to dump cached path data.
    void debug_dump_path_cache(const Context* ctx);

#ifdef _MSC_VER
  #pragma warning(pop)
#endif

}
