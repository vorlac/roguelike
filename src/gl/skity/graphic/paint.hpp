#ifndef SKITY_GRAPHIC_PAINT_HPP
#define SKITY_GRAPHIC_PAINT_HPP

#include <cstdint>
#include <memory>
#include <utility>

#include "gl/skity/geometry/point.hpp"
#include "gl/skity/graphic/color.hpp"

namespace skity {

    class MaskFilter;
    class PathEffect;
    class Shader;
    class Typeface;

    /**
     * @class Paint
     * Controls options applied when drawing.
     */
    class Paint
    {
        enum {
            DEFAULT_FONT_FILL_THRESHOLD = 256,
        };

    public:
        Paint();
        ~Paint();

        Paint& operator=(const Paint& paint);

        void reset();

        enum Style : std::uint8_t {
            kFill_Style,           /// set to fill geometry
            kStroke_Style,         /// set to stroke geometry
            kStrokeAndFill_Style,  /// set to stroke and fill geometry
        };

        // may be used to verify that Paint::Style is a legal value
        constexpr static int32_t StyleCount = kStrokeAndFill_Style + 1;

        Style getStyle() const;

        void setStyle(Style style);

        /**
         * Set the thickness of the pen used by the paint to outline the shape.
         * @TODO may be support stroke-width of zero as hairline
         * @param width pen thickness
         */
        void setStrokeWidth(float width);

        float getStrokeWidth() const;

        float getStrokeMiter() const;

        /**
         * Set the limit at which a sharp corner is drawn beveled.
         *
         * @param miter
         */
        void setStrokeMiter(float miter);

        /**
         * @enum Paint::Cap
         *
         * Cap draws at the beginning and end of an open path contour.
         */
        enum Cap : std::uint8_t {
            kButt_Cap,                 /// no stroke extension
            kRound_Cap,                /// add circle
            kSquare_Cap,               /// add square
            kLast_Cap = kSquare_Cap,   /// largest Cap value
            kDefault_Cap = kButt_Cap,  /// equivalent to kButt_Cap
        };

        constexpr static std::int32_t kCapCount = kLast_Cap + 1;

        Cap getStrokeCap() const;

        void setStrokeCap(Cap cap);

        /**
         * @enum Paint::Join
         *
         * Join specifies how corners are drawn when a shape is stroked.
         */
        enum Join : std::uint8_t {
            kMiter_Join,                  /// extends to miter limit
            kRound_Join,                  /// add circle
            kBevel_Join,                  /// connects outside edges
            kLast_Join = kBevel_Join,     /// equivalent to the largest value for Join
            kDefault_Join = kMiter_Join,  /// equivalent to kMiter_Join
        };

        constexpr static std::int32_t kJoinCount = kLast_Join + 1;

        Join getStrokeJoin() const;

        void setStrokeJoin(Join join);

        constexpr const static float DefaultMiterLimit = float(4);

        void setStrokeColor(float r, float g, float b, float a);

        void setStrokeColor(const Vector& color);

        Vector getStrokeColor() const;

        void setFillColor(float r, float g, float b, float a);

        void setFillColor(const Vector& color);

        Vector getFillColor() const;

        /**
         * Sets alpha and RGB used when stroking and filling. The color is a 32-bit
         * value, unpremultiplied, packing 8-bit components for alpha, red, blue, and
         * green.
         * @param color   unpremultiplied ARGB
         */
        void setColor(Color color);
        /**
         * Requests, but does not require, that edge pixels draw opaque or with
         * partial transparency.
         *
         * @param aa setting for antialiasing
         */
        void setAntiAlias(bool aa);

        bool isAntiAlias() const;

        /**
         * @brief Get the paint's text size.
         *
         * @return the paint's text size.
         */
        float getTextSize() const
        {
            return text_size_;
        }

        /**
         * Set the paint's text size. This value must be > 0
         *
         * @param textSize the paint's text size.
         */
        void setTextSize(float textSize)
        {
            if (textSize <= 0.f)
                return;
            text_size_ = textSize;
        }

        /**
         * @brief Get the Font Threshold object
         *        If font size is larger than this value, the backend renderer may use
         *        path instead of font-texture to draw text.
         *
         * @return float
         */
        float getFontThreshold() const
        {
            return font_fill_threshold_;
        }

        /**
         * Retrieves alpha from the color used when stroking and filling.
         * @return alpha ranging from zero, fully transparent, to 255, fully opaque
         */
        float getAlphaF() const;
        /**
         * Replaces alpha, used to fill or stroke this paint. alpha is a value from
         * 0.0 to 1.0.
         * @param a alpha component of color
         */
        void setAlphaF(float a);
        // Helper that scales the alpha by 255.
        uint8_t getAlpha() const;
        void setAlpha(uint8_t alpha);

        void setPathEffect(std::shared_ptr<PathEffect> pathEffect)
        {
            path_effect_ = std::move(pathEffect);
        }

        std::shared_ptr<PathEffect> getPathEffect() const
        {
            return path_effect_;
        }

        void setShader(std::shared_ptr<Shader> shader)
        {
            shader_ = std::move(shader);
        }

        std::shared_ptr<Shader> getShader() const
        {
            return shader_;
        }

        void setTypeface(std::shared_ptr<Typeface> typeface)
        {
            typeface_ = typeface;
        }

        std::shared_ptr<Typeface> getTypeface() const
        {
            return typeface_;
        }

        void setMaskFilter(std::shared_ptr<MaskFilter> maskFilter)
        {
            mask_filter_ = std::move(maskFilter);
        }

        std::shared_ptr<MaskFilter> getMaskFilter() const
        {
            return mask_filter_;
        }

    private:
        void updateMiterLimit();

    private:
        Cap cap_ = kDefault_Cap;
        Join join_ = kDefault_Join;
        Style style_ = kFill_Style;
        float stroke_width_ = 1.0f;
        float miter_limit_ = 0.f;
        float text_size_ = 14.f;
        float font_fill_threshold_ = DEFAULT_FONT_FILL_THRESHOLD;
        float global_alpha_ = 1.f;
        Vector fill_color_ = { 1, 1, 1, 1 };
        Vector stroke_color_ = { 1, 1, 1, 1 };
        bool is_anti_alias_ = false;
        std::shared_ptr<PathEffect> path_effect_;
        std::shared_ptr<Shader> shader_;
        std::shared_ptr<Typeface> typeface_;
        std::shared_ptr<MaskFilter> mask_filter_;
    };

}  // namespace skity

#endif  // SKITY_GRAPHIC_PAINT_HPP
