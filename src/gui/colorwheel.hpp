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
            return mCallback;
        }

        void setCallback(const std::function<void(const Color&)>& callback)
        {
            mCallback = callback;
        }

        /// Get the current color
        Color color() const;
        /// Set the current color
        void setColor(const Color& color);

        Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;
        void draw(SDL3::SDL_Renderer* renderer) override;
        bool mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers);
        bool mouseDragEvent(const Vector2i& p, const Vector2i& rel, int button, int modifiers);

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
        std::function<void(const Color&)> mCallback;
    };
}
