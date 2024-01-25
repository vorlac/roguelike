#pragma once

namespace rl::nvg {
    typedef struct NSVGrasterizer NSVGrasterizer;

    /* Example Usage:
        // Load SVG
        NSVGimage* image;
        image = nsvgParseFromFile("test.svg", "px", 96);

        // Create rasterizer (can be used to render multiple images).
        struct NSVGrasterizer* rast = nsvgCreateRasterizer();
        // Allocate memory for image
        unsigned char* img = malloc(w*h*4);
        // Rasterize
        nsvgRasterize(rast, image, 0,0,1, img, w, h, w*4);

        // For non-square X,Y scaling, use
        nsvgRasterizeXY(rast, image, 0,0,1,1, img, w, h, w*4);
    */

    // Allocated rasterizer context.
    NSVGrasterizer* nsvgCreateRasterizer(void);

    // Rasterizes SVG image, returns RGBA image (non-premultiplied alpha)
    //   r - pointer to rasterizer context
    //   image - pointer to image to rasterize
    //   tx,ty - image offset (applied after scaling)
    //   scale - image scale (assumes square aspect ratio)
    //   dst - pointer to destination image data, 4 bytes per pixel (RGBA)
    //   w - width of the image to render
    //   h - height of the image to render
    //   stride - number of bytes per scaleline in the destination buffer
    void nsvgRasterize(NSVGrasterizer* r, const NSVGimage* image, float tx, float ty, float scale,
                       unsigned char* dst, int w, int h, int stride);

    // As above, but allow X and Y axes to scale independently for non-square aspects
    // Added by FLTK
    void nsvgRasterizeXY(NSVGrasterizer* r, const NSVGimage* image, float tx, float ty, float sx,
                         float sy, unsigned char* dst, int w, int h, int stride);

    // Deletes rasterizer context.
    void nsvgDeleteRasterizer(NSVGrasterizer*);
}
