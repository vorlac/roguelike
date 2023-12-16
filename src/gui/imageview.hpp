#pragma once

#include <functional>

#include "core/assert.hpp"
#include "gui/widget.hpp"
#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
SDL_C_LIB_END

namespace rl::gui {
    /**
     * \class ImageView imageview.h sdl_gui/imageview.h
     *
     * \brief Widget used to display images.
     */
    class ImageView : public Widget
    {
    public:
        ImageView(Widget* parent, SDL3::SDL_Texture* texture);
        ~ImageView() override;

        void bind_image(SDL3::SDL_Texture* texture);

        Vector2f positionF() const
        {
            return m_pos.tofloat();
        }

        Vector2f size_flt() const
        {
            return m_size.tofloat();
        }

        const Vector2i& imageSize() const
        {
            return m_image_size;
        }

        Vector2i scaled_image_size() const
        {
            return (m_image_size.tofloat() * m_scale).toint();
        }

        Vector2f image_size_flt() const
        {
            return m_image_size.tofloat();
        }

        Vector2f scaled_image_size_flt() const
        {
            return m_image_size.tofloat() * m_scale;
        }

        const Vector2f& offset() const
        {
            return m_offset;
        }

        void set_offset(const Vector2f& offset)
        {
            m_offset = offset;
        }

        float scale() const
        {
            return m_scale;
        }

        void set_scale(float scale)
        {
            m_scale = scale > 0.01f ? scale : 0.01f;
        }

        bool fixed_offset() const
        {
            return m_fixed_offset;
        }

        void set_fixed_offset(bool fixed_offset)
        {
            m_fixed_offset = fixed_offset;
        }

        bool fixed_scale() const
        {
            return m_fixed_scale;
        }

        void set_fixed_scale(bool fixed_scale)
        {
            m_fixed_scale = fixed_scale;
        }

        float zoom_sensitivity() const
        {
            return m_zoom_sensitivity;
        }

        void set_zoom_sensitivity(float zoom_sensitivity)
        {
            m_zoom_sensitivity = zoom_sensitivity;
        }

        float grid_threshold() const
        {
            return m_grid_threshold;
        }

        void set_grid_threshold(float grid_threshold)
        {
            m_grid_threshold = grid_threshold;
        }

        float pixel_info_threshold() const
        {
            return m_pixel_info_threshold;
        }

        void set_pixel_info_threshold(float pixel_info_threshold)
        {
            m_pixel_info_threshold = pixel_info_threshold;
        }

        void set_pixel_info_callback(
            const std::function<std::pair<std::string, Color>(const Vector2i&)>& callback)
        {
            m_pixel_info_callback = callback;
        }

        const std::function<std::pair<std::string, Color>(const Vector2i&)>& pixel_info_callback()
            const
        {
            return m_pixel_info_callback;
        }

        void set_font_scale(float font_scale)
        {
            m_fond_scale_factor = font_scale;
        }

        float font_scale() const
        {
            return m_fond_scale_factor;
        }

        // Image transformation functions.

        // Calculates the image coordinates of the given pixel position on the widget.
        Vector2f image_coord_at(const Vector2f& position) const;

        // Calculates the image coordinates of the given pixel position on the widget.
        // If the position provided corresponds to a coordinate outside the range of
        // the image, the coordinates are clamped to edges of the image.
        Vector2f clamped_image_coord_at(const Vector2f& position) const;

        // Calculates the position inside the widget for the given image coordinate.
        Vector2f positionForCoordinate(const Vector2f& imageCoordinate) const;

        // Modifies the internal state of the image viewer widget so that the pixel at the provided
        // position on the widget has the specified image coordinate. Also clamps the values of
        // offset to the sides of the widget.
        void set_image_coord_at(const Vector2f& position, const Vector2f& imageCoordinate);

        // Centers the image without affecting the scaling factor.
        void center();

        // Centers and scales the image so that it fits inside the widgets.
        void fit();

        // Set the scale while keeping the image centered
        void set_scale_centered(float scale);

        // Moves the offset by the specified amount. Does bound checking.
        void move_offset(const Vector2f& delta);

        /**
         * Changes the scale factor by the provided amount modified by the zoom sensitivity member
         * variable. The scaling occurs such that the image coordinate under the focused position
         * remains in the same position before and after the scaling.
         */
        void zoom(int amount, const Vector2f& focusPosition);

        bool kb_button_event(int key, int scancode, int action, int modifiers) override;
        bool kb_character_event(unsigned int codepoint) override;
        bool mouse_drag_event(const Vector2i& p, const Vector2i& rel, int button,
                              int modifiers) override;
        bool scroll_event(const Vector2i& p, const Vector2f& rel) override;

        // Function indicating whether the grid is currently visible.
        bool grid_visible() const;

        // Function indicating whether the pixel information is currently visible.
        bool pixel_info_visible() const;

        // Function indicating whether any of the overlays are visible.
        bool debug_overlays_visible() const;

        Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override;
        void perform_layout(SDL3::SDL_Renderer* ctx) override;
        void draw(SDL3::SDL_Renderer* renderer) override;

        ImageView& with_image(SDL3::SDL_Texture* texture)
        {
            bind_image(texture);
            return *this;
        }

    private:
        // Helper image methods.
        void update_image_params();

        // Helper drawing methods.
        void draw_widget_border(SDL3::SDL_Renderer* ctx, const SDL3::SDL_Point& ap) const;
        void draw_image_border(SDL3::SDL_Renderer* ctx, const SDL3::SDL_Point& ap) const;
        void draw_debug_overlays(SDL3::SDL_Renderer* ctx) const;
        static void draw_pixel_grid(SDL3::SDL_Renderer* ctx, const Vector2f& upperLeftCorner,
                                    const Vector2f& lowerRightCorner, const float stride);
        void draw_pixel_info(SDL3::SDL_Renderer* ctx, const float stride) const;
        void write_pixel_info(SDL3::SDL_Renderer* ctx, const Vector2f& cellPosition,
                              const Vector2i& pixel, const float stride) const;

        SDL3::SDL_Texture* mTexture = nullptr;
        Vector2i m_image_size;

        // Image display parameters.
        float m_scale;
        Vector2f m_offset;
        bool m_fixed_scale;
        bool m_fixed_offset;

        // Fine-tuning parameters.
        float m_zoom_sensitivity = 1.1f;

        // Image info parameters.
        float m_grid_threshold = -1;
        float m_pixel_info_threshold = -1;

        // Image pixel data display members.
        std::function<std::pair<std::string, Color>(const Vector2i&)> m_pixel_info_callback;
        float m_fond_scale_factor = 0.2f;
    };
}
