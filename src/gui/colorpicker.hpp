#pragma once

#include <functional>
#include <memory>

#include "gui/popupbutton.hpp"

namespace rl::gui {

    class ColorWheel;

    class ColorPicker : public PopupButton
    {
    public:
        ColorPicker(Widget* parent, const Color& color = { 1.f, 0.0f, 0.0f, 1.0f });

        // Set the change callback
        const std::function<void(const Color&)>& callback() const
        {
            return mCallback;
        }

        void setCallback(const std::function<void(const Color&)>&& callback)
        {
            mCallback = std::move(callback);
        }

        /// Get the current color
        Color color() const;

        /// Set the current color
        void setColor(const Color& color);

    protected:
        // std::function<void(const Color&)> mCallback;
        ColorWheel* mColorWheel;
        Button* mPickButton;
    };
}
