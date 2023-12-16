#pragma once

#include "gui/widget.hpp"

namespace rl::gui {
    class ColorWheel : public Widget
    {
    public:
        ColorWheel(Widget* parent, const Color& color = { 1.f, 0.f, 0.f, 1.f });

        /// Set the change callback
        std::function<void(const Color&)> callback() const
        {
            return m_callback;
        }

        void set_callback(const std::function<void(const Color&)>& callback)
        {
            m_callback = callback;
        }

        /// Get the current color
        Color color() const;
        /// Set the current color
        void set_color(const Color& color);

        Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override;
        void draw(const std::unique_ptr<rl::Renderer>& renderer) override;
        void draw(SDL3::SDL_Renderer* renderer) override;
        bool mouse_button_event(const Vector2i& p, int button, bool down, int modifiers);
        bool mouse_drag_event(const Vector2i& p, const Vector2i& rel, int button, int modifiers);

    private:
        enum Region {
            None = 0,
            InnerTriangle = 1,
            OuterCircle = 2,
            Both = 3
        };

        Color hue2rgb(float h) const;
        Region adjustPosition(const Vector2i& p, Region consideredRegions = Both);

    protected:
        float mHue;
        float mWhite;
        float mBlack;
        Region mDragRegion;
        std::function<void(const Color&)> m_callback;
    };
}
