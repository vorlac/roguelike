#pragma once

namespace rl::nvg {
    using NSVGrasterizer = struct NSVGrasterizer;

    // Allocated rasterizer context.
    NSVGrasterizer* nsvg_create_rasterizer();

    // Rasterizes SVG image, returns RGBA image (non-premultiplied alpha)
    //   r - pointer to rasterizer context
    //   image - pointer to image to rasterize
    //   tx,ty - image offset (applied after scaling)
    //   scale - image scale (assumes square aspect ratio)
    //   dst - pointer to destination image data, 4 bytes per pixel (RGBA)
    //   w - width of the image to render
    //   h - height of the image to render
    //   stride - number of bytes per scaleline in the destination buffer
    void nsvg_rasterize(NSVGrasterizer* r, const nsvg::NSVGimage* image, float tx, float ty,
                        float scale, unsigned char* dst, int w, int h, int stride);

    // As above, but allow X and Y axes to scale independently for non-square aspects
    // Added by FLTK
    void nsvg_rasterize_xy(NSVGrasterizer* r, const nsvg::NSVGimage* image, float tx, float ty,
                           float sx, float sy, unsigned char* dst, int w, int h, int stride);

    // Deletes rasterizer context.
    void nsvg_delete_rasterizer(NSVGrasterizer*);
}
