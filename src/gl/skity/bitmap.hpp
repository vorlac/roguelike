#ifndef SKITY_GRAPHIC_BITMAP_HPP
#define SKITY_GRAPHIC_BITMAP_HPP

#include <cstdint>

#include "gl/skity/graphic/color.hpp"
#include "gl/skity/io/pixmap.hpp"

namespace skity {

    /**
     * @class Bitmap
     * Describtes a two-dimensional raster pixel array. For now the internal format
     * only support RGBA_32
     *
     * Bitmap can be drawn using Canvas with software raster or set pixel directory
     * by calling **Bitmap::setPixel**, **Bitmap::blendPixel**
     */
    class Bitmap
    {
    public:
        enum class BlendMode {
            kSrcOver,
        };

        Bitmap();
        Bitmap(uint32_t width, uint32_t height);

        Bitmap(const Bitmap&) = delete;
        Bitmap& operator=(const Bitmap&) = delete;

        ~Bitmap() = default;

        Color getPixel(uint32_t x, uint32_t y);

        void setPixel(uint32_t x, uint32_t y, Color color);

        void setPixel(uint32_t x, uint32_t y, Color4f color);

        void blendPixel(uint32_t x, uint32_t y, Color color, BlendMode blend = BlendMode::kSrcOver);

        void blendPixel(uint32_t x, uint32_t y, Color4f color,
                        BlendMode blend = BlendMode::kSrcOver);

        uint32_t width() const;
        uint32_t height() const;

        uint32_t* getPixelAddr() const
        {
            return pixel_addr_;
        }

        const Pixmap* getPixmap() const
        {
            return pixmap_.get();
        }

    private:
        std::shared_ptr<Pixmap> pixmap_;
        uint32_t* pixel_addr_;
    };

}  // namespace skity

#endif  // SKITY_GRAPHIC_BITMAP_HPP
